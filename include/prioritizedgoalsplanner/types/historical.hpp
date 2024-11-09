#ifndef INCLUDE_CONTEXTUALPLANNER_TYPES_HISTORICAL_HPP
#define INCLUDE_CONTEXTUALPLANNER_TYPES_HISTORICAL_HPP

#include <mutex>
#include <memory>
#include <map>
#include "../util/api.hpp"
#include <prioritizedgoalsplanner/util/alias.hpp>


namespace cp
{


/// Container of the actions already done.
struct CONTEXTUALPLANNER_API Historical
{
  /// Set the mutex.
  void setMutex(std::shared_ptr<std::mutex> pMutex);

  /**
   * @brief Notify that an action finished.
   * @param pActionId Identifier of the finished action.
   */
  void notifyActionDone(const ActionId& pActionId);

  /**
   * @brief Has an action already been done.
   * @param pActionId Action identifier.
   * @return True if the action has already been done, false otherwise.
   */
  bool hasActionAlreadyBeenDone(const ActionId& pActionId) const;

  /**
   * @brief Get the number of time that an action has already been done.
   * @param pActionId Action identifier.
   * @return The number of time that the action has already been done.
   */
  std::size_t getNbOfTimeAnActionHasAlreadyBeenDone(const ActionId& pActionId) const;

private:
  /// Mutex to proect this struct.
  std::shared_ptr<std::mutex> _mutexPtr;
  /// Action to the number of time the action has already been done.
  std::map<ActionId, std::size_t> _actionToNumberOfTimeAleardyDone;

  // Notify that an action finished.
  void _notifyActionDone(const ActionId& pActionId);
  // Has an action already been done.
  bool _hasActionAlreadyBeenDone(const ActionId& pActionId) const;
  // Get the number of time that an action has already been done.
  std::size_t _getNbOfTimeAnActionHasAlreadyBeenDone(const ActionId& pActionId) const;
};


} // !cp


#endif // INCLUDE_CONTEXTUALPLANNER_TYPES_HISTORICAL_HPP
