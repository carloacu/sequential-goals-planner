#include <contextualplanner/types/goal.hpp>
#include <assert.h>
#include <contextualplanner/util/serializer/deserializefrompddl.hpp>
#include <contextualplanner/util/serializer/serializeinpddl.hpp>

namespace cp
{
const std::string Goal::persistFunctionName = "persist";
const std::string Goal::implyFunctionName = "imply";
const std::string Goal::oneStepTowardsFunctionName = "oneStepTowards";
namespace
{
const std::string _persistPrefix = Goal::persistFunctionName + "(";
const std::size_t _persistPrefixSize = _persistPrefix.size();

const std::string _oneStepTowardsPrefix = Goal::oneStepTowardsFunctionName + "(";
const std::size_t _oneStepTowardsPrefixSize = _oneStepTowardsPrefix.size();

const std::string _implyPrefix = Goal::implyFunctionName + "(";
const std::size_t _implyPrefixSize = _implyPrefix.size();
}


Goal::Goal(std::unique_ptr<Condition> pObjective,
           bool pIsPersistentIfSkipped,
           bool pOneStepTowards,
           std::unique_ptr<FactOptional> pConditionFactPtr,
           int pMaxTimeToKeepInactive,
           const std::string& pGoalGroupId)
  : _objective(std::move(pObjective)),
    _maxTimeToKeepInactive(pMaxTimeToKeepInactive),
    _inactiveSince(),
    _isPersistentIfSkipped(pIsPersistentIfSkipped),
    _oneStepTowards(pOneStepTowards),
    _conditionFactPtr(pConditionFactPtr ? std::move(pConditionFactPtr) : std::unique_ptr<FactOptional>()),
    _goalGroupId(pGoalGroupId)
{
  assert(_objective);
}


Goal::Goal(const Goal& pOther,
           const std::map<Parameter, Entity>* pParametersPtr,
           const std::string* pGoalGroupIdPtr)
  : _objective(pOther._objective->clone(pParametersPtr)),
    _maxTimeToKeepInactive(pOther._maxTimeToKeepInactive),
    _inactiveSince(pOther._inactiveSince ? std::make_unique<std::chrono::steady_clock::time_point>(*pOther._inactiveSince) : std::unique_ptr<std::chrono::steady_clock::time_point>()),
    _isPersistentIfSkipped(pOther._isPersistentIfSkipped),
    _oneStepTowards(pOther._oneStepTowards),
    _conditionFactPtr(pOther._conditionFactPtr ? std::make_unique<FactOptional>(*pOther._conditionFactPtr, pParametersPtr) : std::unique_ptr<FactOptional>()),
    _goalGroupId(pGoalGroupIdPtr != nullptr ? *pGoalGroupIdPtr : pOther._goalGroupId)
{
}


Goal Goal::fromStr(const std::string& pStr,
                   const Ontology& pOntology,
                   const SetOfEntities& pEntities,
                   int pMaxTimeToKeepInactive,
                   const std::string& pGoalGroupId)
{
  auto resPtr = strToGoal(pStr, pOntology, pEntities, pMaxTimeToKeepInactive, pGoalGroupId);
  if (!resPtr)
    throw std::runtime_error("Failed to load the goal: " + pStr);
  return *resPtr;
}


void Goal::operator=(const Goal& pOther)
{
  _objective = pOther._objective->clone();
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
  return *_objective == *pOther._objective &&
      _maxTimeToKeepInactive == pOther._maxTimeToKeepInactive &&
      _isPersistentIfSkipped == pOther._isPersistentIfSkipped &&
      _oneStepTowards == pOther._oneStepTowards &&
      _goalGroupId == pOther._goalGroupId;
}

std::unique_ptr<Goal> Goal::clone() const
{
  return std::make_unique<Goal>(*this);
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
  auto res = _objective->toStr();
  if (_conditionFactPtr)
    res = implyFunctionName + "(" + _conditionFactPtr->toStr() + ", " + res + ")";
  if (_oneStepTowards)
    res = oneStepTowardsFunctionName + "(" + res + ")";
  if (_isPersistentIfSkipped)
    res = persistFunctionName + "(" + res + ")";
  return res;
}

std::string Goal::toPddl(std::size_t pIdentation) const
{
  auto res = conditionToPddl(*_objective, pIdentation);
  if (_conditionFactPtr)
    res = "(" + implyFunctionName + " " + _conditionFactPtr->toPddl(false, false) + " " + res + ")";
  if (_oneStepTowards)
    res += " ; one step towards";
  if (_isPersistentIfSkipped)
    res += " ; is persistent if skipped";
  return res;
}


} // !cp
