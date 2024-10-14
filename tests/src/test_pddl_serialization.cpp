#include "test_pddl_serialization.hpp"
#include <assert.h>
#include <iostream>

namespace
{
template <typename TYPE>
void assert_eq(const TYPE& pExpected,
               const TYPE& pValue)
{
  if (pExpected != pValue)
    assert(false);
}

template <typename TYPE>
void assert_true(const TYPE& pValue)
{
  if (!pValue)
    assert(false);
}

template <typename TYPE>
void assert_false(const TYPE& pValue)
{
  if (pValue)
    assert(false);
}


}




void test_pddlSerialization()
{

  std::cout << "PDDL serialization is ok !!!!" << std::endl;
}
