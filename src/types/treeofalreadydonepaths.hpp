#ifndef INCLUDE_PRIORITIZEDGOALSPLANNER_TREEOFALREADYDONEPATH_HPP
#define INCLUDE_PRIORITIZEDGOALSPLANNER_TREEOFALREADYDONEPATH_HPP

#include <map>
#include <string>

namespace pgp
{

struct TreeOfAlreadyDonePath
{
  bool empty() const { return _idOfNextActionsToTree.empty() & _idOfNextInflectionsToTree.empty(); }
  TreeOfAlreadyDonePath* getNextActionTreeIfNotAnExistingLeaf(const std::string& pActionId);
  TreeOfAlreadyDonePath* getNextInflectionTreeIfNotAnExistingLeaf(const std::string& pInflectionId);

private:
  std::map<std::string, TreeOfAlreadyDonePath> _idOfNextActionsToTree;
  std::map<std::string, TreeOfAlreadyDonePath> _idOfNextInflectionsToTree;
};


} // !pgp


#endif // INCLUDE_PRIORITIZEDGOALSPLANNER_TREEOFALREADYDONEPATH_HPP
