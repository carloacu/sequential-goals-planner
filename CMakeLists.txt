project(prioritizedgoalsplanner)
cmake_minimum_required(VERSION 3.8)

include(prioritizedgoalsplanner-config.cmake)

# Making a lib
set(CONTEXTUAL_PLANNER_HPPS
    include/prioritizedgoalsplanner/types/action.hpp
    include/prioritizedgoalsplanner/types/actioninvocation.hpp
    include/prioritizedgoalsplanner/types/actioninvocationwithgoal.hpp
    include/prioritizedgoalsplanner/types/axiom.hpp
    include/prioritizedgoalsplanner/types/condition.hpp
    include/prioritizedgoalsplanner/types/derivedpredicate.hpp
    include/prioritizedgoalsplanner/types/domain.hpp
    include/prioritizedgoalsplanner/types/entity.hpp
    include/prioritizedgoalsplanner/types/event.hpp
    include/prioritizedgoalsplanner/types/fact.hpp
    include/prioritizedgoalsplanner/types/factoptional.hpp
    include/prioritizedgoalsplanner/types/goal.hpp
    include/prioritizedgoalsplanner/types/goalstack.hpp
    include/prioritizedgoalsplanner/types/historical.hpp
    include/prioritizedgoalsplanner/types/lookforanactionoutputinfos.hpp
    include/prioritizedgoalsplanner/types/factstovalue.hpp
    include/prioritizedgoalsplanner/types/ontology.hpp
    include/prioritizedgoalsplanner/types/parameter.hpp
    include/prioritizedgoalsplanner/types/predicate.hpp
    include/prioritizedgoalsplanner/types/problem.hpp
    include/prioritizedgoalsplanner/types/problemmodification.hpp
    include/prioritizedgoalsplanner/types/setofentities.hpp
    include/prioritizedgoalsplanner/types/setofevents.hpp
    include/prioritizedgoalsplanner/types/setoffacts.hpp
    include/prioritizedgoalsplanner/types/setofpredicates.hpp
    include/prioritizedgoalsplanner/types/setofderivedpredicates.hpp
    include/prioritizedgoalsplanner/types/setoftypes.hpp
    include/prioritizedgoalsplanner/types/setofconstfacts.hpp
    include/prioritizedgoalsplanner/types/type.hpp
    include/prioritizedgoalsplanner/types/worldstate.hpp
    include/prioritizedgoalsplanner/types/worldstatemodification.hpp
    include/prioritizedgoalsplanner/util/exportsymbols/macro.hpp
    include/prioritizedgoalsplanner/util/serializer/deserializefrompddl.hpp
    include/prioritizedgoalsplanner/util/serializer/serializeinpddl.hpp
    include/prioritizedgoalsplanner/util/trackers/goalsremovedtracker.hpp
    include/prioritizedgoalsplanner/util/api.hpp
    include/prioritizedgoalsplanner/util/alias.hpp
    include/prioritizedgoalsplanner/util/arithmeticevaluator.hpp
    include/prioritizedgoalsplanner/util/print.hpp
    include/prioritizedgoalsplanner/util/observableunsafe.hpp
    include/prioritizedgoalsplanner/util/replacevariables.hpp
    include/prioritizedgoalsplanner/util/util.hpp
    include/prioritizedgoalsplanner/prioritizedgoalsplanner.hpp
)


set(CONTEXTUAL_PLANNER_SRCS
    src/algo/converttoparallelplan.hpp
    src/algo/converttoparallelplan.cpp
    src/algo/notifyactiondone.hpp
    src/algo/notifyactiondone.cpp
    src/types/action.cpp
    src/types/actioninvocation.cpp
    src/types/actioninvocationwithgoal.cpp
    src/types/axiom.cpp
    src/types/condition.cpp
    src/types/derivedpredicate.cpp
    src/types/domain.cpp
    src/types/entity.cpp
    src/types/expressionParsed.hpp
    src/types/expressionParsed.cpp
    src/types/event.cpp
    src/types/fact.cpp
    src/types/factoptional.cpp
    src/types/factsalreadychecked.hpp
    src/types/goal.cpp
    src/types/goalstack.cpp
    src/types/historical.cpp
    src/types/lookforanactionoutputinfos.cpp
    src/types/factstovalue.cpp
    src/types/parameter.cpp
    src/types/predicate.cpp
    src/types/problemmodification.cpp
    src/types/setofentities.cpp
    src/types/setofevents.cpp
    src/types/setoffacts.cpp
    src/types/setofderivedpredicates.cpp
    src/types/setofpredicates.cpp
    src/types/setoftypes.cpp
    src/types/treeofalreadydonepaths.hpp
    src/types/treeofalreadydonepaths.cpp
    src/types/type.cpp
    src/types/worldstate.cpp
    src/types/worldstatecache.hpp
    src/types/worldstatecache.cpp
    src/types/worldstatemodification.cpp
    src/types/worldstatemodificationprivate.hpp
    src/types/worldstatemodificationprivate.cpp
    src/util/serializer/deserializefrompddl.cpp
    src/util/serializer/serializeinpddl.cpp
    src/util/trackers/goalsremovedtracker.cpp
    src/util/arithmeticevaluator.cpp
    src/util/print.cpp
    src/util/replacevariables.cpp
    src/util/util.cpp
    src/util/uuid.hpp
    src/util/uuid.cpp
    src/prioritizedgoalsplanner.cpp
)


include_directories(include)


add_library(prioritizedgoalsplannerlib SHARED
  ${CONTEXTUAL_PLANNER_HPPS}
  ${CONTEXTUAL_PLANNER_SRCS}
  include/prioritizedgoalsplanner/types/setofderivedpredicates.hpp
  src/types/setofderivedpredicates.cpp
  src/types/axiom.cpp
  include/prioritizedgoalsplanner/types/setofconstfacts.hpp
  include/prioritizedgoalsplanner/types/condtionstovalue.hpp
  src/types/condtionstovalue.cpp
  include/prioritizedgoalsplanner/util/continueorbreak.hpp
)
target_compile_features(prioritizedgoalsplannerlib PRIVATE cxx_std_14)

install(TARGETS prioritizedgoalsplannerlib DESTINATION lib)
install(DIRECTORY include/ DESTINATION include)


add_subdirectory(bin)


if (BUILD_CONTEXTUAL_PLANNER_TESTS)
   add_subdirectory(tests)
endif()
