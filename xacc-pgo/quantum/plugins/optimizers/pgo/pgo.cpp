#include "pgo.hpp"
#include "xacc.hpp"
#include <fstream>
#include <cstdlib>


namespace xacc {
namespace quantum {
void PGO::apply(std::shared_ptr<CompositeInstruction> program,
                             const std::shared_ptr<Accelerator> accelerator,
                             const HeterogeneousMap &options) {
  std::cout << "Test PGO\n";
  // staq_rotation_folding if t gate is up to XX%
  int t = 0;
  std::ifstream file;
  std::string file_name, line;
  const char* home_path = std::getenv("HOME");
  std::vector<std::vector<int>> prof_data;  // 2D vector
  std::vector<int> gate = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 20, 21, 22, 31, 32};

  file_name = std::string(home_path) + "/stateVector/src/correctness/xxx.out";  // countee-based profile data
  file.open(file_name); // open profile data

  if (file.is_open()){
    std::string num_tmp;
    std::vector<int> data_tmp;
    size_t pos = 0;

    while (std::getline(file, line)){
        data_tmp.clear();
        pos = line.find(" ");
        while (pos != std::string::npos){      // read profile data as a 2D vector
            num_tmp = line.substr(0, pos);
            data_tmp.push_back(std::stoi(num_tmp));
            line.erase(0, pos+1);
            pos = line.find(" ");
        }
        prof_data.push_back(data_tmp);
    }
    for (int gate_idx : gate){
        for (int i = 0; i < prof_data.size(); i++){
            t += prof_data[i][gate_idx];
        }
    }
    file.close();
  }
  else{
    std::cout << "profiling data not found.\n";
  }

  if(t > 10){
    auto opt = xacc::getIRTransformation("rotation-folding");
    opt->apply(program, nullptr);
  }
  
}
} // namespace quantum
} // namespace xacc
