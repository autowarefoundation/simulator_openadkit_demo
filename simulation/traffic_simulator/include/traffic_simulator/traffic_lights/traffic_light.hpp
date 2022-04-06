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

#ifndef TRAFFIC_SIMULATOR__TRAFFIC_LIGHTS__TRAFFIC_LIGHT_HPP_
#define TRAFFIC_SIMULATOR__TRAFFIC_LIGHTS__TRAFFIC_LIGHT_HPP_

#include <autoware_auto_perception_msgs/msg/traffic_signal.hpp>
#include <cstdint>
#include <geometry_msgs/msg/point.hpp>
#include <iostream>
#include <limits>
#include <memory>
#include <regex>
#include <set>
#include <stdexcept>
#include <traffic_simulator/color_utils/color_utils.hpp>
#include <traffic_simulator/hdmap_utils/hdmap_utils.hpp>
#include <traffic_simulator/traffic_lights/traffic_light_state.hpp>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

namespace traffic_simulator
{
// clang-format off
struct TrafficLight_
{
  struct Color
  {
    enum Value : std::uint8_t { amber, green, red, white, } value;

    static_assert(static_cast<std::uint8_t>(amber) == 0b0000);
    static_assert(static_cast<std::uint8_t>(green) == 0b0001);
    static_assert(static_cast<std::uint8_t>(red  ) == 0b0010);
    static_assert(static_cast<std::uint8_t>(white) == 0b0011);

    constexpr Color(const Value value = green) : value(value) {}

    Color(const std::string & name)
      : value(make(name))
    {}

    static inline const std::unordered_map<std::string, Value> table
    {
      std::make_pair("amber", amber),
      std::make_pair("green", green),
      std::make_pair("red", red),
      std::make_pair("white", white),
      std::make_pair("yellow", amber),  // DEPRECATED
    };

    static auto make(const std::string & name) -> Color
    {
      try {
        return table.at(name);
      } catch (const std::out_of_range &) {
        throw common::SyntaxError("Invalid traffic light color name ", std::quoted(name), " given.");
      }
    }

    constexpr auto is(const Color given) const
    {
      return value == given;
    }

    constexpr operator Value() const noexcept { return value; }

    friend auto operator>>(std::istream & is, Color & color) -> std::istream &
    {
      std::string name;
      is >> name;
      color.value = Color::make(name);
      return is;
    }

    friend auto operator<<(std::ostream & os, const Color & color) -> std::ostream &
    {
      switch (color.value) {
        case amber:
          return os << "amber";
        case green:
          return os << "green";
        case red:
          return os << "red";
        case white:
          return os << "white";
        default:
          return os;
      }
    }
  };

  struct Status
  {
    enum Value : std::uint8_t { solid_on, solid_off, flashing, unknown, } value;

    static_assert(static_cast<std::uint8_t>(solid_on ) == 0b0000);
    static_assert(static_cast<std::uint8_t>(solid_off) == 0b0001);
    static_assert(static_cast<std::uint8_t>(flashing ) == 0b0010);
    static_assert(static_cast<std::uint8_t>(unknown  ) == 0b0011);

    constexpr Status(const Value value = solid_on) : value(value) {}

    Status(const std::string & name)
      : value(make(name))
    {}

    static inline const std::unordered_map<std::string, Value> table
    {
      std::make_pair("solidOn", solid_on),
      std::make_pair("solidOff", solid_off),
      std::make_pair("flashing", flashing),
      std::make_pair("unknown", unknown),
    };

    static auto make(const std::string & name) -> Status
    {
      try {
        return table.at(name);
      } catch (const std::out_of_range &) {
        throw common::SyntaxError("Invalid traffic light status name ", std::quoted(name), " given.");
      }
    }

    constexpr auto is(const Value given) const
    {
      return value == given;
    }

    constexpr operator Value() const noexcept { return value; }

    friend auto operator>>(std::istream & is, Status & status) -> std::istream &
    {
      std::string name;
      is >> name;
      status.value = Status::make(name);
      return is;
    }

    friend auto operator<<(std::ostream & os, const Status & status) -> std::ostream &
    {
      switch (status.value) {
        case solid_on:
          return os << "solidOn";
        case solid_off:
          return os << "solidOff";
        case flashing:
          return os << "flashing";
        default:
        case unknown:
          return os << "unknown";
      }
    }

    constexpr operator bool() const
    {
      return value == solid_on or value == flashing;
    }
  };

  struct Shape
  {
    enum class Category : std::uint8_t
    {
      circle, cross, arrow,
    };

    static_assert(static_cast<std::uint8_t>(Category::circle) == 0b0000);
    static_assert(static_cast<std::uint8_t>(Category::cross ) == 0b0001);
    static_assert(static_cast<std::uint8_t>(Category::arrow ) == 0b0010);

    enum Value : std::uint16_t
    {
      circle      =                 static_cast<std::uint8_t>(Category::circle),
      cross       =                 static_cast<std::uint8_t>(Category::cross ),
      left        = (0b1000 << 4) | static_cast<std::uint8_t>(Category::arrow ),
      down        = (0b0100 << 4) | static_cast<std::uint8_t>(Category::arrow ),
      up          = (0b0010 << 4) | static_cast<std::uint8_t>(Category::arrow ),
      right       = (0b0001 << 4) | static_cast<std::uint8_t>(Category::arrow ),
      lower_left  = (0b1100 << 4) | static_cast<std::uint8_t>(Category::arrow ),
      upper_left  = (0b1010 << 4) | static_cast<std::uint8_t>(Category::arrow ),
      lower_right = (0b0101 << 4) | static_cast<std::uint8_t>(Category::arrow ),
      upper_right = (0b0011 << 4) | static_cast<std::uint8_t>(Category::arrow ),
    } value;

    static_assert(static_cast<std::uint16_t>(circle     ) == 0b0000'0000);
    static_assert(static_cast<std::uint16_t>(cross      ) == 0b0000'0001);
    static_assert(static_cast<std::uint16_t>(left       ) == 0b1000'0010);
    static_assert(static_cast<std::uint16_t>(down       ) == 0b0100'0010);
    static_assert(static_cast<std::uint16_t>(up         ) == 0b0010'0010);
    static_assert(static_cast<std::uint16_t>(right      ) == 0b0001'0010);
    static_assert(static_cast<std::uint16_t>(lower_left ) == 0b1100'0010);
    static_assert(static_cast<std::uint16_t>(upper_left ) == 0b1010'0010);
    static_assert(static_cast<std::uint16_t>(lower_right) == 0b0101'0010);
    static_assert(static_cast<std::uint16_t>(upper_right) == 0b0011'0010);

    constexpr Shape(const Value value = circle) : value(value) {}

    Shape(const std::string & name)
      : value(make(name))
    {}

    static inline const std::unordered_map<std::string, Shape::Value> table
    {
      std::make_pair("circle",     Shape::circle),
      std::make_pair("cross",      Shape::cross),
      std::make_pair("left",       Shape::left),
      std::make_pair("down",       Shape::down),
      std::make_pair("up",         Shape::up),
      std::make_pair("right",      Shape::right),
      std::make_pair("lowerLeft",  Shape::lower_left),
      std::make_pair("upperLeft",  Shape::upper_left),
      std::make_pair("lowerRight", Shape::lower_right),
      std::make_pair("upperRight", Shape::upper_right),
    };

    static auto make(const std::string & name) -> Shape
    {
      try {
        return table.at(name);
      } catch (const std::out_of_range &) {
        throw common::SyntaxError("Invalid traffic light shape name ", std::quoted(name), " given.");
      }
    }

    constexpr auto category() const
    {
      return static_cast<Category>(static_cast<std::uint16_t>(value) & 0b1111);
    }

    constexpr auto is(const Value given) const
    {
      return value == given;
    }

    constexpr auto is(const Category given) const
    {
      return category() == given;
    }

    constexpr operator Value() const noexcept { return value; }

    friend auto operator>>(std::istream & is, Shape & shape) -> std::istream &
    {
      std::string name;
      is >> name;
      shape.value = Shape::make(name);
      return is;
    }

    friend auto operator<<(std::ostream & os, const Shape & shape) -> std::ostream &
    {
      switch (shape.value) {
        case circle:
          return os << "circle";
        case cross:
          return os << "cross";
        case left:
          return os << "left";
        case down:
          return os << "down";
        case up:
          return os << "up";
        case right:
          return os << "right";
        case lower_left:
          return os << "lowerLeft";
        case upper_left:
          return os << "upperLeft";
        case lower_right:
          return os << "lowerRight";
        case upper_right:
          return os << "upperRight";
        default:
          return os;
      }
    }
  };

  struct Bulb : public std::tuple<Color, Status, Shape>
  {
    constexpr Bulb(const Color color = {}, const Status status = {}, const Shape shape = {})
      : std::tuple<Color, Status, Shape> { color, status, shape }
    {}

    Bulb(const std::string & s)
      : std::tuple<Color, Status, Shape> { parse(s) }
    {}

    auto parse(const std::string & s) -> std::tuple<Color, Status, Shape>
    {
      auto make_pattern_from = [](auto && map)
      {
        std::stringstream ss;

        auto const * separator = "";

        for (auto && [name, value] : map)
        {
          ss << separator << name;
          separator = "|";
        }

        return "(" + ss.str() + ")";
      };


      static const auto pattern = std::regex(
        R"(^)" + make_pattern_from(Color::table)  + R"(?\s*)"
               + make_pattern_from(Status::table) + R"(?\s*)"
               + make_pattern_from(Shape::table)  + R"(?$)"
      );

      if (std::smatch result; std::regex_match(s, result, pattern))
      {
        auto color = [](auto && name)
        {
          return name.empty() ? Color() : Color(name);
        };

        auto status = [](auto && name)
        {
          return name.empty() ? Status() : Status(name);
        };

        auto shape = [](auto && name)
        {
          return name.empty() ? Shape() : Shape(name);
        };

        return std::make_tuple(color(result.str(1)), status(result.str(2)), shape(result.str(3)));
      }
      else
      {
        throw common::SyntaxError("");
      }
    }

    constexpr auto is(const Color color) const
    {
      return std::get<Color>(*this).is(color);
    }

    constexpr auto is(const Status status) const
    {
      return std::get<Status>(*this).is(status);
    }

    constexpr auto is(const Shape shape) const
    {
      return std::get<Shape>(*this).is(shape);
    }

    constexpr auto is(const Shape::Category category) const
    {
      return std::get<Shape>(*this).is(category);
    }

    constexpr auto value() const -> std::uint32_t
    {
      return (static_cast<std::uint32_t>(std::get<Color>(*this).value) << 12) |
             (static_cast<std::uint32_t>(std::get<Status>(*this).value) << 8) |
             static_cast<std::uint32_t>(std::get<Shape>(*this).value);
    }

    friend constexpr auto operator <(const Bulb & lhs, const Bulb & rhs) -> bool
    {
      return lhs.value() < rhs.value();
    }

    explicit operator autoware_auto_perception_msgs::msg::TrafficLight() const
    {
      auto color = [this]() {
        switch (std::get<Color>(*this).value) {
          case Color::amber:
            return autoware_auto_perception_msgs::msg::TrafficLight::AMBER;
          case Color::green:
            return autoware_auto_perception_msgs::msg::TrafficLight::GREEN;
          case Color::red:
            return autoware_auto_perception_msgs::msg::TrafficLight::RED;
          case Color::white:
            return autoware_auto_perception_msgs::msg::TrafficLight::WHITE;
        }
      };

      auto status = [this]() {
        switch (std::get<Status>(*this).value) {
          case Status::solid_on:
            return autoware_auto_perception_msgs::msg::TrafficLight::SOLID_ON;
          case Status::solid_off:
            return autoware_auto_perception_msgs::msg::TrafficLight::SOLID_OFF;
          case Status::flashing:
            return autoware_auto_perception_msgs::msg::TrafficLight::FLASHING;
          case Status::unknown:
            return autoware_auto_perception_msgs::msg::TrafficLight::UNKNOWN;
        }
      };

      auto shape = [this]() {
        switch (std::get<Shape>(*this).value) {
          case Shape::circle:
            return autoware_auto_perception_msgs::msg::TrafficLight::CIRCLE;
          case Shape::cross:
            return autoware_auto_perception_msgs::msg::TrafficLight::CROSS;
          case Shape::left:
            return autoware_auto_perception_msgs::msg::TrafficLight::LEFT_ARROW;
          case Shape::down:
            return autoware_auto_perception_msgs::msg::TrafficLight::DOWN_ARROW;
          case Shape::up:
            return autoware_auto_perception_msgs::msg::TrafficLight::UP_ARROW;
          case Shape::right:
            return autoware_auto_perception_msgs::msg::TrafficLight::RIGHT_ARROW;
          case Shape::lower_left:
            return autoware_auto_perception_msgs::msg::TrafficLight::DOWN_LEFT_ARROW;
          case Shape::lower_right:
            return autoware_auto_perception_msgs::msg::TrafficLight::DOWN_RIGHT_ARROW;
          default:
            throw common::SyntaxError(
              std::get<Shape>(*this), " is not supported as a shape for autoware_auto_perception_msgs::msg::TrafficLight.");
        }
      };

      autoware_auto_perception_msgs::msg::TrafficLight traffic_light;
      traffic_light.color = color();
      traffic_light.status = status();
      traffic_light.shape = shape();
      traffic_light.confidence = 1.0;
      return traffic_light;
    }
  };

  const std::int64_t id;

  std::set<Bulb> bulbs;

  explicit TrafficLight_(const std::int64_t id, hdmap_utils::HdMapUtils & map_manager)
  : id(id)
  {
    if (not map_manager.isTrafficLight(id)) {
      throw common::scenario_simulator_exception::Error("Invalid traffic light ID ", id, " given.");
    }
  }

  auto clear()
  {
    bulbs.clear();
  }

  auto contains(const Color & color, const Status & status, const Shape & shape) const
  {
    return bulbs.find({color, status, shape}) != std::end(bulbs);
  }

  template <typename... Ts>
  auto emplace(Ts&&... xs)
  {
    bulbs.emplace(std::forward<decltype(xs)>(xs)...);
  }

  explicit operator autoware_auto_perception_msgs::msg::TrafficSignal() const
  {
    autoware_auto_perception_msgs::msg::TrafficSignal traffic_signal;
    traffic_signal.map_primitive_id = 42;
    for (auto && bulb : bulbs) {
      traffic_signal.lights.push_back(
        static_cast<autoware_auto_perception_msgs::msg::TrafficLight>(bulb));
    }
    return traffic_signal;
  }
};
// clang-format on

class TrafficLight
{
public:
  const std::int64_t id;

  explicit TrafficLight(const std::int64_t id, hdmap_utils::HdMapUtils & map_manager)
  : id(id), color_(TrafficLightColor::GREEN), arrow_(TrafficLightArrow::NONE)
  {
    auto locate = [&](auto && color) {
      if (const auto position = map_manager.getTrafficLightBulbPosition(id, color)) {
        color_positions_.emplace(color, position.get());
      }
    };

    if (map_manager.isTrafficLight(id)) {
      locate(TrafficLightColor::GREEN);
      locate(TrafficLightColor::RED);
      locate(TrafficLightColor::YELLOW);
    }
  }

  void setArrow(const TrafficLightArrow arrow)
  {
    arrow_ = arrow;
    arrow_changed_ = true;
  }

  void setColor(const TrafficLightColor color)
  {
    color_ = color;
    color_changed_ = true;
  }

  void update(const double) { arrow_changed_ = color_changed_ = false; }

  auto getArrow() const { return arrow_; }
  auto getColor() const { return color_; }

  const geometry_msgs::msg::Point & getPosition(const TrafficLightColor & color) const;
  const geometry_msgs::msg::Point & getPosition(const TrafficLightArrow & arrow) const;

  auto colorChanged() const { return color_changed_; }
  auto arrowChanged() const { return arrow_changed_; }

  explicit operator autoware_auto_perception_msgs::msg::TrafficSignal() const
  {
    autoware_auto_perception_msgs::msg::TrafficSignal traffic_light_state;
    {
      traffic_light_state.map_primitive_id = id;

      try {
        traffic_light_state.lights.push_back(
          convert<autoware_auto_perception_msgs::msg::TrafficLight>(getArrow()));
      } catch (const std::out_of_range &) {
        // NOTE: The traffic light is in Autoware-incompatible state; ignore it.
      }

      try {
        traffic_light_state.lights.push_back(
          convert<autoware_auto_perception_msgs::msg::TrafficLight>(getColor()));
      } catch (const std::out_of_range &) {
        // NOTE: The traffic light is in Autoware-incompatible state; ignore it.
      }
    }

    return traffic_light_state;
  }

private:
  std::unordered_map<TrafficLightColor, geometry_msgs::msg::Point> color_positions_;
  std::unordered_map<TrafficLightArrow, geometry_msgs::msg::Point> arrow_positions_;

  TrafficLightColor color_;
  TrafficLightArrow arrow_;

  bool color_changed_ = false;
  bool arrow_changed_ = false;
};
}  // namespace traffic_simulator

#endif  // TRAFFIC_SIMULATOR__TRAFFIC_LIGHTS__TRAFFIC_LIGHT_HPP_
