#ifndef INCLUDE_CONTEXTUALPLANNER_FACTMODIFICATION_HPP
#define INCLUDE_CONTEXTUALPLANNER_FACTMODIFICATION_HPP

#include <functional>
#include <memory>
#include "../util/api.hpp"
#include "expression.hpp"
#include "factoptional.hpp"


namespace cp
{
struct Problem;
struct FactModificationFact;
struct FactModificationNumber;

enum class FactModificationType
{
  NODE,
  FACT,
  EXPRESSION,
  NUMBER
};

struct CONTEXTUALPLANNER_API FactModification
{
  FactModification(FactModificationType pType);

  virtual bool hasFact(const cp::Fact& pFact) const = 0;
  virtual bool canModifySomethingInTheWorld() const = 0;
  virtual bool isDynamic() const = 0;

  virtual void replaceFact(const cp::Fact& pOldFact,
                           const Fact& pNewFact) = 0;
  virtual void forAll(const std::function<void (const FactOptional&)>& pFactCallback,
                      const std::function<void (const Expression&)>& pExpCallback,
                      const Problem& pProblem) const = 0;
  virtual bool forAllFactsOptUntilTrue(const std::function<bool (const FactOptional&)>& pFactCallback,
                                       const Problem& pProblem) const = 0;
  virtual void forAllFacts(const std::function<void (const FactOptional&)>& pFactCallback,
                           const Problem& pProblem) const = 0;
  virtual bool forAllExpUntilTrue(const std::function<bool (const Expression&)>& pExpCallback) const = 0;

  virtual std::unique_ptr<FactModification> clone(const std::map<std::string, std::string>* pParametersPtr) const = 0;
  virtual std::unique_ptr<FactModification> cloneParamSet(const std::map<std::string, std::set<std::string>>& pParameters) const = 0;
  virtual const FactModificationFact* fcFactPtr() const = 0;
  virtual const FactModificationNumber* fcNumberPtr() const = 0;

  FactModificationType type;

  static std::unique_ptr<FactModification> fromStrWithExps(const std::string& pStr);
  static std::unique_ptr<FactModification> fromStr(const std::string& pStr);
  virtual std::string toStr() const = 0;

  static std::unique_ptr<FactModification> merge(const FactModification& pFactModification1,
                                                 const FactModification& pFactModification2);
};


enum class FactModificationNodeType
{
  AND,
  SET,
  FOR_ALL,
  ADD,
  PLUS
};


struct CONTEXTUALPLANNER_API FactModificationNode : public FactModification
{
  FactModificationNode(FactModificationNodeType pNodeType,
                       std::unique_ptr<FactModification> pLeftOperand,
                       std::unique_ptr<FactModification> pRightOperand,
                       const std::string& pParameterName = "");

  bool hasFact(const Fact& pFact) const override;
  bool canModifySomethingInTheWorld() const override;
  bool isDynamic() const override;

  void replaceFact(const Fact& pOldFact,
                   const Fact& pNewFact) override;
  void forAll(const std::function<void (const FactOptional&)>& pFactCallback,
              const std::function<void (const Expression&)>& pExpCallback,
              const Problem& pProblem) const override;
  bool forAllFactsOptUntilTrue(const std::function<bool (const FactOptional&)>& pFactCallback,
                               const Problem& pProblem) const override;
  void forAllFacts(const std::function<void (const FactOptional&)>& pFactCallback,
                   const Problem& pProblem) const override;
  bool forAllExpUntilTrue(const std::function<bool (const Expression&)>& pExpCallback) const override;

  std::unique_ptr<FactModification> clone(const std::map<std::string, std::string>* pParametersPtr) const override;
  std::unique_ptr<FactModification> cloneParamSet(const std::map<std::string, std::set<std::string>>& pParameters) const override;
  const FactModificationFact* fcFactPtr() const override { return nullptr; }
  const FactModificationNumber* fcNumberPtr() const override { return nullptr; }

  std::string toStr() const override;

  FactModificationNodeType nodeType;
  std::unique_ptr<FactModification> leftOperand;
  std::unique_ptr<FactModification> rightOperand;
  std::string parameterName;

private:
  void _forAllInstruction(const std::function<void (const FactModification&)>& pCallback,
                          const Problem& pProblem) const;
};

struct CONTEXTUALPLANNER_API FactModificationFact : public FactModification
{
  FactModificationFact(const FactOptional& pFactOptional);

  bool hasFact(const cp::Fact& pFact) const override;
  bool canModifySomethingInTheWorld() const override;
  bool isDynamic() const override { return false; }

  void replaceFact(const cp::Fact& pOldFact,
                   const Fact& pNewFact) override;
  void forAll(const std::function<void (const FactOptional&)>& pFactCallback,
              const std::function<void (const Expression&)>&,
              const Problem&) const override { pFactCallback(factOptional); }
  bool forAllFactsOptUntilTrue(const std::function<bool (const FactOptional&)>& pFactCallback,
                               const Problem& pProblem) const override;
  void forAllFacts(const std::function<void (const FactOptional&)>& pFactCallback,
                   const Problem& pProblem) const override;
  bool forAllExpUntilTrue(const std::function<bool (const Expression&)>&) const override { return false; }

  std::unique_ptr<FactModification> clone(const std::map<std::string, std::string>* pParametersPtr) const override;
  std::unique_ptr<FactModification> cloneParamSet(const std::map<std::string, std::set<std::string>>& pParameters) const override;
  const FactModificationFact* fcFactPtr() const override { return this; }
  const FactModificationNumber* fcNumberPtr() const override { return nullptr; }

  std::string toStr() const override { return factOptional.toStr(); }

  FactOptional factOptional;
};

struct CONTEXTUALPLANNER_API FactModificationExpression : public FactModification
{
  FactModificationExpression(const Expression& pExpression);

  bool hasFact(const cp::Fact& pFact) const override;
  bool canModifySomethingInTheWorld() const override { return true; }
  bool isDynamic() const override { return false; }

  void replaceFact(const cp::Fact& pOldFact,
                   const Fact& pNewFact) override;
  void forAll(const std::function<void (const FactOptional&)>&,
              const std::function<void (const Expression&)>& pExpCallback,
              const Problem&) const override { pExpCallback(expression); }
  bool forAllFactsOptUntilTrue(const std::function<bool (const FactOptional&)>&,
                               const Problem&) const override { return false; }
  void forAllFacts(const std::function<void (const FactOptional&)>&,
                   const Problem&) const override {}
  bool forAllExpUntilTrue(const std::function<bool (const Expression&)>& pExpCallback) const override { return pExpCallback(expression); }

  std::unique_ptr<FactModification> clone(const std::map<std::string, std::string>* pParametersPtr) const override;
  std::unique_ptr<FactModification> cloneParamSet(const std::map<std::string, std::set<std::string>>& pParameters) const override;
  const FactModificationFact* fcFactPtr() const override { return nullptr; }
  const FactModificationNumber* fcNumberPtr() const override { return nullptr; }

  std::string toStr() const override { return "<an_expression>"; }

  Expression expression;
};


struct CONTEXTUALPLANNER_API FactModificationNumber : public FactModification
{
  FactModificationNumber(int pNb);

  bool hasFact(const cp::Fact& pFact) const override { return false; }
  bool canModifySomethingInTheWorld() const override { return false; }
  bool isDynamic() const override { return false; }

  void replaceFact(const cp::Fact& pOldFact,
                   const Fact& pNewFact) override {}
  void forAll(const std::function<void (const FactOptional&)>&,
              const std::function<void (const Expression&)>&,
              const Problem&) const override {}
  bool forAllFactsOptUntilTrue(const std::function<bool (const FactOptional&)>&,
                               const Problem&) const override { return false; }
  void forAllFacts(const std::function<void (const FactOptional&)>&,
                   const Problem&) const override {}
  bool forAllExpUntilTrue(const std::function<bool (const Expression&)>& pExpCallback) const override { return false; }

  std::unique_ptr<FactModification> clone(const std::map<std::string, std::string>* pParametersPtr) const override;
  std::unique_ptr<FactModification> cloneParamSet(const std::map<std::string, std::set<std::string>>& pParameters) const override;
  const FactModificationFact* fcFactPtr() const override { return nullptr; }
  const FactModificationNumber* fcNumberPtr() const override { return this; }

  std::string toStr() const override;
  int nb;
};

} // !cp


#endif // INCLUDE_CONTEXTUALPLANNER_FACTMODIFICATION_HPP
