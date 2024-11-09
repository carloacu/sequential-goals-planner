get_filename_component(_prioritizedgoalsplanner_root "${CMAKE_CURRENT_LIST_FILE}" PATH)
get_filename_component(_prioritizedgoalsplanner_root "${_prioritizedgoalsplanner_root}" ABSOLUTE)


set(CONTEXTUALPLANNER_FOUND TRUE)

set(
  CONTEXTUALPLANNER_INCLUDE_DIRS
  ${_prioritizedgoalsplanner_root}/include
  CACHE INTERNAL "" FORCE
)

set(
  CONTEXTUALPLANNER_LIBRARIES
  "prioritizedgoalsplannerlib"
)



