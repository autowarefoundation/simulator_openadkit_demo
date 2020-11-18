// Copyright 2015-2020 Autoware Foundation. All rights reserved.
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

#ifndef OPENSCENARIO_INTERPRETER__SYNTAX__SPEED_ACTION_HPP_
#define OPENSCENARIO_INTERPRETER__SYNTAX__SPEED_ACTION_HPP_

#include <openscenario_interpreter/procedure.hpp>
#include <openscenario_interpreter/syntax/rule.hpp>
#include <openscenario_interpreter/syntax/speed_action_target.hpp>
#include <openscenario_interpreter/syntax/transition_dynamics.hpp>

#include <string>
#include <unordered_map>

namespace openscenario_interpreter
{
inline namespace syntax
{
/* ---- SpeedAction ------------------------------------------------------------
 *
 * <xsd:complexType name="SpeedAction">
 *   <xsd:all>
 *     <xsd:element name="SpeedActionDynamics" type="TransitionDynamics"/>
 *     <xsd:element name="SpeedActionTarget" type="SpeedActionTarget"/>
 *   </xsd:all>
 * </xsd:complexType>
 *
 * -------------------------------------------------------------------------- */
struct SpeedAction
{
  Scope inner_scope;

  const TransitionDynamics speed_action_dynamics;

  const SpeedActionTarget speed_action_target;

  template<typename Node>
  explicit SpeedAction(const Node & node, Scope & outer_scope)
  : inner_scope(outer_scope),
    speed_action_dynamics(
      readElement<TransitionDynamics>("SpeedActionDynamics", node, inner_scope)),
    speed_action_target(readElement<SpeedActionTarget>("SpeedActionTarget", node, inner_scope))
  {}

  std::unordered_map<std::string, Boolean> accomplishments;

  auto start()  // XXX UGLY CODE
  {
    if (speed_action_target.is<AbsoluteTargetSpeed>()) {
      for (const auto & each : inner_scope.actors) {
        accomplishments.emplace(each, false);
        switch (speed_action_dynamics.dynamics_shape) {
          case DynamicsShape::linear:
            setTargetSpeed(each, speed_action_target.as<AbsoluteTargetSpeed>().value, true);
            break;

          case DynamicsShape::step:
            // XXX UGLY CODE
            {
              auto status {
                getEntityStatus(each)
              };
              status.twist.linear.x = speed_action_target.as<AbsoluteTargetSpeed>().value;
              setEntityStatus(each, status);
              setTargetSpeed(each, status.twist.linear.x, true);
            }
            break;

          default:
            THROW(ImplementationFault);
        }
      }
      return unspecified;
    } else {
      THROW(ImplementationFault);
    }
  }

  auto accomplished()
  {
    if (speed_action_target.is<AbsoluteTargetSpeed>()) {
      for (auto && each : accomplishments) {
        if (!cdr(each)) {
          try {
            cdr(each) = Rule(Rule::equalTo)(
              getEntityStatus(
                car(each)).twist.linear.x, speed_action_target.as<AbsoluteTargetSpeed>().value);
          } catch (const SemanticError &) {  // XXX DIRTY HACK!!!
            // NOTE maybe lane-changing
            cdr(each) = false;
          }
        }
      }
      return std::all_of(std::begin(accomplishments), std::end(accomplishments), cdr);
    } else {
      THROW(ImplementationFault);
    }
  }
};
}
}  // namespace openscenario_interpreter

#endif  // OPENSCENARIO_INTERPRETER__SYNTAX__SPEED_ACTION_HPP_