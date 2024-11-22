#include "treeofalreadydonepaths.hpp"

namespace ogp
{


TreeOfAlreadyDonePath* TreeOfAlreadyDonePath::getNextActionTreeIfNotAnExistingLeaf(const std::string& pActionId)
{
  auto it = _idOfNextActionsToTree.find(pActionId);
  if (it == _idOfNextActionsToTree.end())
    return &_idOfNextActionsToTree[pActionId];
  if (!it->second.empty())
    return &it->second;
  return nullptr;
}


TreeOfAlreadyDonePath* TreeOfAlreadyDonePath::getNextInflectionTreeIfNotAnExistingLeaf(const std::string& pInflectionId)
{
  auto it = _idOfNextInflectionsToTree.find(pInflectionId);
  if (it == _idOfNextInflectionsToTree.end())
    return &_idOfNextInflectionsToTree[pInflectionId];
  if (!it->second.empty())
    return &it->second;
  return nullptr;
}




} // !ogp
