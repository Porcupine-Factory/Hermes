
#include "HermesSystemComponent.h"

#include <Hermes/HermesTypeIds.h>

#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/SerializeContext.h>
#if __has_include(<Source/AutoGen/AutoComponentTypes.h>)
#include <Source/AutoGen/AutoComponentTypes.h>
#endif

namespace Hermes
{
    AZ_COMPONENT_IMPL(HermesSystemComponent, "HermesSystemComponent", HermesSystemComponentTypeId);

    void HermesSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<HermesSystemComponent, AZ::Component>()->Version(0);

            if (AZ::EditContext* ec = serializeContext->GetEditContext())
            {
                ec->Class<HermesSystemComponent>("Hermes", "[Description of functionality provided by this System Component]")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("System"))
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true);

                ec->Class<HermesSystemComponent>("NetworkHermes", "[Description of functionality provided by this System Component]")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("System"))
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true);
            }
        }
    }

    void HermesSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("HermesService"));
        provided.push_back(AZ_CRC_CE("NetworkHermesService"));
    }

    void HermesSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("HermesService"));
        incompatible.push_back(AZ_CRC_CE("NetworkHermesService"));
    }

    void HermesSystemComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC_CE("NetworkingService"));
        required.push_back(AZ_CRC_CE("MultiplayerService"));
    }

    void HermesSystemComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }

    HermesSystemComponent::HermesSystemComponent()
    {
        if (HermesInterface::Get() == nullptr)
        {
            HermesInterface::Register(this);
        }
    }

    HermesSystemComponent::~HermesSystemComponent()
    {
        if (HermesInterface::Get() == this)
        {
            HermesInterface::Unregister(this);
        }
    }

    void HermesSystemComponent::Init()
    {
    }

    void HermesSystemComponent::Activate()
    {
        HermesRequestBus::Handler::BusConnect();
        NetworkHermesRequestBus::Handler::BusConnect();
        AZ::TickBus::Handler::BusConnect();
        // Register multiplayer components
        RegisterMultiplayerComponents();
    }

    void HermesSystemComponent::Deactivate()
    {
        AZ::TickBus::Handler::BusDisconnect();
        NetworkHermesRequestBus::Handler::BusDisconnect();
        HermesRequestBus::Handler::BusDisconnect();
    }

    void HermesSystemComponent::OnTick([[maybe_unused]] float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
    }

} // namespace Hermes
