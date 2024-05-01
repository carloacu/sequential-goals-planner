#include <contextualplanner/types/ontology.hpp>
#include <sstream>
#include <stdexcept>


namespace cp
{

Ontology::Ontology()
    : _types(),
      _nameToType()
{
}


void Ontology::addType(const std::string& pTypeToAdd,
                       const std::string& pParentType)
{
  if (pParentType == "")
  {
    _types.push_back(Type(pTypeToAdd));
    _nameToType[pTypeToAdd] = &_types.back();
    return;
  }

  auto it = _nameToType.find(pParentType);
  if (it == _nameToType.end())
    throw std::runtime_error("Parent type \"" + pParentType + "\" is not found.");

  auto type = Type(pTypeToAdd, it->second);
  it->second->subTypes.push_back(type);
  _nameToType[pTypeToAdd] = &it->second->subTypes.back();
}


std::list<std::string> Ontology::typesToStrs() const
{
  if (_types.empty())
    return {};
  std::list<std::string> res;
  for (auto& currType : _types)
    currType.toStrs(res);
  return res;
}


std::string Ontology::typesToStr() const
{
  std::stringstream ss;
  auto strs = typesToStrs();
  bool firstIteration = true;
  for (auto& currStr : strs)
  {
    if (firstIteration)
      firstIteration = false;
    else
      ss << "\n";
    ss << currStr;
  }
  return ss.str();
}



} // !cp
