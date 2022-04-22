#include <contextualplanner/goal.hpp>


namespace cp
{
const std::string Goal::persistFunctionName = "persist";
const std::string Goal::implyFunctionName = "imply";


Goal::Goal(const std::string& pStr,
           const std::string& pGoalGroupId)
  : _isPersistent(false),
    _conditionFactPtr(),
    _fact(Fact::fromStr(pStr)),
    _goalGroupId(pGoalGroupId)
{
  if (_fact.name == persistFunctionName &&
      _fact.parameters.size() == 1 &&
      _fact.value.empty())
  {
    _isPersistent = true;
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
  : _isPersistent(pOther._isPersistent),
    _conditionFactPtr(pOther._conditionFactPtr ? std::unique_ptr<Fact>(new Fact(*pOther._conditionFactPtr)) : std::unique_ptr<Fact>()),
    _fact(pOther._fact),
    _goalGroupId(pOther._goalGroupId)
{
}

void Goal::operator=(const Goal& pOther)
{
  _isPersistent = pOther._isPersistent;
  _conditionFactPtr = pOther._conditionFactPtr ? std::unique_ptr<Fact>(new Fact(*pOther._conditionFactPtr)) : std::unique_ptr<Fact>();
  _fact = pOther._fact;
  _goalGroupId = pOther._goalGroupId;
}

bool Goal::operator==(const Goal& pOther) const
{
  return _isPersistent == pOther._isPersistent &&
      _fact == pOther._fact &&
      _goalGroupId == pOther._goalGroupId;
}


std::string Goal::toStr() const
{
  auto res = _fact.toStr();
  if (_conditionFactPtr)
    res = implyFunctionName + "(" + res + ")";
  if (_isPersistent)
    res = persistFunctionName + "(" + res + ")";
  return res;
}


} // !cp
