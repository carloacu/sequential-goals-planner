#ifndef INCLUDE_CONTEXTUALPLANNER_GOAL_HPP
#define INCLUDE_CONTEXTUALPLANNER_GOAL_HPP

#include <memory>
#include <string>
#include <chrono>
#include "fact.hpp"
#include "api.hpp"



namespace cp
{

struct CONTEXTUALPLANNER_API Goal
{
  Goal(const std::string& pStr,
       bool pIsStackable = true,
       int pMaxTimeToKeepInactive = -1,
       const std::string& pGoalGroupId = "");
  Goal(const Goal& pOther);

  void operator=(const Goal& pOther);
  bool operator==(const Goal& pOther) const;
  bool operator!=(const Goal& pOther) const { return !operator==(pOther); }

  void setInactiveSinceIfNotAlreadySet(const std::unique_ptr<std::chrono::steady_clock::time_point>& pInactiveSince);
  bool wasInactiveForTooLong(const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow);
  void notifyActivity();

  std::string toStr() const;
  bool isStackable() const { return _isStackable; }
  bool isPersistent() const { return _isPersistentIfSkipped; }
  const Fact* conditionFactPtr() const { return _conditionFactPtr ? &*_conditionFactPtr : nullptr; }
  const Fact& fact() const { return _fact; }
  const std::string& getGoalGroupId() const { return _goalGroupId; }

  static const std::string persistFunctionName;
  static const std::string implyFunctionName;

private:
  Fact _fact;
  bool _isStackable;
  int _maxTimeToKeepInactive;
  std::unique_ptr<std::chrono::steady_clock::time_point> _inactiveSince;
  bool _isPersistentIfSkipped;
  std::unique_ptr<Fact> _conditionFactPtr;
  std::string _goalGroupId;
};

} // !cp


#endif // INCLUDE_CONTEXTUALPLANNER_GOAL_HPP
