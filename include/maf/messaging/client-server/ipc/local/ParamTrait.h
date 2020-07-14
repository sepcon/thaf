#pragma once

#include "IncomingPayload.h"
#include "OutgoingPayload.h"
#include <maf/logging/Logger.h>
#include <maf/messaging/client-server/ParamTraitBase.h>
#include <maf/messaging/client-server/ParamTranslatingStatus.h>
#include <maf/utils/Pointers.h>
#include <maf/utils/serialization/Serializer.h>

namespace maf {
namespace messaging {
namespace ipc {
namespace local {

using util::assign_ptr;
class ParamTrait : public ParamTraitBase {
public:
  template <class Message>
  static std::shared_ptr<Message>
  translate(const CSPayloadIFPtr &payload,
            TranslationStatus *status = nullptr) {

    if (!payload) {
      assign_ptr(status, TranslationStatus::NoSource);
      return {};
    }

    try {
      using PureContentType = std::decay_t<Message>;
      // Warning: avoid using dynamic_cast RTTI
      // We asume that the implementer will send/receive the
      // CSPayloadIFPtr as std::shared_ptr of IncomingMsgContent
      auto incomingPayload = static_cast<IncomingPayload *>(payload.get());

      std::shared_ptr<PureContentType> content;
      if (const auto &ibytestream = incomingPayload->stream()) {

        content.reset(new PureContentType);
        auto ds = srz::DSR{*ibytestream};
        ds >> *content;

        if (!ibytestream->fail()) {
          assign_ptr(status, TranslationStatus::Success);
        } else {
          assign_ptr(status, TranslationStatus::SourceCorrupted);
        }
      } else {
        assign_ptr(status, TranslationStatus::NoSource);
      }
      return content;

    } catch (const std::exception &e) {
      assign_ptr(status, TranslationStatus::DestSrcMismatch);
      MAF_LOGGER_ERROR("Could not translate message, exception details: ",
                       e.what());
    }
    return nullptr;
  }

  template <class Message>
  static CSPayloadIFPtr translate(const std::shared_ptr<Message> &content) {
    return std::make_shared<OutgoingPayloadT<Message>>(content);
  }
};

} // namespace local
} // namespace ipc
} // namespace messaging
} // namespace maf
