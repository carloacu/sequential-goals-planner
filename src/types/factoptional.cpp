#include <contextualplanner/types/factoptional.hpp>


namespace cp
{

FactOptional::FactOptional(const Fact& pFact,
                           bool pIsFactNegated)
  : isFactNegated(pIsFactNegated),
    fact(pFact)
{
}

FactOptional::FactOptional(bool pIsFactNegated,
                           const std::string& pName,
                           const std::vector<std::string>& pArgumentStrs,
                           const std::string& pFluentStr,
                           bool pIsFluentNegated,
                           const Ontology& pOntology,
                           const SetOfEntities& pEntities,
                           const std::vector<Parameter>& pParameters,
                           bool pIsOkIfFluentIsMissing)
  : isFactNegated(pIsFactNegated),
    fact(pName, pArgumentStrs, pFluentStr, pIsFluentNegated, pOntology, pEntities, pParameters, pIsOkIfFluentIsMissing)
{
  _simplify();
}


FactOptional::FactOptional(const FactOptional& pOther,
                           const std::map<Parameter, Entity>* pParametersPtr)
  : isFactNegated(pOther.isFactNegated),
    fact(pOther.fact)
{
  if (pParametersPtr != nullptr)
    fact.replaceArguments(*pParametersPtr);
}

FactOptional::FactOptional(const std::string& pStr,
                           const Ontology& pOntology,
                           const SetOfEntities& pEntities,
                           const std::vector<Parameter>& pParameters,
                           std::size_t pBeginPos,
                           std::size_t* pResPos)
  : isFactNegated(false),
    fact(pStr, false, pOntology, pEntities, pParameters, &isFactNegated, pBeginPos, pResPos)
{
  _simplify();
}

bool FactOptional::operator<(const FactOptional& pOther) const
{
  if (isFactNegated != pOther.isFactNegated)
    return isFactNegated < pOther.isFactNegated;
  return fact < pOther.fact;
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

std::string FactOptional::toStr(const std::function<std::string (const Fact&)>* pFactWriterPtr,
                                bool pPrintAnyFluent) const
{
  auto polarityStr = isFactNegated ? "!" : "";
  if (pFactWriterPtr)
    return polarityStr + (*pFactWriterPtr)(fact);
  return polarityStr + fact.toStr(pPrintAnyFluent);
}

std::string FactOptional::toPddl(bool pInEffectContext,
                                 bool pPrintAnyFluent) const
{
  auto res = fact.toPddl(pInEffectContext, pPrintAnyFluent);
  if (isFactNegated)
  {
    if (fact.fluent() && fact.fluent()->isAnyValue())
      return (pInEffectContext ? "(assign " : "(= ") + res + " undefined)";
    return "(not " + res + ")";
  }
  return res;
}

bool FactOptional::doesFactEffectOfSuccessorGiveAnInterestForSuccessor(const FactOptional& pOptFact) const
{
  if (isFactNegated != pOptFact.isFactNegated)
    return true;
  return fact.doesFactEffectOfSuccessorGiveAnInterestForSuccessor(pOptFact.fact);
}


void FactOptional::_simplify()
{
  if (isFactNegated && fact.isValueNegated())
  {
    isFactNegated = false;
    fact.setValueNegated(false);
  }
}



} // !cp
