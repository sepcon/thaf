#pragma once

#include "CSMessage.h"
#include "maf/patterns/Patterns.h"
#include "maf/utils/debugging/Debug.h"

namespace maf {
namespace messaging {

class CSMessage;
class ServiceStubBase;

enum class RequestResultStatus : bool
{
    Incomplete = false,
    Complete = true
};

class RequestKeeperBase : public pattern::UnCopyable
{
public:
    using AbortCallback = std::function<void(void)>;
    OpCode getOperationCode() const;
    OpID getOperationID() const;
    bool valid() const;
    bool respond(const CSMsgContentPtr& answer, RequestResultStatus status = RequestResultStatus::Complete);
    void update(const CSMsgContentPtr& answer);
    CSMsgContentPtr getRequestContent();
    void abortedBy(AbortCallback abortCallback);

protected:
    friend class ServiceStubBase;
    static std::shared_ptr<RequestKeeperBase> create(std::shared_ptr<CSMessage> csMsg, ServiceStubBase* svStub);
    RequestKeeperBase(std::shared_ptr<CSMessage> csMsg, messaging::ServiceStubBase* svStub);
    AbortCallback& getAbortCallback();
    void invalidate();

    std::shared_ptr<CSMessage> _csMsg;
    ServiceStubBase* _svStub;
    AbortCallback _abortCallback;
    std::atomic_bool _valid;
};

template<class MessageTrait>
class RequestKeeper : public RequestKeeperBase
{
public:
    template<class CSMessageContentSpecific>
    std::shared_ptr<CSMessageContentSpecific> getRequestContent();
    template<class CSMessageContentSpecific>
    bool respond(const std::shared_ptr<CSMessageContentSpecific>& answer, RequestResultStatus status = RequestResultStatus::Complete);
};


template<class MessageTrait> template<class CSMessageContentSpecific>
std::shared_ptr<CSMessageContentSpecific> RequestKeeper<MessageTrait>::getRequestContent()
{
    try
    {
        return MessageTrait::template translate<CSMessageContentSpecific>(RequestKeeperBase::getRequestContent());
    }
    catch(const std::exception& e)
    {
        mafErr("Trying to get content of message that carries no payload: " << e.what());
    }
    return {};
}

template<class MessageTrait> template<class CSMessageContentSpecific>
bool RequestKeeper<MessageTrait>::respond(const std::shared_ptr<CSMessageContentSpecific>& answer, RequestResultStatus status)
{
    auto csMsgContent = MessageTrait::template translate(answer);
    return RequestKeeperBase::respond(csMsgContent, status);
}

} // messaging
} // maf
