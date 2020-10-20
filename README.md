# ScenarioSimulator.Auto ![ROS2-Dashing](https://github.com/tier4/scenario_simulator.auto/workflows/ROS2-Dashing/badge.svg)

Open scenario interpreter and simple simulator for Autoware.auto 

# How to use
```
ros2 launch scenario_test_runner scenario_test_runner.launch.py workflow:='$(find-pkg-share scenario_test_runner)/workflow_example.yaml' log_directory:='/tmp'
```