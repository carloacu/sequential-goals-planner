#ifndef INCLUDE_CONTEXTUALPLANNER_TYPES_PROBLEM_HPP
#define INCLUDE_CONTEXTUALPLANNER_TYPES_PROBLEM_HPP

#include <contextualplanner/types/goalstack.hpp>
#include <contextualplanner/types/worldstate.hpp>
#include <contextualplanner/types/historical.hpp>
#include <contextualplanner/types/setofentities.hpp>
#include "../util/api.hpp"


namespace cp
{

/**
 * @brief The problem is the part of the planification that describes the current state.<br/>
 * It is composed of 3 parts:<br/>
 *  * Goals of the bot<br/>
 *  * Current state of the world.<br/>
 *  * History of actions done. It is useful because some actions should not be repeated.<br/>
 *  <br/>
 *  The other part of the planification is the domain. The domain describes how the word can be modified.<br/>
 *  Contrary to the problem, the domain does not change a lot usually.
 */
struct CONTEXTUALPLANNER_API Problem
{
  Problem(const SetOfFacts* pFactsPtr = nullptr)
    : entities(),
      goalStack(),
      worldState(pFactsPtr),
      historical()
  {
  }

  SetOfEntities entities;

  /// Objectives that the bot wants to do.
  GoalStack goalStack;

  /// Current state of the world.
  WorldState worldState;

  /// History of actions done.
  Historical historical;
};

} // !cp


#endif // INCLUDE_CONTEXTUALPLANNER_TYPES_PROBLEM_HPP
