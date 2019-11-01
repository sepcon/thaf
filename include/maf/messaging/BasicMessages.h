#ifndef MESSAGES_H
#define MESSAGES_H

#include "MessageBase.h"
#include <functional>

namespace maf {
namespace messaging {

struct CallbackExcMsg : public MessageBase
{
    CallbackExcMsg(std::function<void()> callback_ = nullptr) : callback(std::move(callback_)) {}
    void execute() { if(callback) callback(); }
    std::function<void()> callback;
};
struct TimeoutMessage : public CallbackExcMsg
{
    TimeoutMessage(unsigned int timerID_, std::function<void()> timeoutCallback): CallbackExcMsg(std::move(timeoutCallback)), timerID(timerID_)
    {
        setPriority(1000);
    }
    unsigned int timerID;
};
}
}
#endif // MESSAGES_H
