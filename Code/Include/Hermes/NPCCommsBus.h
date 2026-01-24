
#pragma once

#include <Hermes/HermesTypeIds.h>

#include <AzCore/EBus/EBus.h>
#include <AzCore/Interface/Interface.h>

namespace Hermes
{
    class NPCCommsRequests
    {
    public:
        AZ_RTTI(NPCCommsRequests, HermesRequestsTypeId);
        virtual ~NPCCommsRequests() = default;
        // Put your public methods here
    };

    class NPCCommsBusTraits : public AZ::EBusTraits
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
        //////////////////////////////////////////////////////////////////////////
    };

    using NPCCommsRequestBus = AZ::EBus<NPCCommsRequests, NPCCommsBusTraits>;
    using NPCCommsInterface = AZ::Interface<NPCCommsRequests>;

} // namespace Hermes
