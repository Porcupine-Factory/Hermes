#pragma once
#include <Hermes/ExampleComponentBus.h>
#include <Hermes/NetworkExampleControllerBus.h>

#include <AzCore/Component/Component.h>
#include <AzCore/Component/EntityBus.h>
#include <AzCore/Component/TickBus.h>

namespace Hermes
{
    class NetworkExample;
    class NetworkExampleController;

    class ExampleComponent
        : public AZ::Component
        , public AZ::TickBus::Handler
        , public NetworkExampleControllerNotificationBus::Handler
        , public ExampleComponentRequestBus::Handler
        , public AZ::EntityBus::Handler
    {
        friend class NetworkExampleController;

    public:
        AZ_COMPONENT(ExampleComponent, "{componentuuid}");

        static void Reflect(AZ::ReflectContext* rc);

        // AZ::Component interface implementation
        void Activate() override;
        void Deactivate() override;

        // AZ::EntityBus interface
        void OnEntityActivated(const AZ::EntityId& entityId) override;

        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);

        // ExampleComponentRequestBus
        AZ::u32 GetExampTemporaryVar() const override;
        void SetExampTemporaryVar(const AZ::u32& new_exampTemporaryVar) override;
        void SetExampTemporaryVarRPC(const AZ::u32& new_exampTemporaryVar) override;
        bool GetIsAutonomousClient() const override;
        bool GetIsServer() const override;
        bool GetIsHost() const override;
        bool GetLocallyEnableNetworkExample() const override;
        void SetLocallyEnableNetworkExample(const bool& new_networkExampleEnabled) override;
        void IsAutonomousSoConnect() override;
        void NotAutonomousSoDisconnect() override;

        // ExampleComponentNotificationBus
        void OnNetworkExampleTickStart(const float& deltaTime);
        void OnNetworkExampleTickFinish(const float& deltaTime);

    private:
        // TickBus interface
        void OnTick(float deltaTime, AZ::ScriptTimePoint) override;

        // Called on each tick
        void ProcessInput(const float& deltaTime, const AZ::u8& tickTimestepNetwork);

        // NetworkExampleControllerNotificationBus
        void OnNetworkTickStart(const float& deltaTime, const bool& server, const AZ::EntityId& entity);
        void OnNetworkTickFinish(const float& deltaTime, const bool& server, const AZ::EntityId& entity);

        // Various methods used to implement the Example functionality
        void SetNetworkExampleProperties() const;
        void GetNetworkExampleProperties();

        // NetworkExample object
        NetworkExample* m_networkExampleObject = nullptr;
        NetworkExampleController* m_networkExampleControllerObject = nullptr;

        // Stores the previous NetworkExample tick deltaTime
        float m_prevDeltaTime = 1.f / 60.f;
        float m_prevNetworkExampleDeltaTime = 0.033f;

        // Various data structures
        AZ::u32 m_exampTemporaryVar = 0;

        // Networking related variables
        bool m_networkExampleEnabled = false;
        bool m_isServer = false;
        bool m_isHost = false;
        bool m_isAutonomousClient = false;
    };
} // namespace Hermes
