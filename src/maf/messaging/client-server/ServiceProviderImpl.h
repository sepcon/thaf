#pragma once
#include <list>
#include <maf/messaging/client-server/CSStatus.h>
#include <maf/messaging/client-server/ServiceProviderIF.h>
#include <maf/messaging/client-server/internal/CSShared.h>
#include <maf/threading/Lockable.h>
#include <map>
#include <set>

namespace maf {
namespace messaging {

class ServerIF;
class ServiceProvider;
class Request;

struct ServiceProviderImpl {
  // clang-format off
    template <typename ValueType>
    using OpIDMap                = threading::Lockable<std::map<OpID, ValueType>>;

    using RequestPtr             = std::shared_ptr<Request>;
    using PropertyPtr            = CSMsgContentBasePtr;
    using RequestMap             = OpIDMap<std::list<RequestPtr>>;
    using PropertyMap            = OpIDMap<PropertyPtr>;
    using RequestHandlerMap      = OpIDMap<RequestHandlerFunction>;
    using Address2OpIDsMap       = threading::Lockable<std::map<Address, std::set<OpID>>>;

    Address2OpIDsMap             regEntriesMap_;
    RequestMap                   requestsMap_;
    std::weak_ptr<ServerIF>      server_;
    ServiceProvider*             delegator_;
    PropertyMap                  propertyMap_;
    RequestHandlerMap            requestHandlerMap_;
    std::atomic<Availability>    availability_ = Availability::Unavailable;

  // clang-format on
  ServiceProviderImpl(ServiceProvider *holder, std::weak_ptr<ServerIF> server);

  ~ServiceProviderImpl();

  ActionCallStatus respondToRequest(const CSMessagePtr &csMsg);

  ActionCallStatus setStatus(const OpID &propertyID,
                             const CSMsgContentBasePtr &property);

  ActionCallStatus broadcastSignal(const OpID &signalID,
                                   const CSMsgContentBasePtr &signal);

  ActionCallStatus broadcast(const OpID &propertyID, OpCode opCode,
                             const CSMsgContentBasePtr &content);

  CSMsgContentBasePtr getStatus(const OpID &propertyID);

  Availability availability() const;

  void startServing();
  void stopServing();

  bool onIncomingMessage(const CSMessagePtr &msg);

  ActionCallStatus sendMessage(const CSMessagePtr &csMsg,
                               const Address &toAddr);
  ActionCallStatus sendBackMessageToClient(const CSMessagePtr &csMsg);
  void onStatusChangeRegister(const CSMessagePtr &msg);
  void onStatusChangeUnregister(const CSMessagePtr &msg);

  RequestPtr saveRequestInfo(const CSMessagePtr &msg);
  RequestPtr pickOutRequestInfo(const CSMessagePtr &msgContent);

  void invalidateAndRemoveAllRequests();

  void saveRegisterInfo(const CSMessagePtr &msg);
  void removeRegisterInfo(const CSMessagePtr &msg);
  void removeAllRegisterInfo();
  void removeRegistersOfAddress(const Address &addr);

  void onAbortActionRequest(const CSMessagePtr &msg);
  void onClientGoesOff(const CSMessagePtr &msg);

  void onActionRequest(const CSMessagePtr &msg);
  void updateLatestStatus(const CSMessagePtr &registerMsg);
  void onStatusGetRequest(const CSMessagePtr &getMsg);
  bool invokeRequestHandlerCallback(const RequestPtr &request);

  bool registerRequestHandler(const OpID &opID,
                              RequestHandlerFunction handlerFunction);
  bool unregisterRequestHandler(const OpID &opID);
};

} // namespace messaging
} // namespace maf
