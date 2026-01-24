#pragma once
#include <Hermes/NPCCommsComponentBus.h>
#include <Hermes/NetworkNPCCommsControllerBus.h>

#include <AzCore/Component/Component.h>
#include <AzCore/Component/EntityBus.h>
#include <AzCore/Component/TickBus.h>

namespace Hermes
{
    class NetworkNPCComms;
    class NetworkNPCCommsController;

    class NPCCommsComponent
        : public AZ::Component
        , public AZ::TickBus::Handler
        , public NetworkNPCCommsControllerNotificationBus::Handler
        , public NPCCommsComponentRequestBus::Handler
        , public AZ::EntityBus::Handler
    {
        friend class NetworkNPCCommsController;

    public:
        AZ_COMPONENT(NPCCommsComponent, "{c1d98158-d5c6-466f-80f6-5f3956759327}");

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

        // NPCCommsComponentRequestBus
        bool GetAggroed() const override;
        void SetAggroed(const bool& new_aggroed) override;
        void SetAggroedRPC(const bool& new_aggroedRPC) override;
        bool GetIsAutonomousClient() const override;
        bool GetIsServer() const override;
        bool GetIsHost() const override;
        bool GetLocallyEnableNetworkNPCComms() const override;
        void SetLocallyEnableNetworkNPCComms(const bool& new_networkNPCCommsEnabled) override;
        void IsAutonomousSoConnect() override;
        void NotAutonomousSoDisconnect() override;

        // NPCCommsComponentNotificationBus
        void OnNetworkNPCCommsTickStart(const float& deltaTime);
        void OnNetworkNPCCommsTickFinish(const float& deltaTime);

    private:
        // TickBus interface
        void OnTick(float deltaTime, AZ::ScriptTimePoint) override;

        // Called on each tick
        void ProcessInput(const float& deltaTime, const AZ::u8& tickTimestepNetwork);

        // NetworkNPCCommsControllerNotificationBus
        void OnNetworkTickStart(const float& deltaTime, const bool& server, const AZ::EntityId& entity);
        void OnNetworkTickFinish(const float& deltaTime, const bool& server, const AZ::EntityId& entity);

        // Various methods used to implement the NPCComms functionality
        void SetNetworkNPCCommsProperties() const;
        void GetNetworkNPCCommsProperties();

        // NetworkNPCComms object
        NetworkNPCComms* m_networkNPCCommsObject = nullptr;
        NetworkNPCCommsController* m_networkNPCCommsControllerObject = nullptr;

        // Stores the previous NetworkNPCComms tick deltaTime
        float m_prevDeltaTime = 1.f / 60.f;
        float m_prevNetworkNPCCommsDeltaTime = 0.033f;

        // Various data structures
        bool m_aggroed = false;

        // Networking related variables
        bool m_networkNPCCommsEnabled = false;
        bool m_isServer = false;
        bool m_isHost = false;
        bool m_isAutonomousClient = false;
    };
} // namespace Hermes
