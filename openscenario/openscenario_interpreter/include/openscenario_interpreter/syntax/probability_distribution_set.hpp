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

#ifndef OPENSCENARIO_INTERPRETER__PROBABILITY_DISTRIBUTION_SET_HPP_
#define OPENSCENARIO_INTERPRETER__PROBABILITY_DISTRIBUTION_SET_HPP_

#include <openscenario_interpreter/syntax/probability_distribution_set_element.hpp>

namespace openscenario_interpreter
{
inline namespace syntax
{
/* ---- ProbabilityDistributionSet ---------------------------------------------
 *
 *  <xsd:complexType name="ProbabilityDistributionSet">
 *    <xsd:sequence>
 *      <xsd:element name="Element" type="ProbabilityDistributionSetElement" maxOccurs="unbounded"/>
 *    </xsd:sequence>
 *  </xsd:complexType>
 *
 * -------------------------------------------------------------------------- */
struct ProbabilityDistributionSet : public ComplexType
{
  const std::list<ProbabilityDistributionSetElement> elements;

  explicit ProbabilityDistributionSet(const pugi::xml_node &, Scope & scope);

  auto evaluate() -> Object {
    return noexcept;
  }
  // TODO: implement evaluate()
  // Use std::discrete_distribution from <random>
};
}  // namespace syntax
}  // namespace openscenario_interpreter
#endif  // OPENSCENARIO_INTERPRETER__PROBABILITY_DISTRIBUTION_SET_HPP_
