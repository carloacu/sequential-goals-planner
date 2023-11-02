#ifndef INCLUDE_CONTEXTUALPLANNER_CONDITION_HPP
#define INCLUDE_CONTEXTUALPLANNER_CONDITION_HPP

#include <functional>
#include <memory>
#include "../util/api.hpp"
#include "factoptional.hpp"


namespace cp
{
struct Problem;
struct ConditionNode;
struct ConditionFact;
struct ConditionNumber;

enum class ConditionType
{
  NODE,
  FACT,
  NUMBER
};

struct CONTEXTUALPLANNER_API Condition
{
  Condition(ConditionType pType);

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
  virtual bool operator==(const Condition& pOther) const = 0;
  virtual bool operator!=(const Condition& pOther) const { return !operator==(pOther); }

  virtual std::string getValue(const Problem& pProblem) const = 0;

  virtual std::unique_ptr<Condition> clone(const std::map<std::string, std::string>* pParametersPtr = nullptr) const = 0;

  virtual const ConditionNode* fcNodePtr() const = 0;
  virtual ConditionNode* fcNodePtr() = 0;

  virtual const ConditionFact* fcFactPtr() const = 0;
  virtual ConditionFact* fcFactPtr() = 0;

  virtual const ConditionNumber* fcNbPtr() const = 0;
  virtual ConditionNumber* fcNbPtr() = 0;

  static std::unique_ptr<Condition> fromStr(const std::string& pStr);

  virtual std::string toStr(const std::function<std::string(const Fact&)>* pFactWriterPtr = nullptr) const = 0;

  ConditionType type;
};


enum class ConditionNodeType
{
  EQUALITY,
  AND,
  PLUS,
  MINUS
};


struct CONTEXTUALPLANNER_API ConditionNode : public Condition
{
  ConditionNode(ConditionNodeType pNodeType,
                std::unique_ptr<Condition> pLeftOperand,
                std::unique_ptr<Condition> pRightOperand);

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
  bool operator==(const Condition& pOther) const override;

  std::string getValue(const Problem& pProblem) const override;

  std::unique_ptr<Condition> clone(const std::map<std::string, std::string>* pParametersPtr) const override;

  const ConditionNode* fcNodePtr() const override { return this; }
  ConditionNode* fcNodePtr() override { return this; }
  const ConditionFact* fcFactPtr() const override { return nullptr; }
  ConditionFact* fcFactPtr() override { return nullptr; }
  const ConditionNumber* fcNbPtr() const override { return nullptr; }
  ConditionNumber* fcNbPtr() override { return nullptr; }

  std::string toStr(const std::function<std::string(const Fact&)>* pFactWriterPtr) const override;

  ConditionNodeType nodeType;
  std::unique_ptr<Condition> leftOperand;
  std::unique_ptr<Condition> rightOperand;
};

struct CONTEXTUALPLANNER_API ConditionFact : public Condition
{
  ConditionFact(const FactOptional& pFactOptional);

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
  bool operator==(const Condition& pOther) const override;

  std::string getValue(const Problem& pProblem) const override;

  const ConditionNode* fcNodePtr() const override { return nullptr; }
  ConditionNode* fcNodePtr() override { return nullptr; }
  const ConditionFact* fcFactPtr() const override { return this; }
  ConditionFact* fcFactPtr() override { return this; }
  const ConditionNumber* fcNbPtr() const override { return nullptr; }
  ConditionNumber* fcNbPtr() override { return nullptr; }

  std::unique_ptr<Condition> clone(const std::map<std::string, std::string>* pParametersPtr) const override;

  std::string toStr(const std::function<std::string(const Fact&)>* pFactWriterPtr) const override { return factOptional.toStr(pFactWriterPtr); }

  FactOptional factOptional;
};


struct CONTEXTUALPLANNER_API ConditionNumber : public Condition
{
  ConditionNumber(int pNb);

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
  bool operator==(const Condition& pOther) const override;

  std::string getValue(const Problem& pProblem) const override;

  std::unique_ptr<Condition> clone(const std::map<std::string, std::string>* pParametersPtr) const override;

  std::string toStr(const std::function<std::string(const Fact&)>* pFactWriterPtr) const override;

  const ConditionNode* fcNodePtr() const override { return nullptr; }
  ConditionNode* fcNodePtr() override { return nullptr; }
  const ConditionFact* fcFactPtr() const override { return nullptr; }
  ConditionFact* fcFactPtr() override { return nullptr; }
  const ConditionNumber* fcNbPtr() const override { return this; }
  ConditionNumber* fcNbPtr() override { return this; }

  int nb;
};



} // !cp


#endif // INCLUDE_CONTEXTUALPLANNER_CONDITION_HPP
