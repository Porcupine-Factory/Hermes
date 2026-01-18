
#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>
#include <Hermes/HermesBus.h>
#include <Hermes/NetworkHermesBus.h>
#include <Hermes/NetworkHermesControllerBus.h>
#include <Hermes/NetworkSpawnerBus.h>
#include <Hermes/NetworkSpawnerControllerBus.h>
#include <Hermes/SpawnerBus.h>

namespace Hermes
{
    class HermesSystemComponent
        : public AZ::Component
        , protected HermesRequestBus::Handler
        , protected NetworkHermesRequestBus::Handler
        , protected SpawnerRequestBus::Handler
        , protected NetworkSpawnerRequestBus::Handler
        , public AZ::TickBus::Handler
    {
    public:
        AZ_COMPONENT_DECL(HermesSystemComponent);

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        HermesSystemComponent();
        ~HermesSystemComponent();

    protected:
        ////////////////////////////////////////////////////////////////////////
        // HermesRequestBus interface implementation

        ////////////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////////////////
        // AZ::Component interface implementation
        void Init() override;
        void Activate() override;
        void Deactivate() override;
        ////////////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////////////////
        // AZTickBus interface implementation
        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;
        ////////////////////////////////////////////////////////////////////////
    };

} // namespace Hermes
