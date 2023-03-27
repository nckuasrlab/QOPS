//===-- Qpgo.cpp --------------------------------------------*- C++ -*-===//
#include "Qpgo.h"
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <llvm/Pass.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/Debug.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Transforms/IPO/PassManagerBuilder.h>
#include <llvm/Transforms/Utils/BasicBlockUtils.h>
#include <tuple>
#include <vector>

// Show debug info by adding '-DENABLE_DEBUG=ON' when using cmake to build the
// pass; otherwise, suppress the debug info
#ifndef NDEBUG
#define QPGO_DEBUG(X)                                                          \
  do {                                                                         \
    X;                                                                         \
  } while (false)
#else
#define QPGO_DEBUG(X)                                                          \
  do {                                                                         \
  } while (false)
#endif

namespace llvm {
const int MAX_NUM_OF_QUBIT = 40;
const int MAX_NUM_OF_QGATE = 33;

// types which are frequently used
static Type *IntType;
static Type *IOFileType;
static Type *IOFilePtrType;
static Type *TimespecType;
static Type *TimespecPtrType;
static ArrayType *CounterInnerType;
static ArrayType *CounterType;

// Register the argument option for the output file
static cl::opt<std::string>
    ProfileDataFilename("profile-gen",
                        cl::desc("Specify output filename for profile data"),
                        cl::value_desc("filename"), cl::init("stderr"));

static cl::opt<std::string>
    ProfileMode("profile-mode",
                cl::desc("Specify the mode for profiling (counter or context)"),
                cl::value_desc("mode"), cl::init("counter"));

static const char *operandToFlag(const Value *Operand) {
  const auto *OperandType = Operand->getType();
  // type check
  if (OperandType->isIntegerTy()) {
    return "%d";
  } else if (OperandType->isFloatTy()) {
    return "%f";
  } else if (OperandType->isDoubleTy()) {
    return "%lf";
  } else {
    Operand->dump();
    return "%p";
  }
}

static void declaredTypes(Module &M) {
  LLVMContext &Context = M.getContext();
  // Declare types
  IntType = Type::getInt32Ty(Context);
  IOFileType = StructType::create(M.getContext(), "struct._IO_FILE");
  IOFilePtrType = PointerType::getUnqual(IOFileType);
  TimespecType = StructType::create(
      ArrayRef<Type *>({Type::getInt64Ty(Context), Type::getInt64Ty(Context)}),
      "struct.timespec");
  TimespecPtrType = PointerType::getUnqual(TimespecType);
  CounterInnerType = ArrayType::get(IntType, MAX_NUM_OF_QGATE);
  CounterType = ArrayType::get(CounterInnerType, MAX_NUM_OF_QUBIT);
}

static bool declareFunctions(Module &M) {
  LLVMContext &Context = M.getContext();
  // int fprintf(FILE *stream, const char *format, ...);
  std::vector<Type *> FprintfArgsType(
      {IOFilePtrType, Type::getInt8PtrTy(Context)});
  auto FprintfType = FunctionType::get(IntType, FprintfArgsType, true);
  auto FprintfFunc =
      Function::Create(FprintfType, Function::ExternalLinkage, "fprintf", M);
  // void fflush(FILE *filehandle);
  auto FflushType = FunctionType::get(IntType, {IOFilePtrType}, false);
  auto FflushFunc =
      Function::Create(FflushType, Function::ExternalLinkage, "fflush", M);
  // void flockfile(FILE *filehandle);
  auto FlockfileFunc =
      Function::Create(FflushType, Function::ExternalLinkage, "flockfile", M);
  // void funlockfile(FILE *filehandle);
  auto FunlockfileFunc =
      Function::Create(FflushType, Function::ExternalLinkage, "funlockfile", M);
  // int clock_gettime(clockid_t clk_id, struct timespec *tp);
  std::vector<Type *> ClockGettimeArgsType({IntType, TimespecPtrType});
  auto ClockGettimeType =
      FunctionType::get(IntType, ClockGettimeArgsType, false);
  auto ClockGettimeFunc = Function::Create(
      ClockGettimeType, Function::ExternalLinkage, "clock_gettime", M);
  // int omp_get_thread_num();
  auto OmpGetThreadNumType = FunctionType::get(IntType, false);
  auto OmpGetThreadNumFunc = Function::Create(
      OmpGetThreadNumType, Function::ExternalLinkage, "omp_get_thread_num", M);
  return true;
}

static bool insertOnMainEntryBlock(BasicBlock &BB, Module &M,
                                   bool is_counter_based) {
  LLVMContext &Context = M.getContext();
  IRBuilder<> Builder(Context);

  Instruction *inst = BB.getFirstNonPHI();
  if (dyn_cast<AllocaInst>(inst)) {
    // FILE *fopen (const char *, const char *);
    auto FopenFunc = M.getOrInsertFunction("fopen", IOFilePtrType,
                                           Type::getInt8PtrTy(Context),
                                           Type::getInt8PtrTy(Context));
    Value *ProfileDataFilenameStrVal = Builder.CreateGlobalStringPtr(
        ProfileDataFilename, "ProfileDataFilenameStrVal", 0, &M);
    Value *WPlusStrVal =
        Builder.CreateGlobalStringPtr("w+", "WPlusStrVal", 0, &M);

    Builder.SetInsertPoint(inst);
    auto OpenedFile =
        Builder.CreateCall(FopenFunc, {ProfileDataFilenameStrVal, WPlusStrVal});
    auto ProfilingFileHandle =
        M.getOrInsertGlobal("_qpgo_profile_file", IOFilePtrType);
    auto GVal = M.getNamedGlobal("_qpgo_profile_file");
    // initialize the variable with an undef value to ensure it is added to the
    // symbol table
    GVal->setInitializer(UndefValue::get(IOFilePtrType));
    Builder.CreateStore(OpenedFile, ProfilingFileHandle);

    if (is_counter_based) { // add a global variable for counters
      auto GlobalCounters = M.getOrInsertGlobal("_qpgo_counters", CounterType);
      auto GCountersVal = M.getNamedGlobal("_qpgo_counters");
      GCountersVal->setAlignment(MaybeAlign(16));
      ConstantAggregateZero *Const0Array =
          ConstantAggregateZero::get(CounterType);
      GCountersVal->setInitializer(Const0Array);
    }
  }
  return true;
}

static bool insertOnMainEndBlock(BasicBlock &BB, Module &M,
                                 bool is_counter_based) {
  LLVMContext &Context = M.getContext();
  IRBuilder<> Builder(Context);
  Instruction *inst = BB.getTerminator();
  Builder.SetInsertPoint(inst);

  auto LoadedProfilingFileHandle =
      Builder.CreateLoad(IOFilePtrType, M.getNamedGlobal("_qpgo_profile_file"));

  if (is_counter_based) { // dump the counters to the file for counter-based
                          // profiling
    Value *GCountersVal = M.getOrInsertGlobal("_qpgo_counters", CounterType);
    Value *CounterFmt =
        Builder.CreateGlobalStringPtr("%d ", "CounterFmt", 0, &M);
    Value *CounterNewlineFmt =
        Builder.CreateGlobalStringPtr("%d\n", "CounterNewlineFmt", 0, &M);
    for (int Qubit = 0; Qubit < MAX_NUM_OF_QUBIT; ++Qubit) {
      for (int GateType = 0; GateType < MAX_NUM_OF_QGATE; ++GateType) {
        Value *Counter = Builder.CreateInBoundsGEP(
            CounterType, GCountersVal,
            ArrayRef<Value *>({Builder.getInt32(0), Builder.getInt32(Qubit),
                               Builder.getInt32(GateType)}));
        Value *LoadedCounter = Builder.CreateLoad(IntType, Counter);
        if (GateType == MAX_NUM_OF_QGATE - 1) {
          Builder.CreateCall(
              M.getFunction("fprintf"),
              {LoadedProfilingFileHandle, CounterNewlineFmt, LoadedCounter});
        } else {
          Builder.CreateCall(
              M.getFunction("fprintf"),
              {LoadedProfilingFileHandle, CounterFmt, LoadedCounter});
        }
      }
    }
  }
  // int fclose(FILE*);
  auto FcloseFunc =
      M.getOrInsertFunction("fclose", Type::getInt32Ty(Context), IOFilePtrType);
  Builder.CreateCall(FcloseFunc, {LoadedProfilingFileHandle});
  return true;
}

static bool insertCounterProbe(
    Module &M,
    std::vector<std::pair<ProfiledFuncKind, CallInst *>> &InjectPoints,
    Value *ProfilingFileHandle) {
  // Setup context
  LLVMContext &Context = M.getContext();
  IRBuilder<> Builder(Context);

  for (auto [ProfiledFunc, CBI] : InjectPoints) {
    // insert printf right before the call instruction
    Builder.SetInsertPoint(CBI);
    // call omp_get_thread_num
    Value *Tid = Builder.CreateAlloca(IntType, 0, "Tid");
    Value *OmpThreadNum =
        Builder.CreateCall(M.getFunction("omp_get_thread_num"), {});
    Builder.CreateStore(OmpThreadNum, Tid);
    Tid = Builder.CreateLoad(IntType, Tid);
    // compare tid and 0
    Value *Cmp =
        Builder.CreateICmpEQ(Tid, Builder.getInt32(0), "icmp_thread_num");

    // split BB and start to insert code in the then block
    auto *ThenBB =
        SplitBlockAndInsertIfThen(Cmp, &*Builder.GetInsertPoint(), false);
    Builder.SetInsertPoint(ThenBB);

    std::vector<Value *> MetaFuncArgs;
    unsigned NumOperands = CBI->getNumArgOperands();
    for (int i = 0; i < NumOperands; ++i) {
      Value *Operand = CBI->getArgOperand(i);
      if (Operand->getType()->isFloatTy()) {
        // We cannot print float number by printf function if we pass it
        // to the function Ref: https://stackoverflow.com/a/28097654
        QPGO_DEBUG(dbgs() << "Add fpext for printing float vaule\n");
        // update operand to the casted version
        Operand = Builder.CreateCast(Instruction::FPExt, Operand,
                                     Type::getDoubleTy(Context));
      }
      MetaFuncArgs.push_back(Operand);
    }

    Value *GCountersVal = M.getOrInsertGlobal("_qpgo_counters", CounterType);
    QPGO_DEBUG(dbgs() << "get _qpgo_counters\n");

    std::vector<Value *> Qubits;
    Value *GateType;
    switch (ProfiledFunc) { // collect counters for different gate types
    case PFK_single_gate:
      GateType = MetaFuncArgs[1];
      Qubits.push_back(MetaFuncArgs[0]);
      break;
    case PFK_control_gate:
      GateType = MetaFuncArgs[2];
      Qubits.push_back(MetaFuncArgs[0]);
      Qubits.push_back(MetaFuncArgs[1]);
      break;
    case PFK_unitary4x4:
      errs() << "Not implement yet for PFK_unitary4x4\n";
      break;
    case PFK_SWAP:
      GateType = Builder.getInt32(
          13); // hard code for the SWAP (ref: simulator's gate.h)
      Qubits.push_back(MetaFuncArgs[0]);
      Qubits.push_back(MetaFuncArgs[1]);
      break;
    case PFK_unitary8x8:
      errs() << "Not implement yet for PFK_unitary8x8\n";
      break;
    }
    for (Value *Qubit : Qubits) { // increase counters
      Value *Counter = Builder.CreateInBoundsGEP(
          CounterType, GCountersVal,
          ArrayRef<Value *>({Builder.getInt32(0), Qubit, GateType}));
      Value *LoadedCounter = Builder.CreateLoad(IntType, Counter);
      Value *tmp = Builder.CreateBinOp(Instruction::Add, LoadedCounter,
                                       Builder.getInt32(1));
      Builder.CreateStore(tmp, Counter);
    }
  }
  return true;
}

static bool insertContextProbe(
    Module &M,
    std::vector<std::pair<ProfiledFuncKind, CallInst *>> &InjectPoints,
    Value *ProfilingFileHandle) {
  // Setup context
  LLVMContext &Context = M.getContext();
  IRBuilder<> Builder(Context);

  // Inject if condition to check tid == 0 for collected functions
  for (auto [ProfiledFunc, CBI] : InjectPoints) {
    // insert printf right before the call instruction
    Builder.SetInsertPoint(CBI);
    // call omp_get_thread_num
    Value *Tid = Builder.CreateAlloca(IntType, 0, "Tid");
    Value *OmpThreadNum =
        Builder.CreateCall(M.getFunction("omp_get_thread_num"), {});
    Builder.CreateStore(OmpThreadNum, Tid);
    Tid = Builder.CreateLoad(IntType, Tid);
    // compare tid and 0
    Value *Cmp =
        Builder.CreateICmpEQ(Tid, Builder.getInt32(0), "icmp_thread_num");

    // split BB and start to insert code in the then block
    auto *ThenBB =
        SplitBlockAndInsertIfThen(Cmp, &*Builder.GetInsertPoint(), false);
    Builder.SetInsertPoint(ThenBB);

    // enum value to string, we only use integer to represent different meta
    // function to reduce the output bytes
    Value *MetaFuncTypeVal = Builder.CreateGlobalStringPtr(
        std::to_string(static_cast<std::underlying_type_t<ProfiledFuncKind>>(
            ProfiledFunc)),
        "MetaFuncTypeVal", 0, &M);
    auto *LoadedProfilingFileHandle =
        Builder.CreateLoad(IOFilePtrType, ProfilingFileHandle);

    // get operands from the call inst
    unsigned NumOperands = CBI->getNumArgOperands();
    QPGO_DEBUG(dbgs() << "Num of arguments: " << NumOperands << ", args: ");
    std::string Fmt = " "; // the format of fprintf function
    for (int i = 0; i < NumOperands - 1; ++i) { // -1 is for skipping density
      const Value *operand = CBI->getArgOperand(i);
      Fmt.append(operandToFlag(operand));
      Fmt.append(" ");
    }
    QPGO_DEBUG(dbgs() << "format specifier of printf: \"");
    QPGO_DEBUG(dbgs().write_escaped(Fmt) << "\"\n");
    Value *MetaFuncArgsFmt =
        Builder.CreateGlobalStringPtr(Fmt, "MetaFuncArgsFmt", 0, &M);
    std::vector<Value *> MetaFuncArgsFmtAndVal(
        {LoadedProfilingFileHandle, MetaFuncArgsFmt});
    for (int i = 0; i < NumOperands - 1; ++i) { // -1 is for skipping density
      Value *Operand = CBI->getArgOperand(i);
      if (Operand->getType()->isFloatTy()) {
        // We cannot print float number by printf function if we pass it
        // to the function Ref: https://stackoverflow.com/a/28097654
        QPGO_DEBUG(dbgs() << "Add fpext for printing float vaule\n");
        // update operand to the casted version
        Operand = Builder.CreateCast(Instruction::FPExt, Operand,
                                     Type::getDoubleTy(Context));
      }
      MetaFuncArgsFmtAndVal.push_back(Operand);
    }

    // call clock_gettime
    Value *TimeSpec = Builder.CreateAlloca(TimespecType, 0, "TimeSpec");
    std::vector<Value *> ClockGettimeArgs(
        {Builder.getInt32(1), TimeSpec}); // CLOCK_MONOTONIC
    Builder.CreateCall(M.getFunction("clock_gettime"), ClockGettimeArgs);

    // TimeSpec.tv_sec*1000000000 + TimeSpec.tv_nsec
    Value *TvSec = Builder.CreateInBoundsGEP(
        TimespecType, TimeSpec,
        ArrayRef<Value *>({Builder.getInt32(0), Builder.getInt32(0)}));
    TvSec = Builder.CreateLoad(Type::getInt64Ty(Context), TvSec);
    Value *E9 = Builder.getInt32(1000000000);
    TvSec = Builder.CreateBinOp(Instruction::Mul, TvSec, E9);
    Value *TvNsec = Builder.CreateInBoundsGEP(
        TimespecType, TimeSpec,
        ArrayRef<Value *>({Builder.getInt32(0), Builder.getInt32(1)}));
    TvNsec = Builder.CreateLoad(Type::getInt64Ty(Context), TvNsec);
    TvNsec = Builder.CreateBinOp(Instruction::Add, TvNsec, TvSec);

    Value *TvNsecFmt =
        Builder.CreateGlobalStringPtr("%lld\n", "TvNsecFmt", 0, &M);

    // Use lockfile to block file write
    Builder.CreateCall(M.getFunction("flockfile"), {LoadedProfilingFileHandle});
    Builder.CreateCall(M.getFunction("fprintf"),
                       {LoadedProfilingFileHandle, MetaFuncTypeVal});
    Builder.CreateCall(M.getFunction("fprintf"), MetaFuncArgsFmtAndVal);
    Builder.CreateCall(M.getFunction("fprintf"),
                       {LoadedProfilingFileHandle, TvNsecFmt, TvNsec});
    Builder.CreateCall(M.getFunction("fflush"), {LoadedProfilingFileHandle});
    Builder.CreateCall(M.getFunction("funlockfile"),
                       {LoadedProfilingFileHandle});
  }
  return true;
}

bool QpgoPass::runOnModule(Module &M) {
  // Setup context
  LLVMContext &Context = M.getContext();
  IRBuilder<> Builder(Context);

  declaredTypes(M);
  declareFunctions(M);

  // Need to load stderr or predefined output file as a global variable first
  std::string StderrOrFile =
      ProfileDataFilename == "stderr" ? "stderr" : "_qpgo_profile_file";
  Value *ProfilingFileHandle = M.getOrInsertGlobal(StderrOrFile, IOFilePtrType);
  if (ProfilingFileHandle == nullptr)
    errs() << "ProfilingFileHandle is null";

  std::vector<std::pair<ProfiledFuncKind, CallInst *>> InjectPoints;
  QPGO_DEBUG(dbgs() << "I saw a module called '" << M.getName() << "'\n");
  for (Function &F : M) {
    QPGO_DEBUG(dbgs() << "I saw a function called '" << F.getName()
                      << "', arg_size: " << F.arg_size() << "\n");

    if (F.hasName() && F.getName() == "main" &&
        ProfileDataFilename != "stderr") {
      // inject fopen, fclose to begin, end of main function if the target
      // output file option is not stderr
      insertOnMainEntryBlock(F.getEntryBlock(), M, ProfileMode == "counter");
      insertOnMainEndBlock(F.getEntryBlock(), M, ProfileMode == "counter");
    }

    // Iterate F, BB
    for (BasicBlock &BB : F) {
      for (Instruction &BI : BB) {
        if (auto *CBI = dyn_cast<CallInst>(&BI)) {
          // get the called function name
          auto CalledFunction = CBI->getCalledFunction();
          if (CalledFunction == nullptr) // inline function will be null
            continue;
          auto FuncName = CalledFunction->getName();

          // process specific functions (gates) and
          // skip llvm functions to prevent error: Running pass 'X86
          // DAG->DAG Instruction Selection when '-g'
          const auto &ProfiledFuncIt = ProfiledFuncsMap.find(FuncName.str());
          if (ProfiledFuncIt == ProfiledFuncsMap.end())
            continue;

          QPGO_DEBUG(dbgs() << "I saw a CallInst called '" << FuncName
                            << "' (collect)\n");
          // collect target functions into the vector
          InjectPoints.push_back(std::make_pair(ProfiledFuncIt->second, CBI));
        }
      }
    }
  }
  // using different modes for profiling
  if (ProfileMode == "counter") {
    insertCounterProbe(M, InjectPoints, ProfilingFileHandle);
  } else if (ProfileMode == "context") {
    insertContextProbe(M, InjectPoints, ProfilingFileHandle);
  } else {
    errs() << ProfileMode
           << " is not supported profiling mode, which should be counter or "
              "context.\n";
  }
  return true;
}

char QpgoPass::ID = 0;

// Automatically enable the pass.
static RegisterPass<QpgoPass> X("QpgoPass", "Qpgo Pass",
                                false /* Only looks at CFG */,
                                false /* Analysis Pass */);

static RegisterStandardPasses Y(PassManagerBuilder::EP_ModuleOptimizerEarly,
                                [](const PassManagerBuilder &Builder,
                                   legacy::PassManagerBase &PM) {
                                  PM.add(new QpgoPass());
                                });

} // namespace llvm
