// Copyright 2015-2021 Tier IV, Inc. All rights reserved.
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

#include <openscenario_interpreter/procedure.hpp>
#include <openscenario_interpreter/syntax/reach_position_condition.hpp>
#include <openscenario_interpreter/utility/overload.hpp>
#include <openscenario_interpreter/utility/print.hpp>
#include <traffic_simulator/helper/helper.hpp>

namespace openscenario_interpreter
{
inline namespace syntax
{
auto ReachPositionCondition::description() const -> String
{
  std::stringstream description;

  description << triggering_entities.description() << "'s distance to given position = ";

  print_to(description, results);

  description << " " << compare << " " << tolerance << "?";

  return description.str();
}

auto ReachPositionCondition::evaluate() -> Element
{
  const auto reach_position = overload(
    [](const WorldPosition & position, auto && triggering_entity, auto && tolerance) {
      return evaluateReachPositionCondition(
        triggering_entity, static_cast<geometry_msgs::msg::Pose>(position), tolerance);
    },
    [](const RelativeWorldPosition & position, auto && triggering_entity, auto && tolerance) {
      return evaluateReachPositionCondition(
        triggering_entity, static_cast<openscenario_msgs::msg::LaneletPose>(position), tolerance);
    },
    [](const LanePosition & position, auto && triggering_entity, auto && tolerance) {
      return evaluateReachPositionCondition(
        triggering_entity, static_cast<openscenario_msgs::msg::LaneletPose>(position), tolerance);
    });

  // TODO USE DistanceCondition::distance
  const auto distance = overload(
    [&](const WorldPosition & position, auto && triggering_entity) {
      const auto pose =
        getRelativePose(triggering_entity, static_cast<geometry_msgs::msg::Pose>(position));
      return std::hypot(pose.position.x, pose.position.y);
    },
    [&](const RelativeWorldPosition & position, auto && triggering_entity) {
      const auto pose =
        getRelativePose(triggering_entity, static_cast<geometry_msgs::msg::Pose>(position));
      return std::hypot(pose.position.x, pose.position.y);
    },
    [&](const LanePosition & position, auto && triggering_entity) {
      const auto pose =
        getRelativePose(triggering_entity, static_cast<geometry_msgs::msg::Pose>(position));
      return std::hypot(pose.position.x, pose.position.y);
    });

  results.clear();

  return asBoolean(triggering_entities.apply([&](const auto & triggering_entity) {
    results.push_back(apply<Double>(distance, position, triggering_entity));
    return apply<bool>(reach_position, position, triggering_entity, tolerance);
  }));
}
}  // namespace syntax
}  // namespace openscenario_interpreter
