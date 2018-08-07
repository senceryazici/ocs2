/*
 * RosMsgConversions.h
 *
 *  Created on: May 24, 2018
 *      Author: farbod
 */

#ifndef ROSMSGCONVERSIONS_OCS2_H_
#define ROSMSGCONVERSIONS_OCS2_H_

#include <ocs2_core/Dimensions.h>
#include <ocs2_core/cost/CostDesiredTrajectories.h>

#include "ocs2_comm_interfaces/SystemObservation.h"

// MPC messages
#include <ocs2_comm_interfaces/mode_sequence.h>
#include <ocs2_comm_interfaces/mpc_observation.h>
#include <ocs2_comm_interfaces/mpc_target_trajectories.h>

namespace ocs2{

/**
 * This class converts MPC data to ROS messages and the reveres.
 *
 * @tparam STATE_DIM: Dimension of the state space.
 * @tparam INPUT_DIM: Dimension of the control input space.
 */
template <size_t STATE_DIM, size_t INPUT_DIM>
class RosMsgConversions
{
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

	typedef Dimensions<STATE_DIM, INPUT_DIM> DIMENSIONS;
	typedef typename DIMENSIONS::scalar_t		scalar_t;
	typedef typename DIMENSIONS::scalar_array_t	scalar_array_t;
	typedef typename DIMENSIONS::size_array_t	size_array_t;

	typedef SystemObservation<STATE_DIM, INPUT_DIM> system_observation_t;

	typedef CostDesiredTrajectories<scalar_t> cost_desired_trajectories_t;
	typedef typename cost_desired_trajectories_t::dynamic_vector_t       dynamic_vector_t;
	typedef typename cost_desired_trajectories_t::dynamic_vector_array_t dynamic_vector_array_t;

	RosMsgConversions() = default;
	~RosMsgConversions() = default;

	/**
	 * Creates the observation message.
	 *
	 * @param [in] observation: The observation structure.
	 * @param [out] observationMsg: The observation message.
	 */
	template <class ContainerAllocator>
	static void CreateObservationMsg(
			const system_observation_t& observation,
			ocs2_comm_interfaces::mpc_observation_<ContainerAllocator>& observationMsg);

	/**
	 * Reads the observation message.
	 *
	 * @param [in] observationMsg: The observation message.
	 * @param [out] observation: The observation structure.
	 */
	template <class ContainerAllocator>
	static void ReadObservationMsg(
			const ocs2_comm_interfaces::mpc_observation_<ContainerAllocator>& observationMsg,
			system_observation_t& observation);

	/**
	 * Creates the mode sequence message.
	 *
	 * @param [in] eventTimes: The event times sequence.
	 * @param [in] subsystemsSequence: The subsystems sequence.
	 * @param [out] modeSequenceMsg: The mode sequence message.
	 */
	template <class ContainerAllocator>
	static void CreateModeSequenceMsg(
			const scalar_array_t& eventTimes,
			const size_array_t& subsystemsSequence,
			ocs2_comm_interfaces::mode_sequence_<ContainerAllocator>& modeSequenceMsg);

	/**
	 * Reads the mode sequence message.
	 *
	 * @param [in] modeSequenceMsg: The mode sequence message.
	 * @param [out] eventTimes: The event times sequence.
	 * @param [out] subsystemsSequence: The subsystems sequence.
	 */
	template <class ContainerAllocator>
	static void ReadModeSequenceMsg(
			const ocs2_comm_interfaces::mode_sequence_<ContainerAllocator>& modeSequenceMsg,
			scalar_array_t& eventTimes,
			size_array_t& subsystemsSequence);

	/**
	 * Creates the target trajectories message.
	 *
	 * @param [in] costDesiredTrajectories: The desired trajectory of the cost.
	 * @param [out] targetTrajectoriesMsg: The target trajectories message.
	 */
	template <class ContainerAllocator>
	static void CreateTargetTrajectoriesMsg(
			const cost_desired_trajectories_t& costDesiredTrajectories,
			ocs2_comm_interfaces::mpc_target_trajectories_<ContainerAllocator>& targetTrajectoriesMsg);

	/**
	 * Reads the target trajectories message.
	 *
	 * @param [in] targetTrajectoriesMsg: The target trajectories message.
	 * @param [out] costDesiredTrajectories: The desired trajectory of the cost.
	 */
	template <class ContainerAllocator>
	static void ReadTargetTrajectoriesMsg(
			const ocs2_comm_interfaces::mpc_target_trajectories_<ContainerAllocator>& targetTrajectoriesMsg,
			cost_desired_trajectories_t& costDesiredTrajectories);

};

} // namespace ocs2

#include "implementation/RosMsgConversions.h"

#endif /* ROSMSGCONVERSIONS_OCS2_H_ */