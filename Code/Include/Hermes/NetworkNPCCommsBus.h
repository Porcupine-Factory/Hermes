#pragma once

#include <AzCore/EBus/EBus.h>
#include <AzCore/Interface/Interface.h>

namespace Hermes
{
    class NetworkNPCCommsRequests
    {
    public:
        AZ_RTTI(NetworkNPCCommsRequests, "{BC57AE97-5E4D-4025-8811-7BBDEC7C1048}");
        virtual ~NetworkNPCCommsRequests() = default;
        // Put your public methods here
    };

    class NetworkNPCCommsBusTraits : public AZ::EBusTraits
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
        //////////////////////////////////////////////////////////////////////////
    };

    using NetworkNPCCommsRequestBus = AZ::EBus<NetworkNPCCommsRequests, NetworkNPCCommsBusTraits>;
    using NetworkNPCCommsInterface = AZ::Interface<NetworkNPCCommsRequests>;

} // namespace Hermes
