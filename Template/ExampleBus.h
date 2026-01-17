
#pragma once

#include <Hermes/HermesTypeIds.h>

#include <AzCore/EBus/EBus.h>
#include <AzCore/Interface/Interface.h>

namespace Hermes
{
    class ExampleRequests
    {
    public:
        AZ_RTTI(ExampleRequests, HermesRequestsTypeId);
        virtual ~ExampleRequests() = default;
        // Put your public methods here
    };

    class ExampleBusTraits : public AZ::EBusTraits
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
        //////////////////////////////////////////////////////////////////////////
    };

    using ExampleRequestBus = AZ::EBus<ExampleRequests, ExampleBusTraits>;
    using ExampleInterface = AZ::Interface<ExampleRequests>;

} // namespace Hermes
