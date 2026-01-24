
#include "HermesModuleInterface.h"
#include <AzCore/Memory/Memory.h>

#include <Hermes/HermesTypeIds.h>

#include <Clients/HermesSystemComponent.h>

namespace Hermes
{
    AZ_TYPE_INFO_WITH_NAME_IMPL(HermesModuleInterface, "HermesModuleInterface", HermesModuleInterfaceTypeId);
    AZ_RTTI_NO_TYPE_INFO_IMPL(HermesModuleInterface, AZ::Module);
    AZ_CLASS_ALLOCATOR_IMPL(HermesModuleInterface, AZ::SystemAllocator);

    HermesModuleInterface::HermesModuleInterface()
    {
        // Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
        // Add ALL components descriptors associated with this gem to m_descriptors.
        // This will associate the AzTypeInfo information for the components with the the SerializeContext, BehaviorContext and EditContext.
        // This happens through the [MyComponent]::Reflect() function.
        m_descriptors.insert(
            m_descriptors.end(),
            {
                HermesSystemComponent::CreateDescriptor(),
                HermesComponent::CreateDescriptor(),
                SpawnerComponent::CreateDescriptor(),
                NPCCommsComponent::CreateDescriptor(),
            });

        //< Register multiplayer components
        CreateComponentDescriptors(m_descriptors);
    }

    AZ::ComponentTypeList HermesModuleInterface::GetRequiredSystemComponents() const
    {
        return AZ::ComponentTypeList{
            azrtti_typeid<HermesSystemComponent>(),
        };
    }
} // namespace Hermes
