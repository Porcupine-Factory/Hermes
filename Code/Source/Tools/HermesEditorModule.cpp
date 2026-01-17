
#include "HermesEditorSystemComponent.h"
#include <Hermes/HermesTypeIds.h>
#include <HermesModuleInterface.h>

namespace Hermes
{
    class HermesEditorModule : public HermesModuleInterface
    {
    public:
        AZ_RTTI(HermesEditorModule, HermesEditorModuleTypeId, HermesModuleInterface);
        AZ_CLASS_ALLOCATOR(HermesEditorModule, AZ::SystemAllocator);

        HermesEditorModule()
        {
            // Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
            // Add ALL components descriptors associated with this gem to m_descriptors.
            // This will associate the AzTypeInfo information for the components with the the SerializeContext, BehaviorContext and
            // EditContext. This happens through the [MyComponent]::Reflect() function.
            m_descriptors.insert(
                m_descriptors.end(),
                {
                    HermesEditorSystemComponent::CreateDescriptor(),
                });
        }

        /**
         * Add required SystemComponents to the SystemEntity.
         * Non-SystemComponents should not be added here
         */
        AZ::ComponentTypeList GetRequiredSystemComponents() const override
        {
            return AZ::ComponentTypeList{
                azrtti_typeid<HermesEditorSystemComponent>(),
            };
        }
    };
} // namespace Hermes

#if defined(O3DE_GEM_NAME)
AZ_DECLARE_MODULE_CLASS(AZ_JOIN(Gem_, O3DE_GEM_NAME, _Editor), Hermes::HermesEditorModule)
#else
AZ_DECLARE_MODULE_CLASS(Gem_Hermes_Editor, Hermes::HermesEditorModule)
#endif
