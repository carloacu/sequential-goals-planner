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

enum class FactModificationType
{
  NODE,
  FACT,
  EXPRESSION
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
  virtual void forAllFacts(const std::function<void (const Fact&)>& pFactCallback,
                           const Problem& pProblem) const = 0;
  virtual bool forAllFactsUntilTrue(const std::function<bool (const Fact&)>& pFactCallback,
                                    const Problem& pProblem) const = 0;
  virtual void forAllNotFacts(const std::function<void (const Fact&)>& pFactCallback,
                              const Problem& pProblem) const = 0;
  virtual bool forAllNotFactsUntilTrue(const std::function<bool (const Fact&)>& pFactCallback,
                                       const Problem& pProblem) const = 0;
  virtual bool forAllExpUntilTrue(const std::function<bool (const Expression&)>& pExpCallback) const = 0;

  virtual std::unique_ptr<FactModification> clone(const std::map<std::string, std::string>* pParametersPtr) const = 0;
  virtual const FactModificationFact* fcFactPtr() const = 0;

  FactModificationType type;

  static std::unique_ptr<FactModification> fromStr(const std::string& pStr);
  static std::unique_ptr<FactModification> merge(const FactModification& pFactModification1,
                                                 const FactModification& pFactModification2);
};


enum class FactModificationNodeType
{
  AND,
  SET,
  FOR_ALL
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
  void forAllFacts(const std::function<void (const Fact&)>& pFactCallback,
                   const Problem& pProblem) const override;
  bool forAllFactsUntilTrue(const std::function<bool (const Fact&)>& pFactCallback,
                            const Problem& pProblem) const override;
  void forAllNotFacts(const std::function<void (const Fact&)>& pFactCallback,
                      const Problem& pProblem) const override;
  bool forAllNotFactsUntilTrue(const std::function<bool (const Fact&)>& pFactCallback,
                               const Problem& pProblem) const override;
  bool forAllExpUntilTrue(const std::function<bool (const Expression&)>& pExpCallback) const override;

  std::unique_ptr<FactModification> clone(const std::map<std::string, std::string>* pParametersPtr) const override;
  const FactModificationFact* fcFactPtr() const override { return nullptr; }

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
  void forAllFacts(const std::function<void (const Fact&)>& pFactCallback,
                   const Problem& pProblem) const override;
  bool forAllFactsUntilTrue(const std::function<bool (const Fact&)>& pFactCallback,
                            const Problem& pProblem) const override;
  void forAllNotFacts(const std::function<void (const Fact&)>& pFactCallback,
                      const Problem& pProblem) const override;
  bool forAllNotFactsUntilTrue(const std::function<bool (const Fact&)>& pFactCallback,
                               const Problem& pProblem) const override;
  bool forAllExpUntilTrue(const std::function<bool (const Expression&)>&) const override { return false; }

  std::unique_ptr<FactModification> clone(const std::map<std::string, std::string>* pParametersPtr) const override;
  const FactModificationFact* fcFactPtr() const override { return this; }

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
  void forAllFacts(const std::function<void (const Fact&)>&,
                   const Problem&) const override {}
  bool forAllFactsUntilTrue(const std::function<bool (const Fact&)>&,
                            const Problem&) const override { return false; }
  void forAllNotFacts(const std::function<void (const Fact&)>&,
                      const Problem&) const override {}
  bool forAllNotFactsUntilTrue(const std::function<bool (const Fact&)>&,
                               const Problem&) const override { return false; }
  bool forAllExpUntilTrue(const std::function<bool (const Expression&)>& pExpCallback) const override { return pExpCallback(expression); }

  std::unique_ptr<FactModification> clone(const std::map<std::string, std::string>* pParametersPtr) const override;
  const FactModificationFact* fcFactPtr() const override { return nullptr; }

  Expression expression;
};


} // !cp


#endif // INCLUDE_CONTEXTUALPLANNER_FACTMODIFICATION_HPP
