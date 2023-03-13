#include <contextualplanner/types/goal.hpp>
#include <assert.h>


namespace cp
{
const std::string Goal::persistFunctionName = "persist";
const std::string Goal::implyFunctionName = "imply";
const std::string Goal::oneStepTowardsFunctionName = "oneStepTowards";


Goal::Goal(const std::string& pStr,
           int pMaxTimeToKeepInactive,
           const std::string& pGoalGroupId)
  : _factOptional(pStr),
    _maxTimeToKeepInactive(pMaxTimeToKeepInactive),
    _inactiveSince(),
    _isPersistentIfSkipped(false),
    _oneStepTowards(false),
    _conditionFactPtr(),
    _goalGroupId(pGoalGroupId)
{
  if (_factOptional.fact.name == persistFunctionName &&
      _factOptional.fact.parameters.size() == 1 &&
      _factOptional.fact.value.empty())
  {
    _isPersistentIfSkipped = true;
    // Temporary variable factParameters is needed for Android compilation (to not have the same assignee and value)
    auto factFirstParameters = std::move(_factOptional.fact.parameters.front());
    _factOptional = std::move(factFirstParameters);
  }

  if (_factOptional.fact.name == oneStepTowardsFunctionName &&
      _factOptional.fact.parameters.size() == 1 &&
      _factOptional.fact.value.empty())
  {
    _oneStepTowards = true;
    // Temporary variable factParameters is needed for Android compilation (to not have the same assignee and value)
    auto factFirstParameters = std::move(_factOptional.fact.parameters.front());
    _factOptional = std::move(factFirstParameters);
  }

  if (_factOptional.fact.name == implyFunctionName &&
      _factOptional.fact.parameters.size() == 2 &&
      _factOptional.fact.value.empty())
  {
    _conditionFactPtr = std::make_unique<FactOptional>(_factOptional.fact.parameters[0]);
    // Temporary variable factParameters is needed for Android compilation (to not have the same assignee and value)
    auto factSecondParameters = std::move(_factOptional.fact.parameters[1]);
    _factOptional = std::move(factSecondParameters);
  }

  assert(!_factOptional.fact.name.empty());
}

Goal::Goal(const Goal& pOther)
  : _factOptional(pOther._factOptional),
    _maxTimeToKeepInactive(pOther._maxTimeToKeepInactive),
    _inactiveSince(pOther._inactiveSince ? std::make_unique<std::chrono::steady_clock::time_point>(*pOther._inactiveSince) : std::unique_ptr<std::chrono::steady_clock::time_point>()),
    _isPersistentIfSkipped(pOther._isPersistentIfSkipped),
    _oneStepTowards(pOther._oneStepTowards),
    _conditionFactPtr(pOther._conditionFactPtr ? std::make_unique<FactOptional>(*pOther._conditionFactPtr) : std::unique_ptr<FactOptional>()),
    _goalGroupId(pOther._goalGroupId)
{
}

void Goal::operator=(const Goal& pOther)
{
  _factOptional = pOther._factOptional;
  _maxTimeToKeepInactive = pOther._maxTimeToKeepInactive;
  if (pOther._inactiveSince)
    _inactiveSince = std::make_unique<std::chrono::steady_clock::time_point>(*pOther._inactiveSince);
  else
    _inactiveSince.reset();
  _isPersistentIfSkipped = pOther._isPersistentIfSkipped;
  _oneStepTowards = pOther._oneStepTowards;
  _conditionFactPtr = pOther._conditionFactPtr ? std::make_unique<FactOptional>(*pOther._conditionFactPtr) : std::unique_ptr<FactOptional>();
  _goalGroupId = pOther._goalGroupId;
}

bool Goal::operator==(const Goal& pOther) const
{
  return _factOptional == pOther._factOptional &&
      _maxTimeToKeepInactive == pOther._maxTimeToKeepInactive &&
      _isPersistentIfSkipped == pOther._isPersistentIfSkipped &&
      _oneStepTowards == pOther._oneStepTowards &&
      _goalGroupId == pOther._goalGroupId;
}


void Goal::setInactiveSinceIfNotAlreadySet(const std::unique_ptr<std::chrono::steady_clock::time_point>& pInactiveSince)
{
  if (!_inactiveSince && pInactiveSince)
    _inactiveSince = std::make_unique<std::chrono::steady_clock::time_point>(*pInactiveSince);
}


bool Goal::isInactiveForTooLong(const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow)
{
  if (_maxTimeToKeepInactive < 0)
    return false;
  if (_maxTimeToKeepInactive == 0)
    return true;
  return _inactiveSince && pNow &&
      std::chrono::duration_cast<std::chrono::seconds>(*pNow - *_inactiveSince).count() > _maxTimeToKeepInactive;
}

void Goal::notifyActivity()
{
  _inactiveSince.reset();
}

std::string Goal::toStr() const
{
  auto res = _factOptional.toStr();
  if (_conditionFactPtr)
    res = implyFunctionName + "(" + _conditionFactPtr->toStr() + ", " + res + ")";
  if (_oneStepTowards)
    res = oneStepTowardsFunctionName + "(" + res + ")";
  if (_isPersistentIfSkipped)
    res = persistFunctionName + "(" + res + ")";
  return res;
}


} // !cp
