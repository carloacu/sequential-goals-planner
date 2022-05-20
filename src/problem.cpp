#include <contextualplanner/problem.hpp>
#include <sstream>

namespace cp
{
const int Problem::defaultPriority = 10;


namespace
{

template <typename T>
T _lexical_cast(const std::string& pStr)
{
  bool firstChar = true;
  for (const auto& currChar : pStr)
  {
    if ((currChar < '0' || currChar > '9') &&
        !(firstChar && currChar == '-'))
      throw std::runtime_error("bad lexical cast: source type value could not be interpreted as target");
    firstChar = false;
  }
  return atoi(pStr.c_str());
}


void _incrementStr(std::string& pStr)
{
  if (pStr.empty())
  {
    pStr = "1";
  }
  else
  {
    try
    {
      std::stringstream ss;
      ss << _lexical_cast<int>(pStr) + 1;
      pStr = ss.str();
    }
    catch (...) {}
  }
}
}


Problem::Problem(const Problem& pOther)
 : historical(pOther.historical),
   onVariablesToValueChanged(),
   onFactsChanged(),
   onGoalsChanged(),
   _goals(pOther._goals),
   _variablesToValue(pOther._variablesToValue),
   _facts(pOther._facts),
   _factNamesToNbOfFactOccurences(pOther._factNamesToNbOfFactOccurences),
   _reachableFacts(pOther._reachableFacts),
   _reachableFactsWithAnyValues(pOther._reachableFactsWithAnyValues),
   _removableFacts(pOther._removableFacts),
   _needToAddReachableFacts(pOther._needToAddReachableFacts)
{
}


std::string Problem::getCurrentGoal() const
{
  for (auto itGoalGroup = _goals.rbegin(); itGoalGroup != _goals.rend(); ++itGoalGroup)
    if (!itGoalGroup->second.empty())
      return itGoalGroup->second.front().toStr();
  return "";
}

void Problem::addVariablesToValue(const std::map<std::string, std::string>& pVariablesToValue)
{
  if (!pVariablesToValue.empty())
    for (const auto& currFactToVal : pVariablesToValue)
      _variablesToValue[currFactToVal.first] = currFactToVal.second;
  onVariablesToValueChanged(_variablesToValue);
}

bool Problem::addFact(const Fact& pFact)
{
  return addFacts(std::vector<Fact>{pFact});
}

template<typename FACTS>
bool Problem::addFacts(const FACTS& pFacts)
{
  bool res = _addFactsWithoutFactNotification(pFacts);
  if (res)
    onFactsChanged(_facts);
  return res;
}

bool Problem::removeFact(const Fact& pFact)
{
  return removeFacts(std::vector<Fact>{pFact});
}

template<typename FACTS>
bool Problem::removeFacts(const FACTS& pFacts)
{
  bool res = _removeFactsWithoutFactNotification(pFacts);
  if (res)
    onFactsChanged(_facts);
  return res;
}

template bool Problem::addFacts<std::set<Fact>>(const std::set<Fact>&);
template bool Problem::addFacts<std::vector<Fact>>(const std::vector<Fact>&);


void Problem::_addFactNameRef(const std::string& pFactName)
{
  auto itFactName = _factNamesToNbOfFactOccurences.find(pFactName);
  if (itFactName == _factNamesToNbOfFactOccurences.end())
    _factNamesToNbOfFactOccurences[pFactName] = 1;
  else
    ++itFactName->second;
}

template<typename FACTS>
bool Problem::_addFactsWithoutFactNotification(const FACTS& pFacts)
{
  bool res = false;
  for (const auto& currFact : pFacts)
  {
    if (_facts.count(currFact) > 0)
      continue;
    res = true;
    _facts.insert(currFact);
    _addFactNameRef(currFact.name);

    auto itReachable = _reachableFacts.find(currFact);
    if (itReachable != _reachableFacts.end())
      _reachableFacts.erase(itReachable);
    else
      _clearRechableAndRemovableFacts();
  }
  return res;
}


template<typename FACTS>
bool Problem::_removeFactsWithoutFactNotification(const FACTS& pFacts)
{
  bool res = false;
  for (const auto& currFact : pFacts)
  {
    auto it = _facts.find(currFact);
    if (it == _facts.end())
      continue;
    res = true;
    _facts.erase(it);
    {
      auto itFactName = _factNamesToNbOfFactOccurences.find(currFact.name);
      if (itFactName == _factNamesToNbOfFactOccurences.end())
        assert(false);
      else if (itFactName->second == 1)
        _factNamesToNbOfFactOccurences.erase(itFactName);
      else
        --itFactName->second;
    }
    _clearRechableAndRemovableFacts();
  }
  return res;
}


template bool Problem::_addFactsWithoutFactNotification<std::set<Fact>>(const std::set<Fact>&);
template bool Problem::_addFactsWithoutFactNotification<std::vector<Fact>>(const std::vector<Fact>&);


void Problem::_clearRechableAndRemovableFacts()
{
  _needToAddReachableFacts = true;
  _reachableFacts.clear();
  _reachableFactsWithAnyValues.clear();
  _removableFacts.clear();
}

bool Problem::modifyFacts(const SetOfFacts& pSetOfFacts)
{
  bool factsChanged = _addFactsWithoutFactNotification(pSetOfFacts.facts);
  factsChanged = _removeFactsWithoutFactNotification(pSetOfFacts.notFacts) || factsChanged;
  bool variablesToValueChanged = false;
  for (auto& currExp : pSetOfFacts.exps)
  {
    if (currExp.elts.size() >= 2)
    {
      auto it = currExp.elts.begin();
      if (it->type == ExpressionElementType::OPERATOR)
      {
        auto op = it->value;
        ++it;
        if (op == "++" &&
            it->type == ExpressionElementType::FACT)
        {
          _incrementStr(_variablesToValue[it->value]);
          variablesToValueChanged = true;
        }
      }
      else if (it->type == ExpressionElementType::FACT)
      {
        auto factToSet = it->value;
        ++it;
        if (it->type == ExpressionElementType::OPERATOR)
        {
          auto op = it->value;
          ++it;
          if (op == "=" &&
              it->type == ExpressionElementType::VALUE)
          {
            _variablesToValue[factToSet] = it->value;
            variablesToValueChanged = true;
          }
        }
      }
    }
  }
  if (factsChanged)
    onFactsChanged(_facts);
  if (variablesToValueChanged)
    onVariablesToValueChanged(_variablesToValue);
  return factsChanged;
}


void Problem::setFacts(const std::set<Fact>& pFacts)
{
  if (_facts != pFacts)
  {
    _facts = pFacts;
    _factNamesToNbOfFactOccurences.clear();
    for (const auto& currFact : pFacts)
      _addFactNameRef(currFact.name);
    _clearRechableAndRemovableFacts();
    onFactsChanged(_facts);
  }
}

void Problem::addReachableFacts(const std::set<Fact>& pFacts)
{
  _reachableFacts.insert(pFacts.begin(), pFacts.end());
}

void Problem::addReachableFactsWithAnyValues(const std::vector<Fact>& pFacts)
{
  _reachableFactsWithAnyValues.insert(pFacts.begin(), pFacts.end());
}

void Problem::addRemovableFacts(const std::set<Fact>& pFacts)
{
  _removableFacts.insert(pFacts.begin(), pFacts.end());
}


void Problem::iterateOnGoalAndRemoveNonPersistent(
    const std::function<bool(const Goal&)>& pManageGoal)
{
  bool hasGoalChanged = false;
  for (auto itGoalsGroup = _goals.end(); itGoalsGroup != _goals.begin(); )
  {
    --itGoalsGroup;
    for (auto itGoal = itGoalsGroup->second.begin(); itGoal != itGoalsGroup->second.end(); )
    {
      if (pManageGoal(*itGoal))
      {
        if (hasGoalChanged)
          onGoalsChanged(_goals);
        return;
      }

      if (itGoal->isPersistent())
      {
        ++itGoal;
      }
      else
      {
        itGoal = itGoalsGroup->second.erase(itGoal);
        hasGoalChanged = true;
      }
    }

    if (itGoalsGroup->second.empty())
      itGoalsGroup = _goals.erase(itGoalsGroup);
  }
  if (hasGoalChanged)
    onGoalsChanged(_goals);
}


void Problem::setGoals(const std::map<int, std::vector<Goal>>& pGoals)
{
  if (_goals != pGoals)
  {
    _goals = pGoals;
    _removeNoStackableGoals(false);
    onGoalsChanged(_goals);
  }
}

void Problem::setGoalsForAPriority(const std::vector<Goal>& pGoals,
                                   int pPriority)
{
  setGoals(std::map<int, std::vector<Goal>>{{pPriority, pGoals}});
}

void Problem::addGoals(const std::map<int, std::vector<Goal>>& pGoals)
{
  if (pGoals.empty())
    return;
  for (auto& currGoals : pGoals)
  {
    auto& existingGoals = _goals[currGoals.first];
    existingGoals.insert(existingGoals.begin(), currGoals.second.begin(), currGoals.second.end());
  }
  _removeNoStackableGoals(false);
  onGoalsChanged(_goals);
}


void Problem::pushFrontGoal(const Goal& pGoal,
                            int pPriority)
{
  auto& existingGoals = _goals[pPriority];
  existingGoals.insert(existingGoals.begin(), pGoal);
  _removeNoStackableGoals(true);
  onGoalsChanged(_goals);
}

void Problem::pushBackGoal(const Goal& pGoal,
                           int pPriority)
{
  auto& existingGoals = _goals[pPriority];
  existingGoals.push_back(pGoal);
  _removeNoStackableGoals(true);
  onGoalsChanged(_goals);
}

void Problem::removeGoals(const std::string& pGoalGroupId)
{
  bool aGoalHasBeenRemoved = false;
  for (auto itGroup = _goals.begin(); itGroup != _goals.end(); )
  {
    for (auto it = itGroup->second.begin(); it != itGroup->second.end(); )
    {
      if (it->getGoalGroupId() == pGoalGroupId)
      {
        it = itGroup->second.erase(it);
        aGoalHasBeenRemoved = true;
      }
      else
      {
        ++it;
      }
    }

    if (itGroup->second.empty())
      itGroup = _goals.erase(itGroup);
    else
      ++itGroup;
  }
  if (aGoalHasBeenRemoved)
  {
    _removeNoStackableGoals(false);
    onGoalsChanged(_goals);
  }
}


ActionId Problem::removeFirstGoalsThatAreAlreadySatisfied()
{
  ActionId res;
  auto isGoalNotAlreadySatisfied = [&](const Goal& pGoal){
    auto* goalConditionFactPtr = pGoal.conditionFactPtr();
    if (goalConditionFactPtr == nullptr ||
        _facts.count(*goalConditionFactPtr) > 0)
    {
      auto& goalFact = pGoal.fact();
      return _facts.count(goalFact) == 0;
    }
    return true;
  };

  iterateOnGoalAndRemoveNonPersistent(isGoalNotAlreadySatisfied);
  return res;
}


void Problem::notifyActionDone(const std::string& pActionId,
    const std::map<std::string, std::string>& pParameters,
    const SetOfFacts& pEffect,
    const std::map<int, std::vector<Goal>>* pGoalsToAdd)
{
    historical.notifyActionDone(pActionId);
    if (pParameters.empty())
    {
      modifyFacts(pEffect);
    }
    else
    {
      cp::SetOfFacts effect;
      effect.notFacts = pEffect.notFacts;
      effect.exps = pEffect.exps;
      for (auto& currFact : pEffect.facts)
      {
        auto fact = currFact;
        fact.fillParameters(pParameters);
        effect.facts.insert(std::move(fact));
      }
      modifyFacts(effect);
    }
    if (pGoalsToAdd != nullptr && !pGoalsToAdd->empty())
      addGoals(*pGoalsToAdd);
}


void Problem::_removeNoStackableGoals(bool pCheckOnlyForSecondGoal)
{
  bool firstGoal = true;
  bool hasGoalChanged = false;
  bool shouldBreak = false;
  for (auto itGoalsGroup = _goals.end(); itGoalsGroup != _goals.begin(); )
  {
    --itGoalsGroup;
    for (auto itGoal = itGoalsGroup->second.begin(); itGoal != itGoalsGroup->second.end(); )
    {
      if (firstGoal)
      {
        firstGoal = false;
        ++itGoal;
        continue;
      }

      if (itGoal->isStackable())
      {
        ++itGoal;
      }
      else
      {
        itGoal = itGoalsGroup->second.erase(itGoal);
        hasGoalChanged = true;
      }
      if (pCheckOnlyForSecondGoal)
      {
        shouldBreak = true;
        break;
      }
    }

    if (itGoalsGroup->second.empty())
      itGoalsGroup = _goals.erase(itGoalsGroup);
    if (shouldBreak)
      break;
  }
  if (hasGoalChanged)
    onGoalsChanged(_goals);
}

} // !cp
