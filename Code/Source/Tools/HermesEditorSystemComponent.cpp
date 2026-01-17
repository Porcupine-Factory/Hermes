
#include "HermesEditorSystemComponent.h"
#include <AzCore/Serialization/SerializeContext.h>

#include <Hermes/HermesTypeIds.h>

namespace Hermes
{
    AZ_COMPONENT_IMPL(HermesEditorSystemComponent, "HermesEditorSystemComponent", HermesEditorSystemComponentTypeId, BaseSystemComponent);

    void HermesEditorSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<HermesEditorSystemComponent, HermesSystemComponent>()->Version(0);
        }
    }

    HermesEditorSystemComponent::HermesEditorSystemComponent() = default;

    HermesEditorSystemComponent::~HermesEditorSystemComponent() = default;

    void HermesEditorSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        BaseSystemComponent::GetProvidedServices(provided);
        provided.push_back(AZ_CRC_CE("HermesEditorService"));
    }

    void HermesEditorSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        BaseSystemComponent::GetIncompatibleServices(incompatible);
        incompatible.push_back(AZ_CRC_CE("HermesEditorService"));
    }

    void HermesEditorSystemComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        BaseSystemComponent::GetRequiredServices(required);
    }

    void HermesEditorSystemComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        BaseSystemComponent::GetDependentServices(dependent);
    }

    void HermesEditorSystemComponent::Activate()
    {
        HermesSystemComponent::Activate();
        AzToolsFramework::EditorEvents::Bus::Handler::BusConnect();
    }

    void HermesEditorSystemComponent::Deactivate()
    {
        AzToolsFramework::EditorEvents::Bus::Handler::BusDisconnect();
        HermesSystemComponent::Deactivate();
    }

} // namespace Hermes
