#ifndef INCLUDE_ORDEREDGOALSPLANNER_TREEOFALREADYDONEPATH_HPP
#define INCLUDE_ORDEREDGOALSPLANNER_TREEOFALREADYDONEPATH_HPP

#include <map>
#include <string>

namespace ogp
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


} // !ogp


#endif // INCLUDE_ORDEREDGOALSPLANNER_TREEOFALREADYDONEPATH_HPP
