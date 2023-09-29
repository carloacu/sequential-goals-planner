#ifndef INCLUDE_CONTEXTUALPLANNER_FACTCONDITION_HPP
#define INCLUDE_CONTEXTUALPLANNER_FACTCONDITION_HPP

#include <functional>
#include <memory>
#include "../util/api.hpp"
#include "factoptional.hpp"


namespace cp
{
struct Problem;
struct FactConditionNode;
struct FactConditionFact;
struct FactConditionNumber;

enum class FactConditionType
{
  NODE,
  FACT,
  NUMBER
};

struct CONTEXTUALPLANNER_API FactCondition
{
  FactCondition(FactConditionType pType);

  virtual bool hasFact(const cp::Fact& pFact) const = 0;
  virtual bool containsFactOpt(const FactOptional& pFactOptional,
                               const std::map<std::string, std::set<std::string>>& pFactParameters,
                               const std::vector<std::string>& pThisFactParameters) const = 0;
  virtual void replaceFact(const cp::Fact& pOldFact,
                           const Fact& pNewFact) = 0;
  virtual void forAll(const std::function<void (const FactOptional&)>& pFactCallback) const = 0;
  virtual bool untilFalse(const std::function<bool (const FactOptional&)>& pFactCallback,
                          const Problem& pProblem,
                          const std::map<std::string, std::set<std::string>>& pParameters) const = 0;
  virtual bool canBeTrue() const = 0;
  virtual bool isTrue(const Problem& pProblem,
                      const std::set<Fact>& pPunctualFacts = {},
                      const std::set<Fact>& pRemovedFacts = {},
                      std::map<std::string, std::set<std::string>>* pParametersPtr = nullptr,
                      bool* pCanBecomeTruePtr = nullptr) const = 0;
  virtual bool canBecomeTrue(const Problem& pProblem) const = 0;
  virtual bool operator==(const FactCondition& pOther) const = 0;
  virtual bool operator!=(const FactCondition& pOther) const { return !operator==(pOther); }

  virtual std::string getValue(const Problem& pProblem) const = 0;

  virtual std::unique_ptr<FactCondition> clone(const std::map<std::string, std::string>* pParametersPtr = nullptr) const = 0;

  virtual const FactConditionNode* fcNodePtr() const = 0;
  virtual FactConditionNode* fcNodePtr() = 0;

  virtual const FactConditionFact* fcFactPtr() const = 0;
  virtual FactConditionFact* fcFactPtr() = 0;

  virtual const FactConditionNumber* fcNbPtr() const = 0;
  virtual FactConditionNumber* fcNbPtr() = 0;

  static std::unique_ptr<FactCondition> fromStr(const std::string& pStr);

  virtual std::string toStr() const = 0;

  FactConditionType type;
};


enum class FactConditionNodeType
{
  EQUALITY,
  AND,
  PLUS,
  MINUS
};


struct CONTEXTUALPLANNER_API FactConditionNode : public FactCondition
{
  FactConditionNode(FactConditionNodeType pNodeType,
                    std::unique_ptr<FactCondition> pLeftOperand,
                    std::unique_ptr<FactCondition> pRightOperand);

  bool hasFact(const Fact& pFact) const override;
  bool containsFactOpt(const FactOptional& pFactOptional,
                       const std::map<std::string, std::set<std::string>>& pFactParameters,
                       const std::vector<std::string>& pThisFactParameters) const override;
  void replaceFact(const Fact& pOldFact,
                   const Fact& pNewFact) override;
  void forAll(const std::function<void (const FactOptional&)>& pFactCallback) const override;
  bool untilFalse(const std::function<bool (const FactOptional&)>& pFactCallback,
                  const Problem& pProblem,
                  const std::map<std::string, std::set<std::string>>& pParameters) const override;
  bool canBeTrue() const override;
  bool isTrue(const Problem& pProblem,
              const std::set<Fact>& pPunctualFacts,
              const std::set<Fact>& pRemovedFacts,
              std::map<std::string, std::set<std::string>>* pParametersPtr,
              bool* pCanBecomeTruePtr) const override;
  bool canBecomeTrue(const Problem& pProblem) const override;
  bool operator==(const FactCondition& pOther) const override;

  std::string getValue(const Problem& pProblem) const override;

  std::unique_ptr<FactCondition> clone(const std::map<std::string, std::string>* pParametersPtr) const override;

  const FactConditionNode* fcNodePtr() const override { return this; }
  FactConditionNode* fcNodePtr() override { return this; }
  const FactConditionFact* fcFactPtr() const override { return nullptr; }
  FactConditionFact* fcFactPtr() override { return nullptr; }
  const FactConditionNumber* fcNbPtr() const override { return nullptr; }
  FactConditionNumber* fcNbPtr() override { return nullptr; }

  std::string toStr() const override;

  FactConditionNodeType nodeType;
  std::unique_ptr<FactCondition> leftOperand;
  std::unique_ptr<FactCondition> rightOperand;
};

struct CONTEXTUALPLANNER_API FactConditionFact : public FactCondition
{
  FactConditionFact(const FactOptional& pFactOptional);

  bool hasFact(const cp::Fact& pFact) const override;
  bool containsFactOpt(const FactOptional& pFactOptional,
                       const std::map<std::string, std::set<std::string>>& pFactParameters,
                       const std::vector<std::string>& pThisFactParameters) const override;
  void replaceFact(const cp::Fact& pOldFact,
                   const Fact& pNewFact) override;
  void forAll(const std::function<void (const FactOptional&)>& pFactCallback) const override { pFactCallback(factOptional); }
  bool untilFalse(const std::function<bool (const FactOptional&)>& pFactCallback,
                  const Problem&,
                  const std::map<std::string, std::set<std::string>>&) const override { return pFactCallback(factOptional); }
  bool canBeTrue() const override { return factOptional.isFactNegated || !factOptional.fact.isUnreachable(); }
  bool isTrue(const Problem& pProblem,
              const std::set<Fact>& pPunctualFacts,
              const std::set<Fact>& pRemovedFacts,
              std::map<std::string, std::set<std::string>>* pParametersPtr,
              bool* pCanBecomeTruePtr) const override;
  bool canBecomeTrue(const Problem& pProblem) const override;
  bool operator==(const FactCondition& pOther) const override;

  std::string getValue(const Problem& pProblem) const override;

  const FactConditionNode* fcNodePtr() const override { return nullptr; }
  FactConditionNode* fcNodePtr() override { return nullptr; }
  const FactConditionFact* fcFactPtr() const override { return this; }
  FactConditionFact* fcFactPtr() override { return this; }
  const FactConditionNumber* fcNbPtr() const override { return nullptr; }
  FactConditionNumber* fcNbPtr() override { return nullptr; }

  std::unique_ptr<FactCondition> clone(const std::map<std::string, std::string>* pParametersPtr) const override;

  std::string toStr() const override { return factOptional.toStr(); }

  FactOptional factOptional;
};


struct CONTEXTUALPLANNER_API FactConditionNumber : public FactCondition
{
  FactConditionNumber(int pNb);

  bool hasFact(const cp::Fact&) const override  { return false; }
  bool containsFactOpt(const FactOptional&,
                       const std::map<std::string, std::set<std::string>>&,
                       const std::vector<std::string>&) const override { return false; }
  void replaceFact(const cp::Fact& pOldFact,
                   const Fact& pNewFact) override {}
  void forAll(const std::function<void (const FactOptional&)>&) const override {}
  bool untilFalse(const std::function<bool (const FactOptional&)>&,
                  const Problem&,
                  const std::map<std::string, std::set<std::string>>&) const override { return true; }
  bool canBeTrue() const override { return true; }
  bool isTrue(const Problem& pProblem,
              const std::set<Fact>& pPunctualFacts,
              const std::set<Fact>& pRemovedFacts,
              std::map<std::string, std::set<std::string>>* pParametersPtr,
              bool* pCanBecomeTruePtr) const override { return true; }
  bool canBecomeTrue(const Problem& pProblem) const override  { return true; }
  bool operator==(const FactCondition& pOther) const override;

  std::string getValue(const Problem& pProblem) const override;

  std::unique_ptr<FactCondition> clone(const std::map<std::string, std::string>* pParametersPtr) const override;

  std::string toStr() const override;

  const FactConditionNode* fcNodePtr() const override { return nullptr; }
  FactConditionNode* fcNodePtr() override { return nullptr; }
  const FactConditionFact* fcFactPtr() const override { return nullptr; }
  FactConditionFact* fcFactPtr() override { return nullptr; }
  const FactConditionNumber* fcNbPtr() const override { return this; }
  FactConditionNumber* fcNbPtr() override { return this; }

  int nb;
};



} // !cp


#endif // INCLUDE_CONTEXTUALPLANNER_FACTCONDITION_HPP
