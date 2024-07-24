#include <contextualplanner/types/factoptional.hpp>


namespace cp
{

FactOptional::FactOptional(const Fact& pFact,
                           bool pIsFactNegated)
  : isFactNegated(pIsFactNegated),
    fact(pFact)
{
}

FactOptional::FactOptional(const FactOptional& pOther,
                           const std::map<std::string, Entity>* pParametersPtr)
  : isFactNegated(pOther.isFactNegated),
    fact(pOther.fact)
{
  if (pParametersPtr != nullptr)
    fact.replaceArguments(*pParametersPtr);
}

FactOptional::FactOptional(const std::string& pStr,
                           const Ontology& pOntology,
                           const SetOfEntities& pEntities,
                           const char* pSeparatorPtr,
                           std::size_t pBeginPos,
                           std::size_t* pResPos)
  : isFactNegated(false),
    fact(pStr, pOntology, pEntities, pSeparatorPtr, &isFactNegated, pBeginPos, pResPos)
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

std::string FactOptional::toStr(const std::function<std::string (const Fact&)>* pFactWriterPtr) const
{
  auto polarityStr = isFactNegated ? "!" : "";
  if (pFactWriterPtr)
    return polarityStr + (*pFactWriterPtr)(fact);
  return polarityStr + fact.toStr();
}


} // !cp
