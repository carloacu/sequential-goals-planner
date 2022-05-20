#ifndef INCLUDE_CONTEXTUALPLANNER_HISTORICAL_HPP
#define INCLUDE_CONTEXTUALPLANNER_HISTORICAL_HPP


#include "api.hpp"
#include <mutex>
#include <memory>
#include <map>
#include <contextualplanner/alias.hpp>



namespace cp
{


struct CONTEXTUALPLANNER_API Historical
{
  void setMutex(std::shared_ptr<std::mutex> pMutex);
  void notifyActionDone(const ActionId& pActionId);
  bool hasActionId(const ActionId& pActionId) const;
  std::size_t getNbOfTimeAnActionHasAlreadyBeenDone(const ActionId& pActionId) const;
private:
  std::shared_ptr<std::mutex> _mutexPtr;
  std::map<ActionId, std::size_t> _actionToNumberOfTimeAleardyDone;

  void _notifyActionDone(const ActionId& pActionId);
  bool _hasActionId(const ActionId& pActionId) const;
  std::size_t _getNbOfTimeAnActionHasAlreadyBeenDone(const ActionId& pActionId) const;
};


} // !cp


#endif // INCLUDE_CONTEXTUALPLANNER_HISTORICAL_HPP
