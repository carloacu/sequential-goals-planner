#include <prioritizedgoalsplanner/types/historical.hpp>


namespace cp
{


void Historical::setMutex(std::shared_ptr<std::mutex> pMutex)
{
  _mutexPtr = std::move(pMutex);
}

void Historical::notifyActionDone(const ActionId& pActionId)
{
  if (_mutexPtr)
  {
    std::lock_guard<std::mutex> lock(*_mutexPtr);
    _notifyActionDone(pActionId);
  }
  else
  {
    _notifyActionDone(pActionId);
  }
}


void Historical::_notifyActionDone(const ActionId& pActionId)
{
  _actionToNumberOfTimeAleardyDone.emplace(pActionId, 0);
  ++_actionToNumberOfTimeAleardyDone[pActionId];
}


bool Historical::hasActionAlreadyBeenDone(const ActionId& pActionId) const
{
  if (_mutexPtr)
  {
    std::lock_guard<std::mutex> lock(*_mutexPtr);
    return _hasActionAlreadyBeenDone(pActionId);
  }
  return _hasActionAlreadyBeenDone(pActionId);
}

std::size_t Historical::getNbOfTimeAnActionHasAlreadyBeenDone(const ActionId& pActionId) const
{
  if (_mutexPtr)
  {
    std::lock_guard<std::mutex> lock(*_mutexPtr);
    return _getNbOfTimeAnActionHasAlreadyBeenDone(pActionId);
  }
  return _getNbOfTimeAnActionHasAlreadyBeenDone(pActionId);
}

bool Historical::_hasActionAlreadyBeenDone(const ActionId& pActionId) const
{
  return _actionToNumberOfTimeAleardyDone.count(pActionId) > 0;
}

std::size_t Historical::_getNbOfTimeAnActionHasAlreadyBeenDone(const ActionId& pActionId) const
{
  auto it = _actionToNumberOfTimeAleardyDone.find(pActionId);
  if (it == _actionToNumberOfTimeAleardyDone.end())
    return 0;
  return it->second;
}


} // !cp
