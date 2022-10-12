project(contextualplanner)
cmake_minimum_required(VERSION 2.8)

include(contextualplanner-config.cmake)

# Making a lib
set(CHATBOTPLANNER_HPPS
    include/contextualplanner/types/enum/expressionelementtype.hpp
    include/contextualplanner/types/action.hpp
    include/contextualplanner/types/domain.hpp
    include/contextualplanner/types/expression.hpp
    include/contextualplanner/types/fact.hpp
    include/contextualplanner/types/goal.hpp
    include/contextualplanner/types/historical.hpp
    include/contextualplanner/types/inference.hpp
    include/contextualplanner/types/problem.hpp
    include/contextualplanner/types/setoffacts.hpp
    include/contextualplanner/types/worldmodification.hpp
    include/contextualplanner/util/exportsymbols/macro.hpp
    include/contextualplanner/util/trackers/goalsremovedtracker.hpp
    include/contextualplanner/util/trackers/factschangedtracker.hpp
    include/contextualplanner/util/api.hpp
    include/contextualplanner/util/alias.hpp
    include/contextualplanner/util/arithmeticevaluator.hpp
    include/contextualplanner/util/print.hpp
    include/contextualplanner/util/observableunsafe.hpp
    include/contextualplanner/util/replacevariables.hpp
    include/contextualplanner/contextualplanner.hpp
)


set(CHATBOTPLANNER_SRCS
    src/types/domain.cpp
    src/types/expression.cpp
    src/types/fact.cpp
    src/types/goal.cpp
    src/types/historical.cpp
    src/types/problem.cpp
    src/types/setoffacts.cpp
    src/util/trackers/goalsremovedtracker.cpp
    src/util/trackers/factschangedtracker.cpp
    src/util/arithmeticevaluator.cpp
    src/util/print.cpp
    src/util/replacevariables.cpp
    src/contextualplanner.cpp
)


include_directories(include)


add_library(contextualplanner SHARED
  ${CHATBOTPLANNER_HPPS}
  ${CHATBOTPLANNER_SRCS}
)
target_compile_features(contextualplanner PRIVATE cxx_std_14)

install(TARGETS contextualplanner DESTINATION lib)
install(DIRECTORY include/ DESTINATION include)


add_subdirectory(tests)

