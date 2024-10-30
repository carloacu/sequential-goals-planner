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

std::string _getFileContentWithoutComments(const std::string& filePath)
{
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Error: Could not open file " + filePath);
    }

    std::string res;
    std::string line;
    while (std::getline(file, line))
    {
      // Trim the line to remove any leading/trailing whitespace
      line.erase(0, line.find_first_not_of(" \t")); // trim left
      line.erase(line.find_last_not_of(" \t") + 1); // trim right

      // Ignore empty lines or lines starting with ';'
      if (line.empty() || line[0] == ';')
        continue;

      res += line + "\n";
    }

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

  std::string expected = _getFileContentWithoutComments(directory + "/problem_plan_result.pddl");
  std::string actualPlan = cp::planToPddl(cp::planForEveryGoals(problem, domain, {}), domain);
  if (actualPlan != expected)
  {
    std::cout << "Get:\n" << actualPlan << std::endl;
    std::cout << "Expected:\n" << expected << std::endl;
    assert(false);
  }
}


}

void test_problemsInData(const std::string& pDataPath)
{
  _test_dataDirectory(pDataPath, "simple");
}
