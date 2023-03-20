//===-- Qpgo.cpp --------------------------------------------*- C++ -*-===//
#include "Qpgo.h"
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/Twine.h>
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
#include <llvm/Support/raw_ostream.h>
#include <llvm/Transforms/IPO/PassManagerBuilder.h>
#include <llvm/Transforms/Utils/BasicBlockUtils.h>
#include <vector>

namespace llvm {
// Register the argument option for the output file
static cl::opt<std::string>
    ProfileDataFilenameOpt("profile-gen",
                           cl::desc("Specify output filename for profile data"),
                           cl::value_desc("filename"), cl::init("stderr"));

static const char *operandToFlag(const Value *Operand) {
  auto OperandType = Operand->getType();
  // type check
  // Ref: https://stackoverflow.com/a/22163892
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
static bool insertOnMainEntryBlock(BasicBlock &BB, Module &M) {
  Instruction *inst = BB.getFirstNonPHI();
  if (dyn_cast<AllocaInst>(inst)) {
    errs() << inst << "\n";
  }
  return true;
}

bool QpgoPass::runOnModule(Module &M) {
  // Setup context
  LLVMContext &Context = M.getContext();
  IRBuilder<> Builder(Context);
  // Declare types
  Type *IOFileType = StructType::create(M.getContext(), "struct._IO_FILE");
  Type *IOFilePtrType = PointerType::getUnqual(IOFileType);
  Type *IntType = Type::getInt32Ty(Context);
  Type *TimespecType = StructType::create(
      ArrayRef<Type *>({Type::getInt64Ty(Context), Type::getInt64Ty(Context)}),
      "struct.timespec");
  Type *TimespecPtrType = PointerType::getUnqual(TimespecType);

  // Declare functions
  // int fprintf(FILE *stream, const char *format, ...);
  std::vector<Type *> FprintfArgsType(
      {IOFilePtrType, Type::getInt8PtrTy(Context)});
  FunctionType *FprintfType = FunctionType::get(IntType, FprintfArgsType, true);
  Constant *FprintfFunc =
      Function::Create(FprintfType, Function::ExternalLinkage, "fprintf", M);
  // void fflush(FILE *filehandle);
  FunctionType *FflushType = FunctionType::get(IntType, {IOFilePtrType}, false);
  Constant *FflushFunc =
      Function::Create(FflushType, Function::ExternalLinkage, "fflush", M);
  // void flockfile(FILE *filehandle);
  Constant *FlockfileFunc =
      Function::Create(FflushType, Function::ExternalLinkage, "flockfile", M);
  // void funlockfile(FILE *filehandle);
  Constant *FunlockfileFunc =
      Function::Create(FflushType, Function::ExternalLinkage, "funlockfile", M);
  // int clock_gettime(clockid_t clk_id, struct timespec *tp);
  std::vector<Type *> ClockGettimeArgsType({IntType, TimespecPtrType});
  FunctionType *ClockGettimeType =
      FunctionType::get(IntType, ClockGettimeArgsType, false);
  Constant *ClockGettimeFunc = Function::Create(
      ClockGettimeType, Function::ExternalLinkage, "clock_gettime", M);
  // int omp_get_thread_num();
  FunctionType *OmpGetThreadNumType = FunctionType::get(IntType, false);
  Constant *OmpGetThreadNumFunc = Function::Create(
      OmpGetThreadNumType, Function::ExternalLinkage, "omp_get_thread_num", M);

  // Need to load stderr as a global variable first
  Value *GlobalStderr = M.getOrInsertGlobal("stderr", IOFilePtrType);
  if (GlobalStderr == nullptr) {
    errs() << "global_var_stderr is null";
  }

  std::vector<CallInst *> InjectPoints;
  errs() << "I saw a module called '" << M.getName() << "'\n";
  for (Function &F : M) {
    errs() << "I saw a function called '" << F.getName()
           << "', arg_size: " << F.arg_size() << "\n";

    if ((F.getName() == "main")) {
      insertOnMainEntryBlock(F.getEntryBlock(), M);
    }

    // Iterate F, BB
    for (BasicBlock &BB : F) {
      for (Instruction &BI : BB) {
        if (auto CBI = dyn_cast<CallInst>(&BI)) {
          // get the called function name
          auto FuncName = CBI->getCalledFunction()->getName();

          // process specific functions (gates) and
          // skip llvm functions to prevent error: Running pass 'X86
          // DAG->DAG Instruction Selection when '-g'
          const auto ProfiledFuncIt = ProfiledFuncs.find(FuncName.str());
          if (ProfiledFuncIt == ProfiledFuncs.end()) {
            continue;
          }
          errs() << "I saw a CallInst called '" << FuncName << "' (collect)\n";
          // collect target functions into the vector
          InjectPoints.push_back(CBI);
        }
      }
    }
  }
  // Inject if condition to check tid == 0 for collected functions
  // Ref: https://blog.csdn.net/weixin_50972562/article/details/125437597
  for (auto *CBI : InjectPoints) {
    auto FuncName = CBI->getCalledFunction()->getName();
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
    const auto ProfiledFuncIt = ProfiledFuncs.find(FuncName.str());
    Value *FuncNameStrVal = Builder.CreateGlobalStringPtr(
        std::to_string(static_cast<std::underlying_type_t<ProfiledFuncKind>>(
            ProfiledFuncIt->second)),
        "FuncNameStrVal", 0, &M);
    auto LoadedStderr = Builder.CreateLoad(IOFilePtrType, GlobalStderr);
    std::vector<Value *> NameArgs({LoadedStderr, FuncNameStrVal});

    // get operands from the call inst
    unsigned NumOperands = CBI->getNumArgOperands();
    errs() << "Num of arguments: " << NumOperands << ", args: ";
    std::string FuncArgsStr = " ";
    for (int i = 0; i < NumOperands; ++i) {
      const Value *operand = CBI->getArgOperand(i);
      FuncArgsStr.append(operandToFlag(operand));
      FuncArgsStr.append(" ");
    }
    errs() << "format specifier of printf: \"";
    errs().write_escaped(FuncArgsStr) << "\"\n";
    Value *FuncArgsStrVal =
        Builder.CreateGlobalStringPtr(FuncArgsStr, "FuncArgsStrVal", 0, &M);
    std::vector<Value *> ArgsArgs({LoadedStderr, FuncArgsStrVal});
    for (int i = 0; i < NumOperands; ++i) {
      Value *Operand = CBI->getArgOperand(i);
      if (Operand->getType()->isFloatTy()) {
        // We cannot print float number by printf function if we pass it
        // to the function Ref: https://stackoverflow.com/a/28097654
        errs() << "Add fpext for printing float vaule\n";
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

    Value *TvStrVal = Builder.CreateGlobalStringPtr("%lld\n", "TvStrVal", 0, &M);
    std::vector<Value *> TimeSpecArgs({LoadedStderr, TvStrVal, TvNsec});

    // Use lockfile to block stderr
    Builder.CreateCall(M.getFunction("flockfile"), {LoadedStderr});
    Builder.CreateCall(M.getFunction("fprintf"), NameArgs);
    Builder.CreateCall(M.getFunction("fprintf"), ArgsArgs);
    Builder.CreateCall(M.getFunction("fprintf"), TimeSpecArgs);
    Builder.CreateCall(M.getFunction("fflush"), {LoadedStderr});
    Builder.CreateCall(M.getFunction("funlockfile"), {LoadedStderr});
  }
  errs() << ProfileDataFilenameOpt << "\n";
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
