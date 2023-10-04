// Copyright 2015 TIER IV, Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <quaternion_operation/quaternion_operation.h>

#include <ament_index_cpp/get_package_share_directory.hpp>
#include <cpp_mock_scenarios/catalogs.hpp>
#include <cpp_mock_scenarios/cpp_scenario_node.hpp>
#include <random001_parameters.hpp>
#include <rclcpp/rclcpp.hpp>
#include <traffic_simulator/api/api.hpp>
#include <traffic_simulator_msgs/msg/behavior_parameter.hpp>

// headers in STL
#include <memory>
#include <string>
#include <vector>

class RandomScenario : public cpp_mock_scenarios::CppScenarioNode
{
public:
  explicit RandomScenario(const rclcpp::NodeOptions & option)
  : cpp_mock_scenarios::CppScenarioNode(
      "lanechange_left", ament_index_cpp::get_package_share_directory("kashiwanoha_map") + "/map",
      "lanelet2_map.osm", __FILE__, false, option),
    param_listener_(std::make_shared<random001::ParamListener>(get_node_parameters_interface()))
  {
    start();
  }

private:
  std::shared_ptr<random001::ParamListener> param_listener_;
  random001::Params params_;
  void onUpdate() override
  {
    [&]() {
      if (param_listener_->is_old(params_)) {
        param_listener_->refresh_dynamic_parameters();
        params_ = param_listener_->get_params();
      }
    }();

    const auto spawn_and_change_lane = [&](const auto & entity_name, const auto spawn_s_value) {
      api_.spawn(
        entity_name,
        api_.canonicalize(
          traffic_simulator::helper::constructLaneletPose(34513, spawn_s_value, 0, 0, 0, 0)),
        getVehicleParameters());
      api_.requestSpeedChange(entity_name, 10, true);
      api_.setLinearVelocity(entity_name, 10);
      api_.requestLaneChange(entity_name, traffic_simulator::lane_change::Direction::RIGHT);
    };

    if (api_.isInLanelet("ego", 34684, 0.1)) {
      if (!api_.entityExists("lane_following_0")) {
        spawn_and_change_lane("lane_following_0", 0.0);
      }
      if (!api_.entityExists("lane_following_1")) {
        spawn_and_change_lane("lane_following_1", 7.0);
      }
    }
    if (api_.isInLanelet("ego", 34606, 0.1)) {
      api_.requestAcquirePosition(
        "ego",
        api_.canonicalize(traffic_simulator::helper::constructLaneletPose(34681, 0, 0, 0, 0, 0)));
    }
    if (api_.isInLanelet("ego", 34681, 0.1)) {
      api_.requestAcquirePosition(
        "ego",
        api_.canonicalize(traffic_simulator::helper::constructLaneletPose(34606, 0, 0, 0, 0, 0)));
    }

    const auto spawn_and_cross_pedestrian = [&](const auto & entity_name, const auto lanelet_id) {
      if (
        !api_.entityExists(entity_name) &&
        !api_.reachPosition(
          "ego", api_.canonicalize(traffic_simulator::helper::constructLaneletPose(34576, 25.0)),
          5.0)) {
        api_.spawn(
          entity_name,
          api_.canonicalize(traffic_simulator::helper::constructLaneletPose(lanelet_id, 0.0)),
          getPedestrianParameters());
      }
    };
    spawn_and_cross_pedestrian("pedestrian_0", 34385);
    spawn_and_cross_pedestrian("pedestrian_1", 34392);
  }
  void onInitialize() override
  {
    params_ = param_listener_->get_params();

    const auto spawn_road_parking_vehicle =
      [&](const auto & entity_name, const auto spawn_s_value, const auto offset) {
        api_.spawn(
          entity_name,
          api_.canonicalize(
            traffic_simulator::helper::constructLaneletPose(34705, spawn_s_value, offset, 0, 0, 0)),
          getVehicleParameters());
        api_.requestSpeedChange(entity_name, 0, true);
      };
    spawn_road_parking_vehicle("road_parking_0", 10.0, 2.3);
    spawn_road_parking_vehicle("road_parking_1", 5.0, 2.3);

    api_.spawn(
      "ego",
      api_.canonicalize(traffic_simulator::helper::constructLaneletPose(34621, 10, 0, 0, 0, 0)),
      getVehicleParameters());
    api_.requestAcquirePosition(
      "ego",
      api_.canonicalize(traffic_simulator::helper::constructLaneletPose(34606, 0, 0, 0, 0, 0)));
    api_.requestSpeedChange("ego", 10, true);
    api_.setLinearVelocity("ego", 10);
  }
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::NodeOptions options;
  auto component = std::make_shared<RandomScenario>(options);
  rclcpp::spin(component);
  rclcpp::shutdown();
  return 0;
}
