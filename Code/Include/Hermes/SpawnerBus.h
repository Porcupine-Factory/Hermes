
#pragma once

#include <Hermes/HermesTypeIds.h>

#include <AzCore/EBus/EBus.h>
#include <AzCore/Interface/Interface.h>

namespace Hermes
{
    class SpawnerRequests
    {
    public:
        AZ_RTTI(SpawnerRequests, HermesRequestsTypeId);
        virtual ~SpawnerRequests() = default;
        // Put your public methods here
    };

    class SpawnerBusTraits : public AZ::EBusTraits
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
        //////////////////////////////////////////////////////////////////////////
    };

    using SpawnerRequestBus = AZ::EBus<SpawnerRequests, SpawnerBusTraits>;
    using SpawnerInterface = AZ::Interface<SpawnerRequests>;

} // namespace Hermes
