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

#include <atomic>
#include <boost/lexical_cast.hpp>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <status_monitor/status_monitor.hpp>

namespace common
{
static std::size_t count = 0;

StatusMonitor::StatusMonitor()
{
  if (not count++) {
    statuses = {};

    watchdog = std::thread([this]() {
      if (file.open("/tmp/monitor-" + boost::lexical_cast<std::string>(std::this_thread::get_id()));
          not file.is_open()) {
        std::cout << "FILE OPEN FAILED!!!" << std::endl;
      }

      while (not terminating.load(std::memory_order_acquire)) {
        std::this_thread::sleep_for(std::chrono::seconds(1));

        std::stringstream ss;

        ss << "WATCHDOG[" << std::this_thread::get_id() << "]\n";

        if (not statuses.empty()) {
          for (auto && [id, status] : statuses) {
            // clang-format off
            ss << "  thread-id[" << id << "]\n"
               << "    elapsed: " << status.elapsed_time_since_last_access<std::chrono::milliseconds>().count() << " [ms]\n"
               << "    status.ok(): " << std::boolalpha << status.ok() << "\n";
            // clang-format on
          }
        }

        file << ss.str() << std::flush;
      }

      std::cout << "WATCHDOG[" << std::this_thread::get_id() << "] TERMINATED" << std::endl;
    });
  }
}

StatusMonitor::~StatusMonitor()
{
  if (auto iter = statuses.find(std::this_thread::get_id()); iter != std::end(statuses)) {
    statuses.erase(iter);
  }

  if (not --count) {
    terminating.store(true, std::memory_order_release);
    watchdog.join();
    terminating.store(false, std::memory_order_release);

    file.close();
  }
}
}  // namespace common
