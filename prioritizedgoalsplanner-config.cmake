get_filename_component(_prioritizedgoalsplanner_root "${CMAKE_CURRENT_LIST_FILE}" PATH)
get_filename_component(_prioritizedgoalsplanner_root "${_prioritizedgoalsplanner_root}" ABSOLUTE)


set(PRIORITIZEDGOALSPLANNER_FOUND TRUE)

set(
  PRIORITIZEDGOALSPLANNER_INCLUDE_DIRS
  ${_prioritizedgoalsplanner_root}/include
  CACHE INTERNAL "" FORCE
)

set(
  PRIORITIZEDGOALSPLANNER_LIBRARIES
  "prioritized_goals_planner_lib"
)



