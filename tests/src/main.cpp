#include "plannerusingexternaldata.hpp"
#include <prioritizedgoalsplanner/util/util.hpp>

std::string PlannerUsingExternalData::dataPath = "";

// Arguments to set: --dataPath ../../data/


int main(int argc, char **argv)
{
  for (int i = 0; i < argc; ++i)
  {
    if ((i + 1) < argc)
    {
      const std::string currAgrv = argv[i];
      if (currAgrv == "--dataPath")
        PlannerUsingExternalData::dataPath = argv[i + 1];
    }
  }

  cp::CONTEXTUALPLANNER_DEBUG_FOR_TESTS = true;
  ::testing::InitGoogleTest(&argc, argv);
  int res = RUN_ALL_TESTS();
  return res;
}
