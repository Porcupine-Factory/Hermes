#pragma once
#include <Hermes/CharacterNameComponentBus.h>
#include <Hermes/NetworkCharacterNameControllerBus.h>

#include <AzCore/Component/Component.h>
#include <AzCore/Component/EntityBus.h>
#include <AzCore/Component/TickBus.h>

namespace Hermes
{
    class NetworkCharacterName;
    class NetworkCharacterNameController;

    class CharacterNameComponent
        : public AZ::Component
        , public AZ::TickBus::Handler
        , public NetworkCharacterNameControllerNotificationBus::Handler
        , public CharacterNameComponentRequestBus::Handler
        , public AZ::EntityBus::Handler
    {
        friend class NetworkCharacterNameController;

    public:
        AZ_COMPONENT(CharacterNameComponent, "{84f235d8-c480-4ff6-b12a-b88e27590645}");

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

        // CharacterNameComponentRequestBus
        AZStd::string GetName() const override;
        void SetName(const AZStd::string& new_name) override;
        void SetNameOnServerRPC(const AZStd::string& name) override;
        bool GetIsAutonomousClient() const override;
        bool GetIsServer() const override;
        bool GetIsHost() const override;
        bool GetLocallyEnableNetworkCharacterName() const override;
        void SetLocallyEnableNetworkCharacterName(const bool& new_networkCharacterNameEnabled) override;
        void IsAutonomousSoConnect() override;
        void NotAutonomousSoDisconnect() override;

    private:
        // TickBus interface
        void OnTick(float deltaTime, AZ::ScriptTimePoint) override;

        // Called on each tick
        void ProcessInput(const float& deltaTime, const AZ::u8& tickTimestepNetwork);

        // NetworkCharacterNameControllerNotificationBus
        void OnNetworkTickStart(const float& deltaTime, const bool& server, const AZ::EntityId& entityId);
        void OnNetworkTickFinish(const float& deltaTime, const bool& server, const AZ::EntityId& entityId);

        // Various methods used to implement the CharacterName functionality
        void GetNetworkCharacterNameProperties();
        void SetNetworkCharacterNameProperties();

        // NetworkCharacterName object
        NetworkCharacterName* m_networkCharacterNameObject = nullptr;
        NetworkCharacterNameController* m_networkCharacterNameControllerObject = nullptr;

        // Stores the previous NetworkCharacterName tick deltaTime
        float m_prevDeltaTime = 1.f / 60.f;
        float m_prevNetworkCharacterNameDeltaTime = 0.033f;

        // Various data structures
        AZStd::string m_name = "";
        AZStd::string m_nameOnServer = "";

        // Networking related variables
        bool m_networkCharacterNameEnabled = false;
        bool m_isServer = false;
        bool m_isHost = false;
        bool m_isAutonomousClient = false;
    };
} // namespace Hermes
