/******************************************************************************
Copyright (c) 2020, Ruben Grandia. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of the copyright holder nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
******************************************************************************/

#include <ocs2_core/loopshaping/dynamics/LoopshapingDynamics.h>
#include <ocs2_core/loopshaping/dynamics/LoopshapingDynamicsEliminatePattern.h>
#include <ocs2_core/loopshaping/dynamics/LoopshapingDynamicsInputPattern.h>
#include <ocs2_core/loopshaping/dynamics/LoopshapingDynamicsOutputPattern.h>

namespace ocs2 {

vector_t LoopshapingDynamics::computeFlowMap(scalar_t time, const vector_t& state, const vector_t& input,
                                             const PreComputation* preCompPtr) {
  assert(preCompPtr != nullptr);
  assert(dynamic_cast<const LoopshapingPreComputation*>(preCompPtr) != nullptr);
  const LoopshapingPreComputation& preComp = *reinterpret_cast<const LoopshapingPreComputation*>(preCompPtr);
  const auto* preComp_system = preComp.getSystemPreComputationPtr();
  const auto& x_system = preComp.getSystemState();
  const auto& u_system = preComp.getSystemInput();
  const auto& x_filter = preComp.getFilterState();
  const auto& u_filter = preComp.getFilterInput();

  const vector_t dynamics_system = systemDynamics_->computeFlowMap(time, x_system, u_system, preComp_system);
  const vector_t dynamics_filter = filterFlowmap(x_filter, u_filter, u_system);

  return loopshapingDefinition_->concatenateSystemAndFilterState(dynamics_system, dynamics_filter);
}

vector_t LoopshapingDynamics::computeJumpMap(scalar_t time, const vector_t& state, const PreComputation* preCompPtr) {
  assert(preCompPtr != nullptr);
  assert(dynamic_cast<const LoopshapingPreComputation*>(preCompPtr) != nullptr);
  const LoopshapingPreComputation& preComp = *reinterpret_cast<const LoopshapingPreComputation*>(preCompPtr);
  const auto* preComp_system = preComp.getSystemPreComputationPtr();
  const auto& x_system = preComp.getSystemState();

  const vector_t jumpMap_system = systemDynamics_->computeJumpMap(time, x_system, preComp_system);

  // Filter doesn't Jump
  const auto& jumMap_filter = preComp.getFilterState();

  return loopshapingDefinition_->concatenateSystemAndFilterState(jumpMap_system, jumMap_filter);
}

vector_t LoopshapingDynamics::computeGuardSurfaces(scalar_t time, const vector_t& state) {
  const vector_t x_system = loopshapingDefinition_->getSystemState(state);
  return systemDynamics_->computeGuardSurfaces(time, x_system);
}

VectorFunctionLinearApproximation LoopshapingDynamics::jumpMapLinearApproximation(scalar_t t, const vector_t& x,
                                                                                  const PreComputation* preCompPtr) {
  assert(preCompPtr != nullptr);
  assert(dynamic_cast<const LoopshapingPreComputation*>(preCompPtr) != nullptr);
  const LoopshapingPreComputation& preComp = *reinterpret_cast<const LoopshapingPreComputation*>(preCompPtr);

  // System jump
  const auto& x_system = preComp.getSystemState();
  const auto* preComp_system = preComp.getSystemPreComputationPtr();
  const auto jumpMap_system = systemDynamics_->jumpMapLinearApproximation(t, x_system, preComp_system);

  // Filter doesn't Jump
  const auto& jumMap_filter = preComp.getFilterState();

  VectorFunctionLinearApproximation jumpMap;
  jumpMap.f = loopshapingDefinition_->concatenateSystemAndFilterState(jumpMap_system.f, jumMap_filter);

  jumpMap.dfdx.resize(jumpMap.f.size(), jumpMap.f.size());
  jumpMap.dfdx.topLeftCorner(jumpMap_system.f.size(), x_system.size()) = jumpMap_system.dfdx;
  jumpMap.dfdx.topRightCorner(jumpMap_system.f.size(), jumMap_filter.size()).setZero();
  jumpMap.dfdx.bottomLeftCorner(jumMap_filter.size(), x_system.size()).setZero();
  jumpMap.dfdx.bottomRightCorner(jumMap_filter.size(), jumMap_filter.size()).setIdentity();

  jumpMap.dfdu.resize(jumpMap.f.size(), 0);

  return jumpMap;
}

VectorFunctionLinearApproximation LoopshapingDynamics::guardSurfacesLinearApproximation(scalar_t t, const vector_t& x, const vector_t& u) {
  throw std::runtime_error("[LoopshapingDynamics] Guard surfaces not implemented");
}

vector_t LoopshapingDynamics::flowMapDerivativeTime(scalar_t t, const vector_t& x, const vector_t& u) {
  const vector_t system_df = systemDynamics_->flowMapDerivativeTime(t, x, u);
  const vector_t filter_df = vector_t::Zero(loopshapingDefinition_->getInputFilter().getNumStates());
  return loopshapingDefinition_->concatenateSystemAndFilterState(system_df, filter_df);
}

vector_t LoopshapingDynamics::jumpMapDerivativeTime(scalar_t t, const vector_t& x, const vector_t& u) {
  const vector_t system_dg = systemDynamics_->jumpMapDerivativeTime(t, x, u);
  const vector_t filter_dg = vector_t::Zero(loopshapingDefinition_->getInputFilter().getNumStates());
  return loopshapingDefinition_->concatenateSystemAndFilterState(system_dg, filter_dg);
}

vector_t LoopshapingDynamics::guardSurfacesDerivativeTime(scalar_t t, const vector_t& x, const vector_t& u) {
  throw std::runtime_error("[LoopshapingDynamics] Guard surfaces not implemented");
}

std::unique_ptr<LoopshapingDynamics> LoopshapingDynamics::create(const SystemDynamicsBase& systemDynamics,
                                                                 std::shared_ptr<LoopshapingDefinition> loopshapingDefinition) {
  // wrap the system pre-computation
  std::unique_ptr<LoopshapingPreComputation> preCompPtr(
      new LoopshapingPreComputation(loopshapingDefinition, systemDynamics.getPreComputationPtr()));

  switch (loopshapingDefinition->getType()) {
    case LoopshapingType::outputpattern:
      return std::unique_ptr<LoopshapingDynamics>(
          new LoopshapingDynamicsOutputPattern(systemDynamics, std::move(loopshapingDefinition), std::move(preCompPtr)));
    case LoopshapingType::inputpattern:
      return std::unique_ptr<LoopshapingDynamics>(
          new LoopshapingDynamicsInputPattern(systemDynamics, std::move(loopshapingDefinition), std::move(preCompPtr)));
    case LoopshapingType::eliminatepattern:
      return std::unique_ptr<LoopshapingDynamics>(
          new LoopshapingDynamicsEliminatePattern(systemDynamics, std::move(loopshapingDefinition), std::move(preCompPtr)));
    default:
      throw std::runtime_error("[LoopshapingDynamics::create] invalid loopshaping type");
  }
}

}  // namespace ocs2
