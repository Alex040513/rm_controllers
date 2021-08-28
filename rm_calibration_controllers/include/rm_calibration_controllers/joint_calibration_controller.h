/*******************************************************************************
 * BSD 3-Clause License
 *
 * Copyright (c) 2021, Qiayuan Liao
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************/

//
// Created by qiayuan on 5/16/21.
//

#ifndef RM_CALIBRATION_CONTROLLERS_JOINT_CALIBRATION_CONTROLLER_H_
#define RM_CALIBRATION_CONTROLLERS_JOINT_CALIBRATION_CONTROLLER_H_
#include <ros/ros.h>
#include <controller_interface/multi_interface_controller.h>
#include <rm_common/hardware_interface/actuator_extra_interface.h>
#include <hardware_interface/joint_command_interface.h>
#include <effort_controllers/joint_velocity_controller.h>
#include <control_msgs/QueryCalibrationState.h>

namespace rm_calibration_controllers
{
class JointCalibrationController
  : public controller_interface::MultiInterfaceController<hardware_interface::EffortJointInterface,
                                                          hardware_interface::ActuatorExtraInterface>
{
public:
  JointCalibrationController() = default;
  /** @brief Get necessary params from param server. Init joint_calibration_controller.
   *
   * Get params from param server and check whether these params are set.Init JointVelocityController.Check
   * whether threshold is set correctly.
   *
   * @param robot_hw Robot hardware.
   * @param root_nh Root node-handle of a ROS node.
   * @param controller_nh Controller node handle of a ROS node.
   * @return True if init successful, false when failed.
   */
  bool init(hardware_interface::RobotHW* robot_hw, ros::NodeHandle& root_nh, ros::NodeHandle& controller_nh) override;
  /** @brief Execute corresponding action according to current calibration controller state.
   *
   * Execute corresponding action according to current joint state. If INITIALIZED, target joint will be set
   * a vel_search_ and countdown_ to move, and switch state to MOVING. If MOVING, target joint will move until
   * current velocity lower than threshold last for a while, and switch state to CALIBRATED. If CALIBRATED,
   * target joint velocity will be set to zero and wait for next command.
   *
   * @param time Current time.
   * @param period Current time - last time.
   */
  void update(const ros::Time& time, const ros::Duration& period) override;
  /** @brief Switch all of the actuators state to INITIALIZED.
   *
   * Switch all of the actuator state to INITIALIZED in order to restart the calibration.
   *
   * @param time Current time.
   */
  void starting(const ros::Time& time) override;

private:
  /** @brief Provide a service to know the state of target actuators.
   *
   * When requesting to this server, it will return respond about whether target actuators has been calibrated.
   *
   * @param req The request of knowing the state of target motors.
   * @param resp The respond included the state of target motors.
   * @return True if get respond successfully.
   */
  bool isCalibrated(control_msgs::QueryCalibrationState::Request& req,
                    control_msgs::QueryCalibrationState::Response& resp);
  ros::Time last_publish_time_;
  ros::ServiceServer is_calibrated_srv_;
  //  enum { INITIALIZED, BEGINNING, MOVING_TO_LOW, MOVING_TO_HIGH, CALIBRATED }; for GPIO switch
  enum
  {
    INITIALIZED,
    MOVING,
    CALIBRATED
  };
  int state_{}, countdown_{};
  double vel_search_{}, threshold_{};
  std::vector<hardware_interface::ActuatorExtraHandle> actuators_;
  effort_controllers::JointVelocityController velocity_ctrl_;
};

}  // namespace rm_calibration_controllers
#endif  // RM_CALIBRATION_CONTROLLERS_JOINT_CALIBRATION_CONTROLLER_H_
