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

// Show debug info by adding '-DENABLE_DEBUG=ON' when building the pass
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
// Register the argument option for the output file
static cl::opt<std::string>
    ProfileDataFilename("profile-gen",
                        cl::desc("Specify output filename for profile data"),
                        cl::value_desc("filename"), cl::init("stderr"));

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

static auto getDeclaredTypes(Module &M) {
  LLVMContext &Context = M.getContext();
  // Declare types
  Type *IntType = Type::getInt32Ty(Context);
  Type *IOFileType = StructType::create(M.getContext(), "struct._IO_FILE");
  Type *IOFilePtrType = PointerType::getUnqual(IOFileType);
  Type *TimespecType = StructType::create(
      ArrayRef<Type *>({Type::getInt64Ty(Context), Type::getInt64Ty(Context)}),
      "struct.timespec");
  Type *TimespecPtrType = PointerType::getUnqual(TimespecType);
  return std::make_tuple(IntType, IOFilePtrType, TimespecType, TimespecPtrType);
}

static bool declareFunctions(Module &M) {
  LLVMContext &Context = M.getContext();
  auto [IntType, IOFilePtrType, TimespecType, TimespecPtrType] =
      getDeclaredTypes(M);
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

static bool insertOnMainEntryBlock(BasicBlock &BB, Module &M) {
  LLVMContext &Context = M.getContext();
  IRBuilder<> Builder(Context);

  Instruction *inst = BB.getFirstNonPHI();
  if (dyn_cast<AllocaInst>(inst)) {
    Type *IOFilePtrType = std::get<1>(getDeclaredTypes(M));
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
    auto GlobalFile = M.getOrInsertGlobal("profile_data_file", IOFilePtrType);
    auto GVal = M.getNamedGlobal("profile_data_file");
    // initialize the variable with an undef value to ensure it is added to the
    // symbol table
    GVal->setInitializer(UndefValue::get(IOFilePtrType));
    Builder.CreateStore(OpenedFile, GlobalFile);
  }
  return true;
}

static bool insertOnMainEndBlock(BasicBlock &BB, Module &M) {
  LLVMContext &Context = M.getContext();
  IRBuilder<> Builder(Context);
  Instruction *inst = BB.getTerminator();
  Type *IOFilePtrType = std::get<1>(getDeclaredTypes(M));
  // int fclose(FILE*);
  auto FcloseFunc =
      M.getOrInsertFunction("fclose", Type::getInt32Ty(Context), IOFilePtrType);
  auto GlobalFile = M.getNamedGlobal("profile_data_file");

  Builder.SetInsertPoint(inst);
  auto LoadedGlobalFile = Builder.CreateLoad(IOFilePtrType, GlobalFile);
  Builder.CreateCall(FcloseFunc, {LoadedGlobalFile});
  return true;
}

bool QpgoPass::runOnModule(Module &M) {
  // Setup context
  LLVMContext &Context = M.getContext();
  IRBuilder<> Builder(Context);

  auto [IntType, IOFilePtrType, TimespecType, TimespecPtrType] =
      getDeclaredTypes(M);
  declareFunctions(M);

  // Need to load stderr or predefined output file as a global variable first
  std::string StderrOrFile =
      ProfileDataFilename == "stderr" ? "stderr" : "profile_data_file";
  Value *GlobalProfileDataFile =
      M.getOrInsertGlobal(StderrOrFile, IOFilePtrType);
  if (GlobalProfileDataFile == nullptr)
    errs() << "GlobalProfileDataFile is null";

  std::vector<std::pair<ProfiledFuncKind, CallInst *>> InjectPoints;
  QPGO_DEBUG(dbgs() << "I saw a module called '" << M.getName() << "'\n");
  for (Function &F : M) {
    QPGO_DEBUG(dbgs() << "I saw a function called '" << F.getName()
                      << "', arg_size: " << F.arg_size() << "\n");

    if (F.hasName() && F.getName() == "main" &&
        ProfileDataFilename != "stderr") {
      // inject fopen, fclose to begin, end of main function if the target
      // output file option is not stderr
      insertOnMainEntryBlock(F.getEntryBlock(), M);
      insertOnMainEndBlock(F.getEntryBlock(), M);
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

    // enum value to string
    Value *FuncNameStrVal = Builder.CreateGlobalStringPtr(
        std::to_string(static_cast<std::underlying_type_t<ProfiledFuncKind>>(
            ProfiledFunc)),
        "FuncNameStrVal", 0, &M);
    auto *LoadedProfileDataFile =
        Builder.CreateLoad(IOFilePtrType, GlobalProfileDataFile);

    // get operands from the call inst
    unsigned NumOperands = CBI->getNumArgOperands();
    QPGO_DEBUG(dbgs() << "Num of arguments: " << NumOperands << ", args: ");
    std::string FuncArgsStr = " ";
    for (int i = 0; i < NumOperands; ++i) {
      const Value *operand = CBI->getArgOperand(i);
      FuncArgsStr.append(operandToFlag(operand));
      FuncArgsStr.append(" ");
    }
    QPGO_DEBUG(dbgs() << "format specifier of printf: \"");
    QPGO_DEBUG(dbgs().write_escaped(FuncArgsStr) << "\"\n");
    Value *FuncArgsStrVal =
        Builder.CreateGlobalStringPtr(FuncArgsStr, "FuncArgsStrVal", 0, &M);
    std::vector<Value *> ArgsArgs({LoadedProfileDataFile, FuncArgsStrVal});
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
      ArgsArgs.push_back(Operand);
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

    Value *TvStrVal =
        Builder.CreateGlobalStringPtr("%lld\n", "TvStrVal", 0, &M);

    // Use lockfile to block file write
    Builder.CreateCall(M.getFunction("flockfile"), {LoadedProfileDataFile});
    Builder.CreateCall(M.getFunction("fprintf"),
                       {LoadedProfileDataFile, FuncNameStrVal});
    Builder.CreateCall(M.getFunction("fprintf"), ArgsArgs);
    Builder.CreateCall(M.getFunction("fprintf"),
                       {LoadedProfileDataFile, TvStrVal, TvNsec});
    Builder.CreateCall(M.getFunction("fflush"), {LoadedProfileDataFile});
    Builder.CreateCall(M.getFunction("funlockfile"), {LoadedProfileDataFile});
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
