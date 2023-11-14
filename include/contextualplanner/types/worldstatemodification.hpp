#ifndef INCLUDE_CONTEXTUALPLANNER_WORLDSTATEMODIFICATION_HPP
#define INCLUDE_CONTEXTUALPLANNER_WORLDSTATEMODIFICATION_HPP

#include <functional>
#include <memory>
#include "../util/api.hpp"
#include "factoptional.hpp"


namespace cp
{
struct WorldState;
struct WorldStateModificationNode;
struct WorldStateModificationFact;
struct WorldStateModificationNumber;

enum class WorldStateModificationType
{
  NODE,
  FACT,
  NUMBER
};

struct CONTEXTUALPLANNER_API WorldStateModification
{
  WorldStateModification(WorldStateModificationType pType);

  virtual bool hasFact(const cp::Fact& pFact) const = 0;
  virtual bool canModifySomethingInTheWorld() const = 0;
  virtual bool isDynamic() const = 0;

  virtual void replaceFact(const cp::Fact& pOldFact,
                           const Fact& pNewFact) = 0;
  virtual void forAll(const std::function<void (const FactOptional&)>& pFactCallback,
                      const WorldState& pWorldState) const = 0;
  virtual bool forAllUntilTrue(const std::function<bool (const FactOptional&)>& pFactCallback,
                               const WorldState& pWorldState) const = 0;

  virtual bool operator==(const WorldStateModification& pOther) const = 0;
  virtual bool operator!=(const WorldStateModification& pOther) const { return !operator==(pOther); }

  virtual std::string getValue(const WorldState& pWorldState) const = 0;

  virtual std::unique_ptr<WorldStateModification> clone(const std::map<std::string, std::string>* pParametersPtr) const = 0;
  virtual std::unique_ptr<WorldStateModification> cloneParamSet(const std::map<std::string, std::set<std::string>>& pParameters) const = 0;
  virtual const WorldStateModificationNode* fmNodePtr() const = 0;
  virtual const WorldStateModificationFact* fcFactPtr() const = 0;
  virtual const WorldStateModificationNumber* fcNumberPtr() const = 0;

  WorldStateModificationType type;

  static std::unique_ptr<WorldStateModification> fromStr(const std::string& pStr);
  virtual std::string toStr() const = 0;

  static std::unique_ptr<WorldStateModification> merge(const WorldStateModification& pWsModif1,
                                                       const WorldStateModification& pWsModif2);
};


enum class WorldStateModificationNodeType
{
  AND,
  SET,
  FOR_ALL,
  ADD,
  PLUS,
  MINUS
};


struct CONTEXTUALPLANNER_API WorldStateModificationNode : public WorldStateModification
{
  WorldStateModificationNode(WorldStateModificationNodeType pNodeType,
                             std::unique_ptr<WorldStateModification> pLeftOperand,
                             std::unique_ptr<WorldStateModification> pRightOperand,
                             const std::string& pParameterName = "");

  bool hasFact(const Fact& pFact) const override;
  bool canModifySomethingInTheWorld() const override;
  bool isDynamic() const override;

  void replaceFact(const Fact& pOldFact,
                   const Fact& pNewFact) override;
  void forAll(const std::function<void (const FactOptional&)>& pFactCallback,
              const WorldState& pWorldState) const override;
  bool forAllUntilTrue(const std::function<bool (const FactOptional&)>& pFactCallback,
                       const WorldState& pWorldState) const override;
  bool operator==(const WorldStateModification& pOther) const override;

  std::string getValue(const WorldState& pWorldState) const override;

  std::unique_ptr<WorldStateModification> clone(const std::map<std::string, std::string>* pParametersPtr) const override;
  std::unique_ptr<WorldStateModification> cloneParamSet(const std::map<std::string, std::set<std::string>>& pParameters) const override;
  const WorldStateModificationNode* fmNodePtr() const { return this; }
  const WorldStateModificationFact* fcFactPtr() const override { return nullptr; }
  const WorldStateModificationNumber* fcNumberPtr() const override { return nullptr; }

  std::string toStr() const override;

  WorldStateModificationNodeType nodeType;
  std::unique_ptr<WorldStateModification> leftOperand;
  std::unique_ptr<WorldStateModification> rightOperand;
  std::string parameterName;

private:
  void _forAllInstruction(const std::function<void (const WorldStateModification&)>& pCallback,
                          const WorldState& pWorldState) const;
};

struct CONTEXTUALPLANNER_API WorldStateModificationFact : public WorldStateModification
{
  WorldStateModificationFact(const FactOptional& pFactOptional);

  bool hasFact(const cp::Fact& pFact) const override;
  bool canModifySomethingInTheWorld() const override;
  bool isDynamic() const override { return false; }

  void replaceFact(const cp::Fact& pOldFact,
                   const Fact& pNewFact) override;
  void forAll(const std::function<void (const FactOptional&)>& pFactCallback,
              const WorldState&) const override { pFactCallback(factOptional); }
  bool forAllUntilTrue(const std::function<bool (const FactOptional&)>& pFactCallback,
                       const WorldState& pWorldState) const override;
  bool operator==(const WorldStateModification& pOther) const override;

  std::string getValue(const WorldState& pWorldState) const override;

  std::unique_ptr<WorldStateModification> clone(const std::map<std::string, std::string>* pParametersPtr) const override;
  std::unique_ptr<WorldStateModification> cloneParamSet(const std::map<std::string, std::set<std::string>>& pParameters) const override;
  const WorldStateModificationNode* fmNodePtr() const { return nullptr; }
  const WorldStateModificationFact* fcFactPtr() const override { return this; }
  const WorldStateModificationNumber* fcNumberPtr() const override { return nullptr; }

  std::string toStr() const override { return factOptional.toStr(); }

  FactOptional factOptional;
};


struct CONTEXTUALPLANNER_API WorldStateModificationNumber : public WorldStateModification
{
  WorldStateModificationNumber(int pNb);

  bool hasFact(const cp::Fact& pFact) const override { return false; }
  bool canModifySomethingInTheWorld() const override { return false; }
  bool isDynamic() const override { return false; }

  void replaceFact(const cp::Fact& pOldFact,
                   const Fact& pNewFact) override {}
  void forAll(const std::function<void (const FactOptional&)>&,
              const WorldState&) const override {}
  bool forAllUntilTrue(const std::function<bool (const FactOptional&)>&,
                       const WorldState&) const override { return false; }
  bool operator==(const WorldStateModification& pOther) const override;

  std::string getValue(const WorldState& pWorldState) const override;

  std::unique_ptr<WorldStateModification> clone(const std::map<std::string, std::string>* pParametersPtr) const override;
  std::unique_ptr<WorldStateModification> cloneParamSet(const std::map<std::string, std::set<std::string>>& pParameters) const override;
  const WorldStateModificationNode* fmNodePtr() const { return nullptr; }
  const WorldStateModificationFact* fcFactPtr() const override { return nullptr; }
  const WorldStateModificationNumber* fcNumberPtr() const override { return this; }

  std::string toStr() const override;
  int nb;
};

} // !cp


#endif // INCLUDE_CONTEXTUALPLANNER_WORLDSTATEMODIFICATION_HPP
