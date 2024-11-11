#ifndef INCLUDE_PRIORITIZEDGOALSPLANNER_ALIAS_HPP
#define INCLUDE_PRIORITIZEDGOALSPLANNER_ALIAS_HPP

#include <string>


namespace pgp
{
using ActionId = std::string;
using SetOfEventsId = std::string;
using EventId = std::string;

using FullEventId = std::string;


static const FullEventId generateFullEventId(const SetOfEventsId& pSetOfEventsId, const EventId& pEventId)
{
  return pSetOfEventsId + "|" + pEventId;
}


} // !pgp


#endif // INCLUDE_PRIORITIZEDGOALSPLANNER_ALIAS_HPP
