
#pragma once

#include <Hermes/HermesTypeIds.h>

#include <AzCore/EBus/EBus.h>
#include <AzCore/Interface/Interface.h>

namespace Hermes
{
    class CharacterNameRequests
    {
    public:
        AZ_RTTI(CharacterNameRequests, HermesRequestsTypeId);
        virtual ~CharacterNameRequests() = default;
        // Put your public methods here
    };

    class CharacterNameBusTraits : public AZ::EBusTraits
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
        //////////////////////////////////////////////////////////////////////////
    };

    using CharacterNameRequestBus = AZ::EBus<CharacterNameRequests, CharacterNameBusTraits>;
    using CharacterNameInterface = AZ::Interface<CharacterNameRequests>;

} // namespace Hermes
