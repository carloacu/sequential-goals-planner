#include "test_problems_in_data.hpp"
#include <fstream>
#include <iostream>
#include <contextualplanner/types/domain.hpp>
#include <contextualplanner/types/problem.hpp>
#include <contextualplanner/util/serializer/deserializefrompddl.hpp>
#include <contextualplanner/util/serializer/serializeinpddl.hpp>
#include <contextualplanner/contextualplanner.hpp>


using namespace cp;

namespace
{

std::string _getFileContent(const std::string& filePath)
{
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Error: Could not open file " + filePath);
    }

    std::string res;
    std::string line;
    while (std::getline(file, line))
        res += line + "\n";

    file.close();
    return res;
}


void _test_dataDirectory(const std::string& pDataPath,
                         const std::string& pProblemDirectory)
{
  auto directory = pDataPath + "/" + pProblemDirectory;

  auto domainContent = _getFileContent(directory + "/domain.pddl");
  std::map<std::string, cp::Domain> loadedDomains;
  auto domain = cp::pddlToDomain(domainContent, loadedDomains);
  loadedDomains.emplace(domain.getName(), std::move(domain));

  auto problemContent = _getFileContent(directory + "/problem.pddl");
  cp::DomainAndProblemPtrs domainAndProblemPtrs = cp::pddlToProblem(problemContent, loadedDomains);
  auto& problem = *domainAndProblemPtrs.problemPtr;

  std::string expected =
      R"(00: (move robot1 locationA locationB) [1]
01: (pick-up robot1 box1 locationB) [1]
02: (move robot1 locationB locationA) [1]
03: (drop robot1 box1 locationA) [1]
)";
  std::string actualPlan = cp::planToPddl(cp::planForEveryGoals(problem, domain, {}), domain);
  if (actualPlan != expected)
  {
    std::cout << "\n" << actualPlan << std::endl;
    assert(false);
  }
}


}

void test_problemsInData(const std::string& pDataPath)
{
  _test_dataDirectory(pDataPath, "simple");
}
