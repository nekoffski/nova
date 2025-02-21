#include "Core.hh"

namespace sl::details {

EventHandlerRecord::EventHandlerRecord(EventCallback&& callback) :
    callback(std::forward<EventCallback>(callback)) {}

}  // namespace sl::details
