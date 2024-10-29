#include <iostream>
#include <fstream>
#include <string>
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
}


int main(int argc, char* argv[])
{
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <domain_pddl_file> <problem_pddl_file>" << std::endl;
        return 1;
    }

    auto domainContent = _getFileContent(argv[1]);
    std::map<std::string, cp::Domain> loadedDomains;
    auto domain = cp::pddlToDomain(domainContent, loadedDomains);
    loadedDomains.emplace(domain.getName(), std::move(domain));

    auto problemContent = _getFileContent(argv[2]);
    cp::DomainAndProblemPtrs domainAndProblemPtrs = cp::pddlToProblem(problemContent, loadedDomains);
    auto& problem = *domainAndProblemPtrs.problemPtr;

    if (argc > 3 &&
        std::string(argv[3]) == "--print_successions")
    {
      std::cout << domain.printSuccessionCache() << std::endl;
      return 0;
    }

    std::cout << cp::planToPddl(cp::planForEveryGoals(problem, domain, {}), domain) << std::endl;
    return 0;
}
