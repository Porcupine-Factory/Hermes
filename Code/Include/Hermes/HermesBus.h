
#pragma once

#include <Hermes/HermesTypeIds.h>

#include <AzCore/EBus/EBus.h>
#include <AzCore/Interface/Interface.h>

namespace Hermes
{
    class HermesRequests
    {
    public:
        AZ_RTTI(HermesRequests, HermesRequestsTypeId);
        virtual ~HermesRequests() = default;
        // Put your public methods here
    };

    class HermesBusTraits : public AZ::EBusTraits
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
        //////////////////////////////////////////////////////////////////////////
    };

    using HermesRequestBus = AZ::EBus<HermesRequests, HermesBusTraits>;
    using HermesInterface = AZ::Interface<HermesRequests>;

} // namespace Hermes
