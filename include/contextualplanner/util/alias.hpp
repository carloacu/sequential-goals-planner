#ifndef INCLUDE_CONTEXTUALPLANNER_ALIAS_HPP
#define INCLUDE_CONTEXTUALPLANNER_ALIAS_HPP

#include <string>


namespace cp
{
using ActionId = std::string;
using SetOfEventsId = std::string;
using EventId = std::string;

using FullEventId = std::string;


static const FullEventId generateFullEventId(const SetOfEventsId& pSetOfEventsId, const EventId& pEventId)
{
  return pSetOfEventsId + "|" + pEventId;
}


} // !cp


#endif // INCLUDE_CONTEXTUALPLANNER_ALIAS_HPP
