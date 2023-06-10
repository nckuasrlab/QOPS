//Copyright © 2019 D-Wave Systems Inc.
//The software is licensed to authorized users only under the applicable license agreement.  See License.txt.

#ifndef LOCAL_HPP_INCLUDED
#define LOCAL_HPP_INCLUDED

#include "dwave_sapi.h"
#include "sapi-impl.hpp"

namespace sapi {

// class LocalC4OptimizeSolver : public sapi_Solver {
// private:
//   virtual const sapi_SolverProperties* propertiesImpl() const;
//   virtual SubmittedProblemPtr submitImpl(
//       sapi_ProblemType type, const sapi_Problem *problem,
//       const sapi_SolverParameters *params) const;

//   virtual IsingResultPtr solveImpl(
//       sapi_ProblemType type, const sapi_Problem *problem,
//       const sapi_SolverParameters *params) const {
//     return submit(type, problem, params)->result();
//   }
// };


// class LocalC4SampleSolver : public sapi_Solver {
// private:
//   virtual const sapi_SolverProperties* propertiesImpl() const;
//   virtual SubmittedProblemPtr submitImpl(
//       sapi_ProblemType type, const sapi_Problem *problem,
//       const sapi_SolverParameters *params) const;

//   virtual IsingResultPtr solveImpl(
//       sapi_ProblemType type, const sapi_Problem *problem,
//       const sapi_SolverParameters *params) const {
//     return submit(type, problem, params)->result();
//   }
// };


// class LocalIsingHeuristicSolver : public sapi_Solver {
// private:
//   virtual const sapi_SolverProperties* propertiesImpl() const;
//   virtual SubmittedProblemPtr submitImpl(
//       sapi_ProblemType type, const sapi_Problem *problem,
//       const sapi_SolverParameters *params) const;

//   virtual IsingResultPtr solveImpl(
//       sapi_ProblemType type, const sapi_Problem *problem,
//       const sapi_SolverParameters *params) const {
//     return submit(type, problem, params)->result();
//   }
// };


// namespace solvernames {
// const auto sample = "c4-sw_sample";
// const auto optimize = "c4-sw_optimize";
// const auto heuristic = "ising-heuristic";
// } // namespace solvernames

sapi::SolverMap localSolverMap() {
  auto ret = sapi::SolverMap();
//   ret[solvernames::sample] = SolverPtr(new LocalC4SampleSolver);
//   ret[solvernames::optimize] = SolverPtr(new LocalC4OptimizeSolver);
//   ret[solvernames::heuristic] = SolverPtr(new LocalIsingHeuristicSolver);
  return ret;
}

class LocalConnection : public sapi_Connection {
public:
  LocalConnection() :sapi_Connection(localSolverMap()) {}
};

} // namespace sapi


#endif
