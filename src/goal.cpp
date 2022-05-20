#include <contextualplanner/goal.hpp>


namespace cp
{
const std::string Goal::persistFunctionName = "persist";
const std::string Goal::implyFunctionName = "imply";


Goal::Goal(const std::string& pStr,
           bool pIsStackable,
           int pMaxTimeToKeepInactive,
           const std::string& pGoalGroupId)
  :  _fact(Fact::fromStr(pStr)),
    _isStackable(pIsStackable),
    _maxTimeToKeepInactive(pMaxTimeToKeepInactive),
    _inactiveSince(),
    _isPersistentIfSkipped(false),
    _conditionFactPtr(),
    _goalGroupId(pGoalGroupId)
{
  if (_fact.name == persistFunctionName &&
      _fact.parameters.size() == 1 &&
      _fact.value.empty())
  {
    _isPersistentIfSkipped = true;
    _fact = _fact.parameters.front();
  }

  if (_fact.name == implyFunctionName &&
      _fact.parameters.size() == 2 &&
      _fact.value.empty())
  {
    _conditionFactPtr = std::unique_ptr<Fact>(new Fact(_fact.parameters[0]));
    _fact = _fact.parameters[1];
  }
}

Goal::Goal(const Goal& pOther)
  : _fact(pOther._fact),
    _isStackable(pOther._isStackable),
    _maxTimeToKeepInactive(pOther._maxTimeToKeepInactive),
    _inactiveSince(pOther._inactiveSince ? std::make_unique<std::chrono::steady_clock::time_point>(*pOther._inactiveSince) : std::unique_ptr<std::chrono::steady_clock::time_point>()),
    _isPersistentIfSkipped(pOther._isPersistentIfSkipped),
    _conditionFactPtr(pOther._conditionFactPtr ? std::unique_ptr<Fact>(new Fact(*pOther._conditionFactPtr)) : std::unique_ptr<Fact>()),
    _goalGroupId(pOther._goalGroupId)
{
}

void Goal::operator=(const Goal& pOther)
{
  _fact = pOther._fact;
  _isStackable = pOther._isStackable;
  _maxTimeToKeepInactive = pOther._maxTimeToKeepInactive;
  if (pOther._inactiveSince)
    _inactiveSince = std::make_unique<std::chrono::steady_clock::time_point>(*pOther._inactiveSince);
  else
    _inactiveSince.reset();
  _isPersistentIfSkipped = pOther._isPersistentIfSkipped;
  _conditionFactPtr = pOther._conditionFactPtr ? std::unique_ptr<Fact>(new Fact(*pOther._conditionFactPtr)) : std::unique_ptr<Fact>();
  _goalGroupId = pOther._goalGroupId;
}

bool Goal::operator==(const Goal& pOther) const
{
  return _fact == pOther._fact &&
      _isStackable == pOther._isStackable &&
      _maxTimeToKeepInactive == pOther._maxTimeToKeepInactive &&
      _inactiveSince == pOther._inactiveSince &&
      _isPersistentIfSkipped == pOther._isPersistentIfSkipped &&
      _goalGroupId == pOther._goalGroupId;
}


void Goal::setInactiveSinceIfNotAlreadySet(const std::unique_ptr<std::chrono::steady_clock::time_point>& pInactiveSince)
{
  if (!_inactiveSince && pInactiveSince)
    _inactiveSince = std::make_unique<std::chrono::steady_clock::time_point>(*pInactiveSince);
}


bool Goal::wasInactiveForTooLong(const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow)
{
  return _inactiveSince && pNow && _maxTimeToKeepInactive > 0 &&
      std::chrono::duration_cast<std::chrono::seconds>(*pNow - *_inactiveSince).count() > _maxTimeToKeepInactive;
}

void Goal::notifyActivity()
{
  _inactiveSince.reset();
}

std::string Goal::toStr() const
{
  auto res = _fact.toStr();
  if (_conditionFactPtr)
    res = implyFunctionName + "(" + res + ")";
  if (_isPersistentIfSkipped)
    res = persistFunctionName + "(" + res + ")";
  return res;
}


} // !cp
