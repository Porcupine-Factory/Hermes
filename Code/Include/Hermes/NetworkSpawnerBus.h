#pragma once

#include <AzCore/EBus/EBus.h>
#include <AzCore/Interface/Interface.h>

namespace Hermes
{
    class NetworkSpawnerRequests
    {
    public:
        AZ_RTTI(NetworkSpawnerRequests, "{48D0CB31-593D-4B75-B611-82647F5E5CC0}");
        virtual ~NetworkSpawnerRequests() = default;
        // Put your public methods here
    };

    class NetworkSpawnerBusTraits : public AZ::EBusTraits
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
        //////////////////////////////////////////////////////////////////////////
    };

    using NetworkSpawnerRequestBus = AZ::EBus<NetworkSpawnerRequests, NetworkSpawnerBusTraits>;
    using NetworkSpawnerInterface = AZ::Interface<NetworkSpawnerRequests>;

} // namespace Hermes
