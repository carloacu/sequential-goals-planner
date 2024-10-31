cmake_minimum_required(VERSION 2.8)

project(contextualplanner_tests)

include_directories(".")

if(NOT CONTEXTUALPLANNER_FOUND)
  find_package(contextualplannerlib)
endif()

enable_testing()
find_package(GTest REQUIRED)


add_executable(
  contextualplanner_tests

  src/plannerusingexternaldata.hpp

  src/test_planner.cpp
  src/test_facttoconditions.cpp
  src/test_plannerWithSingleType.cpp
  src/test_ontology.cpp
  src/test_pddl_serialization.cpp
  src/test_setoffacts.cpp
  src/test_arithmeticevaluator.cpp
  src/test_successionscache.cpp
  src/docexamples/test_planningDummyExample.hpp
  src/docexamples/test_planningDummyExample.cpp
  src/docexamples/test_planningExampleWithAPreconditionSolve.hpp
  src/docexamples/test_planningExampleWithAPreconditionSolve.cpp
  src/test_util.cpp
  src/test_pddl_serialization.cpp
  src/test_problems_in_data.cpp
  src/main.cpp
)


target_link_libraries(contextualplanner_tests
  ${GTEST_LIBRARIES}
  ${CONTEXTUALPLANNER_LIBRARIES}
)

add_test(
  NAME contextualplanner_tests
  COMMAND contextualplanner_tests
)
