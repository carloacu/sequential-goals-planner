# Contextual Planner


## Description

This is C++14 library to do plannification adapted for social context.<br/>
A Kotlin version for Android is also available here https://github.com/carloacu/contextualplanner-android

The plannification part is highly inspirated from the PDDL language.<br/>
https://en.wikipedia.org/wiki/Planning_Domain_Definition_Language


### Definition of words

 * Action: Axiomatic thing that the bot can do
 * Domain: Set of all the actions that the bot can do.
 * Fact: Axiomatic knowledge
 * World: Set of facts currently true
 * Goal: Characteritics that the world should have. It is the motivation of the bot for doing actions that will modify the world in order to repect these characteristics.


### Drawbacks of existing planners

The plannification language has some drawbacks when it is applied to chatbot or to social robotics.

 * The choice of actions to do is led by minimizing the planning cost. (a planning cost is equal to the number of actions to do generaly)
   But for chatbot or social robotics we are often more interested about the most revelant action for the current context than shortest number of actions.

 * The associated algorithms are often costly. But in chatbot and social robotics we need reactivity.



### Improvements that this library is trying to solve


The improvements for chatbot and social robotics are:

 * The actions to do are chosen because each of them bring a significant step toward the goal resolution and
   because they are revelant according to the context. (context = facts already present in world)
   In other words, the planner tries to find a path to the goal that is the most revelant according to the context.

 * As we do not have to find the shortest path (because we focus about the context instead) and as we only need to find the next
   action, there is the possibility to have a solution much more optimized. For chatbot or social robotics it is important
   to be reactive even if the domain and the goals are bigs.



## Code documentation


[Here](include/contextualplanner/contextualplanner.hpp) are the documented headers of the main functions.

### Types

Here are the types providec by this library:

 * [Action](include/contextualplanner/types/action.hpp): Axiomatic thing that the bot can do.
 * [Domain](include/contextualplanner/types/domain.hpp): Set of all the actions that the bot can do.
 * [WorldModification](include/contextualplanner/types/worldmodification.hpp): Specification of a modification of the world.



## Examples of usage


Here is an example with only one action to do:

```cpp
#include <map>
#include <memory>
#include <assert.h>
#include <contextualplanner/contextualplanner.hpp>


void planningDummyExample()
{
  // Fact
  const std::string userIsGreeted = "user_is_greeted";

  // Action id
  const std::string sayHi = "say_hi";

  // Current clock to set to different functions
  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());

  // Initialize the domain with an action
  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(sayHi, cp::Action({}, {userIsGreeted}));
  cp::Domain domain(actions);

  // Initialize the problem with the goal to satisfy
  cp::Problem problem;
  problem.setGoals({userIsGreeted}, now);

  // Look for the next action to do
  std::map<std::string, std::string> parameters;
  auto actionToDo1 = cp::lookForAnActionToDo(parameters, problem, domain, now);
  assert(sayHi == actionToDo1); // The action found is "say_hi", because it is needed to satisfy the preconditions of "ask_how_I_can_help"
  // When the action is finished we notify the planner
  cp::notifyActionDone(problem, domain, actionToDo1, parameters, now);

  // Look for the next action to do
  auto actionToDo3 = cp::lookForAnActionToDo(parameters, problem, domain, now);
  assert("" == actionToDo3); // No action found
}
```


Here is an example with two actions to do and with the usage of preconditions:


```cpp
#include <map>
#include <memory>
#include <assert.h>
#include <contextualplanner/contextualplanner.hpp>


void planningExampleWithAPreconditionSolve()
{
  // Facts
  const std::string userIsGreeted = "user_is_greeted";
  const std::string proposedOurHelpToUser = "proposed_our_help_to_user";

  // Action ids
  const std::string sayHi = "say_hi";
  const std::string askHowICanHelp = "ask_how_I_can_help";

  // Current clock to set to different functions
  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());

  // Initialize the domain with a set of actions
  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(sayHi, cp::Action({}, {userIsGreeted}));
  actions.emplace(askHowICanHelp, cp::Action({userIsGreeted}, {proposedOurHelpToUser}));
  cp::Domain domain(actions);

  // Initialize the problem with the goal to satisfy
  cp::Problem problem;
  problem.setGoals({proposedOurHelpToUser}, now);

  // Look for the next action to do
  std::map<std::string, std::string> parameters;
  auto actionToDo1 = cp::lookForAnActionToDo(parameters, problem, domain, now);
  assert(sayHi == actionToDo1); // The action found is "say_hi", because it is needed to satisfy the preconditions of "ask_how_I_can_help"
  // When the action is finished we notify the planner
  cp::notifyActionDone(problem, domain, actionToDo1, parameters, now);

  // Look for the next action to do
  auto actionToDo2 = cp::lookForAnActionToDo(parameters, problem, domain, now);
  assert(askHowICanHelp == actionToDo2); // The action found is "ask_how_I_can_help"
  // When the action is finished we notify the planner
  cp::notifyActionDone(problem, domain, actionToDo2, parameters, now);

  // Look for the next action to do
  auto actionToDo3 = cp::lookForAnActionToDo(parameters, problem, domain, now);
  assert("" == actionToDo3); // No action found
}
```
