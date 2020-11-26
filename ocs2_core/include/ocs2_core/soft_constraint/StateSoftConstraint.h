/******************************************************************************
Copyright (c) 2020, Farbod Farshidian. All rights reserved.

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

#pragma once

#include <memory>

#include <ocs2_core/Types.h>
#include <ocs2_core/constraint/StateConstraint.h>
#include <ocs2_core/cost/StateCost.h>
#include <ocs2_core/soft_constraint/SoftConstraintPenaltyBase.h>

namespace ocs2 {

class StateSoftConstraint : public StateCost {
 public:
  /**
   * Constructor.
   * @param [in] constraintPtr: A pointer to the constraint which will be enforced as soft constraints.
   * @param [in] penaltyPtr: A pointer to the penalty function on the constraint.
   * @param [in] constraintOrder: The order of constraint's approximation.
   */
  StateSoftConstraint(std::unique_ptr<StateConstraint> constraintPtr, std::unique_ptr<SoftConstraintPenaltyBase> penaltyPtr,
                      ConstraintOrder constraintOrder = ConstraintOrder::Quadratic)
      : constraintPtr_(std::move(constraintPtr)), penaltyPtr_(std::move(penaltyPtr)), constraintOrder_(constraintOrder) {}

  ~StateSoftConstraint() override = default;

  scalar_t getValue(scalar_t time, const vector_t& state, const CostDesiredTrajectories& /* desiredTrajectory */) const override {
    return penaltyPtr_->getValue(constraintPtr_->getValue(time, state));
  }

  ScalarFunctionQuadraticApproximation getQuadraticApproximation(scalar_t time, const vector_t& state,
                                                                 const CostDesiredTrajectories& /* desiredTrajectory */) const override {
    switch (constraintOrder_) {
      case ConstraintOrder::Linear:
        return penaltyPtr_->getQuadraticApproximation(constraintPtr_->getLinearApproximation(time, state));
      case ConstraintOrder::Quadratic:
        return penaltyPtr_->getQuadraticApproximation(constraintPtr_->getQuadraticApproximation(time, state));
    }
  }

 private:
  std::unique_ptr<StateConstraint> constraintPtr_;
  std::unique_ptr<SoftConstraintPenaltyBase> penaltyPtr_;
  ConstraintOrder constraintOrder_;
};

}  // namespace ocs2
