#include "plannerusingexternaldata.hpp"
#include <gtest/gtest.h>
#include <fstream>
#include <prioritizedgoalsplanner/types/domain.hpp>
#include <prioritizedgoalsplanner/types/problem.hpp>
#include <prioritizedgoalsplanner/util/serializer/deserializefrompddl.hpp>
#include <prioritizedgoalsplanner/util/serializer/serializeinpddl.hpp>
#include <prioritizedgoalsplanner/prioritizedgoalsplanner.hpp>


using namespace pgp;

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
  std::map<std::string, pgp::Domain> loadedDomains;
  auto domain = pgp::pddlToDomain(domainContent, loadedDomains);
  loadedDomains.emplace(domain.getName(), std::move(domain));

  auto problemContent = _getFileContent(directory + "/problem.pddl");
  pgp::DomainAndProblemPtrs domainAndProblemPtrs = pgp::pddlToProblem(problemContent, loadedDomains);
  auto& problem = *domainAndProblemPtrs.problemPtr;

  std::string expected = _getFileContentWithoutComments(directory + "/problem_plan_result.pddl");
  std::string actualPlan = pgp::planToPddl(pgp::planForEveryGoals(problem, domain, {}), domain);
  EXPECT_EQ(expected, actualPlan);
}


}


TEST_F(PlannerUsingExternalData, test_problemsInData_simple)
{
  _test_dataDirectory(PlannerUsingExternalData::dataPath, "simple");
}


TEST_F(PlannerUsingExternalData, test_problemsInData_prioritized_goals)
{
  _test_dataDirectory(PlannerUsingExternalData::dataPath, "prioritized_goals");
}
