#ifndef INCLUDE_CONTEXTUALPLANNER_FACTCONDITION_HPP
#define INCLUDE_CONTEXTUALPLANNER_FACTCONDITION_HPP

#include <functional>
#include <memory>
#include "../util/api.hpp"
#include "expression.hpp"
#include "factoptional.hpp"


namespace cp
{
struct Problem;
struct FactConditionNode;
struct FactConditionFact;
struct FactConditionExpression;

enum class FactConditionType
{
  NODE,
  FACT,
  EXPRESSION
};

struct CONTEXTUALPLANNER_API FactCondition
{
  FactCondition(FactConditionType pType);

  virtual bool hasFact(const cp::Fact& pFact) const = 0;
  virtual bool containsFact(const Fact& pFact) const = 0;
  virtual bool containsNotFact(const Fact& pFact) const = 0;
  virtual bool containsExpression(const Expression& pExpression) const = 0;
  virtual void replaceFact(const cp::Fact& pOldFact,
                           const Fact& pNewFact) = 0;
  virtual void forAll(const std::function<void (const FactOptional&)>& pFactCallback,
                      const std::function<void (const Expression&)>& pExpCallback) const = 0;
  virtual bool untilFalse(const std::function<bool (const FactOptional&)>& pFactCallback,
                          const std::function<bool (const Expression&)>& pExpCallback,
                          const Problem& pProblem,
                          const std::map<std::string, std::string>& pParameters) const = 0;
  virtual bool canBeTrue() const = 0;
  virtual bool isTrue(const Problem& pProblem,
                      const std::set<Fact>& pPunctualFacts = {},
                      const std::set<Fact>& pRemovedFacts = {},
                      std::map<std::string, std::string>* pParametersPtr = nullptr) const = 0;
  virtual bool canBecomeTrue(const Problem& pProblem) const = 0;
  virtual bool operator==(const FactCondition& pOther) const = 0;

  virtual std::unique_ptr<FactCondition> clone(const std::map<std::string, std::string>* pParametersPtr = nullptr) const = 0;

  virtual const FactConditionNode* fcNodePtr() const = 0;
  virtual FactConditionNode* fcNodePtr() = 0;

  virtual const FactConditionFact* fcFactPtr() const = 0;
  virtual FactConditionFact* fcFactPtr() = 0;

  virtual const FactConditionExpression* fcExpPtr() const = 0;
  virtual FactConditionExpression* fcExpPtr() = 0;

  static std::unique_ptr<FactCondition> fromStr(const std::string& pStr);
  virtual std::string toStr() const = 0;

  FactConditionType type;
};


enum class FactConditionNodeType
{
  EQUALITY,
  AND
};


struct CONTEXTUALPLANNER_API FactConditionNode : public FactCondition
{
  FactConditionNode(FactConditionNodeType pNodeType,
                    std::unique_ptr<FactCondition> pLeftOperand,
                    std::unique_ptr<FactCondition> pRightOperand);

  bool hasFact(const Fact& pFact) const override;
  bool containsFact(const Fact& pFact) const override;
  bool containsNotFact(const Fact& pFact) const override;
  bool containsExpression(const Expression& pExpression) const override;
  void replaceFact(const Fact& pOldFact,
                   const Fact& pNewFact) override;
  void forAll(const std::function<void (const FactOptional&)>& pFactCallback,
              const std::function<void (const Expression&)>& pExpCallback) const override;
  bool untilFalse(const std::function<bool (const FactOptional&)>& pFactCallback,
                  const std::function<bool (const Expression&)>& pExpCallback,
                  const Problem& pProblem,
                  const std::map<std::string, std::string>& pParameters) const override;
  bool canBeTrue() const override;
  bool isTrue(const Problem& pProblem,
              const std::set<Fact>& pPunctualFacts,
              const std::set<Fact>& pRemovedFacts,
              std::map<std::string, std::string>* pParametersPtr) const override;
  bool canBecomeTrue(const Problem& pProblem) const override;
  bool operator==(const FactCondition& pOther) const override;

  std::unique_ptr<FactCondition> clone(const std::map<std::string, std::string>* pParametersPtr) const override;

  const FactConditionNode* fcNodePtr() const  { return this; }
  FactConditionNode* fcNodePtr()  { return this; }
  const FactConditionFact* fcFactPtr() const override { return nullptr; }
  FactConditionFact* fcFactPtr() override { return nullptr; }
  const FactConditionExpression* fcExpPtr() const override { return nullptr; }
  FactConditionExpression* fcExpPtr() override { return nullptr; }

  std::string toStr() const override;

  FactConditionNodeType nodeType;
  std::unique_ptr<FactCondition> leftOperand;
  std::unique_ptr<FactCondition> rightOperand;
};

struct CONTEXTUALPLANNER_API FactConditionFact : public FactCondition
{
  FactConditionFact(const FactOptional& pFactOptional);

  bool hasFact(const cp::Fact& pFact) const override;
  bool containsFact(const Fact& pFact) const override;
  bool containsNotFact(const Fact& pFact) const override;
  bool containsExpression(const Expression&) const override { return false; }
  void replaceFact(const cp::Fact& pOldFact,
                   const Fact& pNewFact) override;
  void forAll(const std::function<void (const FactOptional&)>& pFactCallback,
              const std::function<void (const Expression&)>&) const override { pFactCallback(factOptional); }
  bool untilFalse(const std::function<bool (const FactOptional&)>& pFactCallback,
                  const std::function<bool (const Expression&)>&,
                  const Problem&,
                  const std::map<std::string, std::string>&) const override { return pFactCallback(factOptional); }
  bool canBeTrue() const override { return factOptional.isFactNegated || !factOptional.fact.isUnreachable(); }
  bool isTrue(const Problem& pProblem,
              const std::set<Fact>& pPunctualFacts,
              const std::set<Fact>& pRemovedFacts,
              std::map<std::string, std::string>* pParametersPtr) const override;
  bool canBecomeTrue(const Problem& pProblem) const override;
  bool operator==(const FactCondition& pOther) const override;

  const FactConditionNode* fcNodePtr() const  { return nullptr; }
  FactConditionNode* fcNodePtr()  { return nullptr; }
  const FactConditionFact* fcFactPtr() const override { return this; }
  FactConditionFact* fcFactPtr() override { return this; }
  const FactConditionExpression* fcExpPtr() const override { return nullptr; }
  FactConditionExpression* fcExpPtr() override { return nullptr; }

  std::unique_ptr<FactCondition> clone(const std::map<std::string, std::string>* pParametersPtr) const override;

  std::string toStr() const override { return factOptional.toStr(); }

  FactOptional factOptional;
};

struct CONTEXTUALPLANNER_API FactConditionExpression : public FactCondition
{
  FactConditionExpression(const Expression& pExpression);

  bool hasFact(const cp::Fact& pFact) const override;
  bool containsFact(const Fact& pFact) const override { return false; }
  bool containsNotFact(const Fact& pFact) const override  { return false; }
  bool containsExpression(const Expression& pExpression) const override;
  void replaceFact(const cp::Fact& pOldFact,
                   const Fact& pNewFact) override;
  void forAll(const std::function<void (const FactOptional&)>&,
              const std::function<void (const Expression&)>& pExpCallback) const override { pExpCallback(expression); }
  bool untilFalse(const std::function<bool (const FactOptional&)>&,
                  const std::function<bool (const Expression&)>& pExpCallback,
                  const Problem&,
                  const std::map<std::string, std::string>&) const override { return pExpCallback(expression); }
  bool canBeTrue() const override { return true; }
  bool isTrue(const Problem& pProblem,
              const std::set<Fact>& pPunctualFacts,
              const std::set<Fact>& pRemovedFacts,
              std::map<std::string, std::string>* pParametersPtr) const override;
  bool canBecomeTrue(const Problem& pProblem) const override;
  bool operator==(const FactCondition& pOther) const override;

  std::unique_ptr<FactCondition> clone(const std::map<std::string, std::string>* pParametersPtr) const override;

  std::string toStr() const override { return "<an_expression>"; }

  const FactConditionNode* fcNodePtr() const  { return nullptr; }
  FactConditionNode* fcNodePtr()  { return nullptr; }
  const FactConditionFact* fcFactPtr() const override { return nullptr; }
  FactConditionFact* fcFactPtr() override { return nullptr; }
  const FactConditionExpression* fcExpPtr() const override { return this; }
  FactConditionExpression* fcExpPtr() override { return this; }

  Expression expression;
};


} // !cp


#endif // INCLUDE_CONTEXTUALPLANNER_FACTCONDITION_HPP
