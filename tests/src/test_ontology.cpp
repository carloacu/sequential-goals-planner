#include "test_arithmeticevaluator.hpp"
#include <assert.h>
#include <iostream>
#include <contextualplanner/types/ontology.hpp>

namespace
{
template <typename TYPE>
void assert_eq(const TYPE& pExpected,
               const TYPE& pValue)
{
  if (pExpected != pValue)
    assert(false);
}
}




int test_ontology()
{
  cp::Ontology ontology;
  assert_eq<std::string>("", ontology.typesToStr());
  ontology.addType("object");
  ontology.addType("voiture", "object");
  ontology.addType("maison", "object");
  ontology.addType("citroen", "voiture");
  ontology.addType("ferrari", "voiture");
  ontology.addType("peugeot", "voiture");
  ontology.addType("c3", "citroen");
  ontology.addType("location");

  assert_eq<std::string>("voiture maison - object\n"
                         "citroen ferrari peugeot - voiture\n"
                         "c3 - citroen\n"
                         "location",
                         ontology.typesToStr());

  std::cout << "ontology is ok !!!!" << std::endl;
  return 0;
}
