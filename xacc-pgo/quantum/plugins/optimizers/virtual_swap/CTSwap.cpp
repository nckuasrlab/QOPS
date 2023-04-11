#include "CTSwap.hpp"
#include "xacc.hpp"

namespace {
size_t getSwapIndex(size_t index, size_t* swapIndex, size_t numQbit){
  for(size_t i = 0; i < 6; ++i){
    if(index == i && swapIndex[i] != numQbit)
      return swapIndex[i];
    if(index == swapIndex[i])
      return i;
  }
  return numQbit;
}
} // namespace

namespace xacc {
namespace quantum {
void CTSwap::apply(std::shared_ptr<CompositeInstruction> program,
                             const std::shared_ptr<Accelerator> accelerator,
                             const HeterogeneousMap &options) {
  const size_t numQbit = program->nLogicalBits();
  
  // virtual swap
  auto provider = xacc::getIRProvider("quantum");
  const auto buffer_name = program -> getInstruction(0) -> getBufferNames()[0];
  
  // get swap qbit index
  size_t controlCount[numQbit] = {0};
  for(size_t i = 0; i < program->nInstructions(); ++i){
    auto nowInst = program -> getInstruction(i);
    if(nowInst -> bits().size() == 2)
      controlCount[nowInst -> bits()[0]]++;
  }
  size_t swapIndex[6];
  for(int i = 0; i < 6; ++i)
    swapIndex[i] = numQbit;
  for(size_t i = 0; i < 6; ++i){
    size_t bigCount = i;
    for(size_t j = 6; j < numQbit; ++j){
      if(controlCount[bigCount] < controlCount[j])
        bigCount = j;
    }
    if(bigCount != i){
      swapIndex[i] = bigCount;
      controlCount[bigCount] = controlCount[i];
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
    size_t newQbit = getSwapIndex(nowInst -> bits()[0], swapIndex, numQbit);
    if(newQbit != numQbit && nowInst -> bits().size() == 1)  
      nowInst->setBits({newQbit});
    if(nowInst -> bits().size() == 2){
      newQbit = getSwapIndex(nowInst -> bits()[0], swapIndex, numQbit);
      if(newQbit != numQbit)  
        nowInst->setBits({newQbit, nowInst -> bits()[1]});
      newQbit = getSwapIndex(nowInst -> bits()[1], swapIndex, numQbit);
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