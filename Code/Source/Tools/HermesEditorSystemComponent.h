
#pragma once

#include <AzToolsFramework/API/ToolsApplicationAPI.h>

#include <Clients/HermesSystemComponent.h>

namespace Hermes
{
    /// System component for Hermes editor
    class HermesEditorSystemComponent
        : public HermesSystemComponent
        , protected AzToolsFramework::EditorEvents::Bus::Handler
    {
        using BaseSystemComponent = HermesSystemComponent;

    public:
        AZ_COMPONENT_DECL(HermesEditorSystemComponent);

        static void Reflect(AZ::ReflectContext* context);

        HermesEditorSystemComponent();
        ~HermesEditorSystemComponent();

    private:
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        // AZ::Component
        void Activate() override;
        void Deactivate() override;
    };
} // namespace Hermes
