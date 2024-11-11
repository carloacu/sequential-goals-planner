cmake_minimum_required(VERSION 3.8)

project(prioritized_goals_planner_bin)

include_directories(".")

if(NOT CONTEXTUALPLANNER_FOUND)
  find_package(prioritized_goals_planner_lib)
endif()


add_executable(
  prioritized_goals_planner

  src/main.cpp
)


target_link_libraries(prioritized_goals_planner
  ${CONTEXTUALPLANNER_LIBRARIES}
)
