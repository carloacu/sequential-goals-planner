cmake_minimum_required(VERSION 2.8)

project(contextualplanner_bin)

include_directories(".")

if(NOT CONTEXTUALPLANNER_FOUND)
  find_package(contextualplannerlib)
endif()


add_executable(
  contextualplanner

  src/main.cpp
)


target_link_libraries(contextualplanner
  ${CONTEXTUALPLANNER_LIBRARIES}
)
