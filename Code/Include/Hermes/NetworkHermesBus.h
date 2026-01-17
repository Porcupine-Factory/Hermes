#pragma once

#include <AzCore/EBus/EBus.h>
#include <AzCore/Interface/Interface.h>

namespace Hermes
{
    class NetworkHermesRequests
    {
    public:
        AZ_RTTI(NetworkHermesRequests, "{04ED2475-4F84-4C7D-8E92-ABF2E3EC7694}");
        virtual ~NetworkHermesRequests() = default;
        // Put your public methods here
    };

    class NetworkHermesBusTraits : public AZ::EBusTraits
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
        //////////////////////////////////////////////////////////////////////////
    };

    using NetworkHermesRequestBus = AZ::EBus<NetworkHermesRequests, NetworkHermesBusTraits>;
    using NetworkHermesInterface = AZ::Interface<NetworkHermesRequests>;

} // namespace Hermes
