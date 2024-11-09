cmake_minimum_required(VERSION 3.8)

project(prioritizedgoalsplanner_bin)

include_directories(".")

if(NOT CONTEXTUALPLANNER_FOUND)
  find_package(prioritizedgoalsplannerlib)
endif()


add_executable(
  prioritizedgoalsplanner

  src/main.cpp
)


target_link_libraries(prioritizedgoalsplanner
  ${CONTEXTUALPLANNER_LIBRARIES}
)
