#pragma once

#include <AzCore/EBus/EBus.h>
#include <AzCore/Interface/Interface.h>

namespace Hermes
{
    class NetworkExampleRequests
    {
    public:
        AZ_RTTI(NetworkExampleRequests, "{networkbusuuid}");
        virtual ~NetworkExampleRequests() = default;
        // Put your public methods here
    };

    class NetworkExampleBusTraits : public AZ::EBusTraits
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
        //////////////////////////////////////////////////////////////////////////
    };

    using NetworkExampleRequestBus = AZ::EBus<NetworkExampleRequests, NetworkExampleBusTraits>;
    using NetworkExampleInterface = AZ::Interface<NetworkExampleRequests>;

} // namespace Hermes
