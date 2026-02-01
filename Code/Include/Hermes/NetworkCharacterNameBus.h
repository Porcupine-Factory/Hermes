#pragma once

#include <AzCore/EBus/EBus.h>
#include <AzCore/Interface/Interface.h>

namespace Hermes
{
    class NetworkCharacterNameRequests
    {
    public:
        AZ_RTTI(NetworkCharacterNameRequests, "{7103E090-3AC2-4E67-BDAF-3296CAD1D8E2}");
        virtual ~NetworkCharacterNameRequests() = default;
        // Put your public methods here
    };

    class NetworkCharacterNameBusTraits : public AZ::EBusTraits
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
        //////////////////////////////////////////////////////////////////////////
    };

    using NetworkCharacterNameRequestBus = AZ::EBus<NetworkCharacterNameRequests, NetworkCharacterNameBusTraits>;
    using NetworkCharacterNameInterface = AZ::Interface<NetworkCharacterNameRequests>;

} // namespace Hermes
