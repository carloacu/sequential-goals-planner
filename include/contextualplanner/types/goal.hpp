#ifndef INCLUDE_CONTEXTUALPLANNER_GOAL_HPP
#define INCLUDE_CONTEXTUALPLANNER_GOAL_HPP

#include <memory>
#include <string>
#include <chrono>
#include "factoptional.hpp"
#include "../util/api.hpp"


namespace cp
{

// A characteristic that the world should have. It is the motivation of the bot for doing actions to respect this characteristic of the world.
struct CONTEXTUALPLANNER_API Goal
{
  /**
   * @brief Construct a goal.
   * @param pStr Serialized string corresponding to the fact contained in this goal.
   * @param pMaxTimeToKeepInactive Max time to keep this goal not in the top of the goals stack.
   * @param pGoalGroupId Identifier of a group of goals.
   */
  Goal(const std::string& pStr,
       int pMaxTimeToKeepInactive = -1,
       const std::string& pGoalGroupId = "");

  /// Construct a goal from another goal.
  Goal(const Goal& pOther);

  /// Set content from another goal.
  void operator=(const Goal& pOther);

  /// Check equality with another goal.
  bool operator==(const Goal& pOther) const;
  /// Check not equality with another goal.
  bool operator!=(const Goal& pOther) const { return !operator==(pOther); }

  /// Set start of inactive time, if not already set. (inactive = not in top of the goals stack)
  void setInactiveSinceIfNotAlreadySet(const std::unique_ptr<std::chrono::steady_clock::time_point>& pInactiveSince);

  // TODO: Optimize by having to lazy current time value!
  /**
   * @brief Check if the goal is inactive for too long.
   * @param pNow Current time.
   * @return True if the fact is inactive for too long, false otherwise.
   */
  bool isInactiveForTooLong(const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow);

  /// Notify that the gaol is active. (so in top of the goals stack)
  void notifyActivity();

  /// Get the time when this goal became incative. It is a null pointer if the goal is active.
  const std::unique_ptr<std::chrono::steady_clock::time_point>& getInactiveSince() const { return _inactiveSince; }

  /// Convert this goal to a string.
  std::string toStr() const;

  /// Know if the goal will be kept in the goals stack, when we succeded or failed to satisfy it.
  bool isPersistent() const { return _isPersistentIfSkipped; }

  /// Get the condition associated. It is a fact that should be present in the world to enable this goal.
  const FactOptional* conditionFactOptionalPtr() const { return _conditionFactPtr ? &*_conditionFactPtr : nullptr; }

  /// Get a const reference of the optional fact contained in this goal.
  const FactOptional& factOptional() const { return _factOptional; }

  /// Get a reference of the optional fact contained in this goal.
  FactOptional& factOptional() { return _factOptional; }

  /// Get the group identifier of this goal. It can be empty if the goal does not belong to a group.
  const std::string& getGoalGroupId() const { return _goalGroupId; }

  /**
   * Get the maximum time that we allow for this goal to be inactive.<b/>
   * A negative value means that the time is infinite.
   */
  int getMaxTimeToKeepInactive() const { return _maxTimeToKeepInactive; }

  /// Persist function name.
  static const std::string persistFunctionName;
  /// Imply function name.
  static const std::string implyFunctionName;

private:
  /// Fact that can be negated that is contained in this goal.
  FactOptional _factOptional;
  /**
   * The maximum time that we allow for this goal to be inactive.<b/>
   * A negative value means that the time is infinite.
   */
  int _maxTimeToKeepInactive;
  /// Time when this goal became inactive. It is a null pointer if the goal is active.
  std::unique_ptr<std::chrono::steady_clock::time_point> _inactiveSince;
  /// Know if the goal will be kept in the goals stack, when we succeded or failed to satisfy it.
  bool _isPersistentIfSkipped;
  /// Condition associated, it is a fact that should be present in the world to enable this goal.
  std::unique_ptr<FactOptional> _conditionFactPtr;
  /// Group identifier of this goal. It can be empty if the goal does not belong to a group.
  std::string _goalGroupId;
};

} // !cp


#endif // INCLUDE_CONTEXTUALPLANNER_GOAL_HPP
