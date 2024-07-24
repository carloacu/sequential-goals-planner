#include <contextualplanner/types/ontology.hpp>


namespace cp
{

bool Ontology::empty() const
{
  return types.empty() && predicates.empty() && constants.empty();
}


} // !cp
