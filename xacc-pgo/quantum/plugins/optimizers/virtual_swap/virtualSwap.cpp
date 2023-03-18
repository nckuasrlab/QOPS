#include "virtualSwap.hpp"
#include "xacc.hpp"

const int numQbit = 24;

namespace {
size_t getSwapIndex(size_t index, size_t* swapIndex){
  for(size_t i = 0; i < 6; ++i){
    if(index == i && swapIndex[i] != numQbit)
      return swapIndex[index];
    if(index == swapIndex[i])
      return i;
  }
  return numQbit;
}
} // namespace

namespace xacc {
namespace quantum {
void VirtualSwap::apply(std::shared_ptr<CompositeInstruction> program,
                             const std::shared_ptr<Accelerator> accelerator,
                             const HeterogeneousMap &options) {
  std::cout << "Test virtual swap\n";
  
  // virtual swap
  auto provider = xacc::getIRProvider("quantum");
  const auto buffer_name = program -> getInstruction(0) -> getBufferNames()[0];
  
  // get swap qbit index
  size_t targetCount[numQbit] = {0};
  for(size_t i = 0; i < program->nInstructions(); ++i){
    auto nowInst = program -> getInstruction(i);
    if(nowInst -> bits().size() == 2)
      targetCount[nowInst -> bits()[1]]++;
  }
  size_t swapIndex[6];
  for(int i = 0; i < 6; ++i)
    swapIndex[i] = numQbit;
  for(size_t i = 0; i < 6; ++i){
    size_t bigCount = i;
    for(size_t j = 6; j < numQbit; ++j){
      if(targetCount[bigCount] < targetCount[j])
        bigCount = j;
    }
    if(bigCount != i){
      swapIndex[i] = bigCount;
      targetCount[bigCount] = targetCount[i];
    }
  }
  for(int i = 5; i > 0; --i){
    for(int j = i - 1; j >= 0; --j)
      if(swapIndex[i] == swapIndex[j])
        swapIndex[j] = numQbit;
  }

  // change qbit index
  for(size_t i = 0; i < program->nInstructions(); ++i){
    auto nowInst = program -> getInstruction(i);
    size_t newQbit = getSwapIndex(nowInst -> bits()[0], swapIndex);
    if(newQbit != numQbit && nowInst -> bits().size() == 1)  
      nowInst->setBits({newQbit});
    if(nowInst -> bits().size() == 2){
      newQbit = getSwapIndex(nowInst -> bits()[0], swapIndex);
      if(newQbit != numQbit)  
        nowInst->setBits({newQbit, nowInst -> bits()[1]});
      newQbit = getSwapIndex(nowInst -> bits()[1], swapIndex);
      if(newQbit != numQbit)  
        nowInst->setBits({nowInst -> bits()[0], newQbit});
    }
  }

  // add swap gate
  for(size_t i = 0; i < 6; ++i)
    if(swapIndex[i] != numQbit){
      auto Inst = provider -> createInstruction("Swap", {i, swapIndex[i]});
      Inst -> setBufferNames({buffer_name, buffer_name});
      program -> insertInstruction(0, Inst);
      Inst = provider -> createInstruction("Swap", {i, swapIndex[i]});
      Inst -> setBufferNames({buffer_name, buffer_name});
      program -> addInstruction(Inst);
    }
}
} // namespace quantum
} // namespace xacc