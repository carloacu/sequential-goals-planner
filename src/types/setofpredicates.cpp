#include <contextualplanner/types/setofpredicates.hpp>
#include <vector>
#include <contextualplanner/types/setoftypes.hpp>
#include <contextualplanner/util/util.hpp>


namespace cp
{

SetOfPredicates::SetOfPredicates()
    : _predicates(),
      _nameToPredicate()
{
}


SetOfPredicates SetOfPredicates::fromStr(const std::string& pStr,
                                         const SetOfTypes& pSetOfTypes)
{
  SetOfPredicates res;
  std::vector<std::string> lineSplitted;
  cp::split(lineSplitted, pStr, "\n");
  for (auto& currLine : lineSplitted)
    res.addPredicate(Predicate(currLine, pSetOfTypes));
  return res;
}


void SetOfPredicates::addPredicate(const Predicate& pPredicate)
{
  _predicates.push_back(pPredicate);
  _nameToPredicate[pPredicate.name] = &_predicates.back();
}

const Predicate* SetOfPredicates::nameToPredicatePtr(const std::string& pName) const
{
  auto it = _nameToPredicate.find(pName);
  if (it != _nameToPredicate.end())
    return it->second;
  return nullptr;
}

Predicate SetOfPredicates::nameToPredicate(const std::string& pName) const
{
  auto it = _nameToPredicate.find(pName);
  if (it != _nameToPredicate.end())
    return *it->second;

  if (empty()) // For retrocompatibility
    return Predicate(pName, {});
  throw std::runtime_error("\"" + pName + "\" is not a predicate name");
}


std::string SetOfPredicates::toStr() const
{
  std::string res;
  bool firstIteration = true;
  for (auto& currPredicate : _predicates)
  {
    if (firstIteration)
      firstIteration = false;
    else
      res += "\n";
    res += currPredicate.toStr();
  }
  return res;
}

bool SetOfPredicates::empty() const
{
  return _predicates.empty() && _nameToPredicate.empty();
}

} // !cp
