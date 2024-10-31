#ifndef INCLUDE_CONTEXTUALPLANNER_SETOFFACTS_HPP
#define INCLUDE_CONTEXTUALPLANNER_SETOFFACTS_HPP

#include "../util/api.hpp"
#include <list>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>
#include <sstream>


namespace cp
{
struct Fact;
struct Entity;
struct Ontology;
struct SetOfEntities;


struct CONTEXTUALPLANNER_API SetOfFacts
{
  SetOfFacts();

  static SetOfFacts fromPddl(const std::string& pStr,
                             std::size_t& pPos,
                             const Ontology& pOntology,
                             const SetOfEntities& pEntities,
                             bool pCanFactsBeRemoved = true);

  std::string toPddl(std::size_t pIdentation, bool pPrintTimeLessFactsToo) const;

  void addFactsFromPddl(const std::string& pStr,
                        std::size_t& pPos,
                        const Ontology& pOntology,
                        const SetOfEntities& pEntities,
                        bool pCanFactsBeRemoved = true);

  void add(const Fact& pFact,
           bool pCanBeRemoved = true);

  bool erase(const Fact& pValue);

  void clear();

  class SetOfFactIterator {
     public:
         // Constructor accepting reference to std::list<Toto*>
         SetOfFactIterator(const std::list<Fact>* listPtr)
           : _listPtr(listPtr),
             _list()
         {}

         SetOfFactIterator(std::list<Fact>&& list)
           : _listPtr(nullptr),
             _list(std::move(list))
         {}

         // Custom iterator class for non-const access
         class Iterator {
             typename std::list<Fact>::const_iterator iter;

         public:
             Iterator(typename std::list<Fact>::const_iterator it) : iter(it) {}

             // Overload the dereference operator to return Toto& instead of Toto*
             const Fact& operator*() const { return *iter; }

             // Pre-increment operator
             Iterator& operator++() {
                 ++iter;
                 return *this;
             }

             bool operator==(const Iterator& other) const { return iter == other.iter; }
             bool operator!=(const Iterator& other) const { return iter != other.iter; }
         };

         // Begin and end methods to return the custom iterator
         Iterator begin() const { return Iterator(_listPtr != nullptr ? _listPtr->begin() : _list.begin()); }
         Iterator end() const { return Iterator(_listPtr != nullptr ? _listPtr->end() : _list.end()); }
         bool empty() const { return begin() == end(); }
         std::string toStr() const;

     private:
         const std::list<Fact>* _listPtr;
         std::list<Fact> _list;
  };


  SetOfFactIterator find(const Fact& pFact,
                         bool pIgnoreFluent = false) const;

  const std::map<Fact, bool>& facts() const { return _facts; }

  /**
   * @brief Get the value of a fact in the world state.
   * @param[in] pFact Fact to extract the value.
   * @return The value of the fact in the world state, an empty string if the fact is not in the world state.
   */
  std::optional<Entity> getFactFluent(const Fact& pFact) const;

  /**
   * @brief Extract the potential arguments of a fact parameter.
   * @param[out] pPotentialArgumentsOfTheParameter The extracted the potential arguments of a fact parameter.
   * @param[in] pFact Fact to consider for the parameter.
   * @param[in] pParameter Parameter to consider in the fact.
   */
  void extractPotentialArgumentsOfAFactParameter(std::set<Entity>& pPotentialArgumentsOfTheParameter,
                                                 const Fact& pFact,
                                                 const std::string& pParameter) const;

  bool empty() const { return _facts.empty(); }

private:
  /// Fact to bool True if the fact is timeless
  std::map<Fact, bool> _facts;
  std::optional<std::map<std::string, std::list<Fact>>> _exactCallToListsOpt;
  std::optional<std::map<std::string, std::list<Fact>>> _exactCallWithoutFluentToListsOpt;
  struct ParameterToValues
  {
    ParameterToValues(std::size_t pNbOfArgs)
     : all(),
       argIdToArgValueToValues(pNbOfArgs),
       fluentValueToValues()
    {
    }
    std::list<Fact> all;
    std::vector<std::map<std::string, std::list<Fact>>> argIdToArgValueToValues;
    std::map<std::string, std::list<Fact>> fluentValueToValues;
  };
  std::map<std::string, ParameterToValues> _signatureToLists;

  bool _erase(const Fact& pValue);

  // TODO: can be static
  void _removeAValueForList(std::list<Fact>& pList,
                            const Fact& pValue) const;

  const std::list<Fact>* _findAnExactCall(const std::optional<std::map<std::string, std::list<Fact>>>& pExactCalls,
                                          const std::string& pExactCall) const;

};

} // !cp


#endif // INCLUDE_CONTEXTUALPLANNER_SETOFFACTS_HPP
