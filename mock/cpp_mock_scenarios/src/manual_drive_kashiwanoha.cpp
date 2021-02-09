// Copyright 2015-2020 Tier IV, Inc. All rights reserved.
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

#include <cpp_mock_scenarios/catalogs.hpp>

#include <simulation_api/api/api.hpp>
#include <quaternion_operation/quaternion_operation.h>
#include <ament_index_cpp/get_package_share_directory.hpp>

#include <rclcpp/rclcpp.hpp>

// headers in STL
#include <memory>
#include <vector>
#include <string>

// headers in pugixml
#include "pugixml.hpp"

class ScenarioRunnerMoc : public rclcpp::Node
{
public:
  explicit ScenarioRunnerMoc(const rclcpp::NodeOptions & option)
  : Node("scenario_runner", option),
    api_(this, ament_index_cpp::get_package_share_directory(
        "cargo_delivery") + "/maps/kashiwa/lanelet2_map_with_private_road_and_walkway_ele_fix.osm")
  {
    api_.setVerbose(true);
    api_.initialize(1.0, 0.05);
    pugi::xml_document vehicle_catalog_xml_doc;
    Catalog catalog;
    vehicle_catalog_xml_doc.load_string(catalog.vehicle_catalog_xml.c_str());
    api_.spawn(
      true, "ego", simulation_api::entity::VehicleParameters(
        vehicle_catalog_xml_doc).toRosMsg());
    api_.setEntityStatus(
      "ego",
      simulation_api::helper::constractLaneletPose(35026, 0, -0.591),
      simulation_api::helper::constractActionStatus(0));
    api_.spawn(
      false, "npc", simulation_api::entity::VehicleParameters(
        vehicle_catalog_xml_doc).toRosMsg());
    api_.setEntityStatus(
      "npc",
      simulation_api::helper::constractLaneletPose(35026, 10, 0.0000001),
      simulation_api::helper::constractActionStatus(10));
    api_.setTargetSpeed("npc", 5, true);
    simulation_api_schema::LidarConfiguration configuration;
    configuration.set_entity("ego");
    configuration.set_horizontal_resolution(1.0 / 180.0 * M_PI);
    configuration.set_scan_duration(0.1);
    configuration.set_topic_name("points_raw");
    configuration.add_vertical_angles(-15.0 / 180.0 * M_PI);
    configuration.add_vertical_angles(-13.0 / 180.0 * M_PI);
    configuration.add_vertical_angles(-11.0 / 180.0 * M_PI);
    configuration.add_vertical_angles(-9.0 / 180.0 * M_PI);
    configuration.add_vertical_angles(-7.0 / 180.0 * M_PI);
    configuration.add_vertical_angles(-5.0 / 180.0 * M_PI);
    configuration.add_vertical_angles(-3.0 / 180.0 * M_PI);
    configuration.add_vertical_angles(-1.0 / 180.0 * M_PI);
    configuration.add_vertical_angles(1.0 / 180.0 * M_PI);
    configuration.add_vertical_angles(3.0 / 180.0 * M_PI);
    configuration.add_vertical_angles(5.0 / 180.0 * M_PI);
    configuration.add_vertical_angles(7.0 / 180.0 * M_PI);
    configuration.add_vertical_angles(9.0 / 180.0 * M_PI);
    configuration.add_vertical_angles(11.0 / 180.0 * M_PI);
    configuration.add_vertical_angles(13.0 / 180.0 * M_PI);
    configuration.add_vertical_angles(15.0 / 180.0 * M_PI);
    api_.attachLidarSensor(configuration);
    using namespace std::chrono_literals;
    update_timer_ = this->create_wall_timer(50ms, std::bind(&ScenarioRunnerMoc::update, this));
  }

private:
  void update()
  {
    api_.updateFrame();
    current_time_ = current_time_ + 0.05;
  }
  bool lanechange_excuted_;
  bool target_speed_setted_;
  bool bob_spawned_;
  double current_time_;
  int port_;
  scenario_simulator::API api_;
  rclcpp::TimerBase::SharedPtr update_timer_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::NodeOptions options;
  auto component = std::make_shared<ScenarioRunnerMoc>(options);
  rclcpp::spin(component);
  rclcpp::shutdown();
  return 0;
}
