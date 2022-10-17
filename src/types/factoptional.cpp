#include <contextualplanner/types/factoptional.hpp>


namespace cp
{

FactOptional::FactOptional(const Fact& pFact,
                           bool pIsFactNegated)
  : isFactNegated(pIsFactNegated),
    fact(pFact)
{
}

FactOptional::FactOptional(const FactOptional& pOther)
  : isFactNegated(pOther.isFactNegated),
    fact(pOther.fact)
{
}

FactOptional::FactOptional(const std::string& pStr,
                           const char* pSeparatorPtr,
                           std::size_t pBeginPos,
                           std::size_t* pResPos)
  : isFactNegated(false),
    fact(pStr, pSeparatorPtr, &isFactNegated, pBeginPos, pResPos)
{
}


void FactOptional::operator=(const FactOptional& pOther)
{
  isFactNegated = pOther.isFactNegated;
  fact = pOther.fact;
}


bool FactOptional::operator==(const FactOptional& pOther) const
{
  return isFactNegated == pOther.isFactNegated &&
      fact == pOther.fact;
}

std::string FactOptional::toStr() const
{
  if (isFactNegated)
    return "!" + fact.toStr();
  return fact.toStr();
}


} // !cp
