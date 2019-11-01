#include <maf/messaging/client-server/IAMessageRouter.h>
#include <maf/messaging/client-server/ServiceRequesterInterface.h>
#include <maf/messaging/client-server/ServiceProviderInterface.h>

namespace maf {
namespace messaging {


bool IAMessageRouter::init(const Address &, long long)
{
    return ClientBase::init({}, {}) && ServerBase::init({});
} 

bool IAMessageRouter::deinit()
{
    return ClientBase::deinit() && ClientBase::deinit();
}

bool IAMessageRouter::registerServiceRequester(const std::shared_ptr<ServiceRequesterInterface> &requester)
{
    if(ClientBase::registerServiceRequester(requester))
    {
        if(ServerBase::hasServiceProvider(requester->serviceID()))
        {
            requester->onServiceStatusChanged(requester->serviceID(), Availability::Unavailable, Availability::Available);
        }
        return true;
    }
    else
    {
        return false;
    }
}

DataTransmissionErrorCode IAMessageRouter::sendMessageToClient(const CSMessagePtr &msg, const Address &/*addr*/)
{
    if(ClientBase::onIncomingMessage(msg))
    {
        return DataTransmissionErrorCode::Success;
    }
    else {
        return DataTransmissionErrorCode::ReceiverUnavailable;
    }
}

DataTransmissionErrorCode IAMessageRouter::sendMessageToServer(const CSMessagePtr &msg)
{
    msg->setSourceAddress(Address{"", 0}); //BUG: later must be validated by validator
    if(ServerBase::onIncomingMessage(msg))
    {
        return DataTransmissionErrorCode::Success;
    }
    else
    {
        return DataTransmissionErrorCode::ReceiverUnavailable;
    }
}

void IAMessageRouter::notifyServiceStatusToClient(ServiceID sid, Availability oldStatus, Availability newStatus)
{
    ClientBase::onServiceStatusChanged(sid, oldStatus, newStatus);
}

}
}
