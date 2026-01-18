#pragma once
#include <Hermes/NetworkSpawnerControllerBus.h>
#include <Hermes/SpawnerComponentBus.h>

#include <AzCore/Component/Component.h>
#include <AzCore/Component/EntityBus.h>
#include <AzCore/Component/TickBus.h>
#include <AzFramework/Spawnable/SpawnableEntitiesInterface.h>

namespace Hermes
{
    class NetworkSpawner;
    class NetworkSpawnerController;

    class SpawnerComponent
        : public AZ::Component
        , public AZ::TickBus::Handler
        , public NetworkSpawnerControllerNotificationBus::Handler
        , public SpawnerComponentRequestBus::Handler
        , public AZ::EntityBus::Handler
    {
        friend class NetworkSpawnerController;

    public:
        AZ_COMPONENT(SpawnerComponent, "{af018f8d-ecf1-4343-8b68-30dd7b4d2c5c}");

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

        // SpawnerComponentRequestBus
        bool SpawnPrefabName(const AZStd::string& prefabName, const AZ::Transform& transform) override;
        bool DepawnPrefabTicketId(const uint32_t& prefabTicketId) override;
        uint32_t GetLastSpawnedTicketId() override;
        void SetLastSpawnedTicketId(const uint32_t& new_lastSpawnedTicketId) override;
        uint32_t GetLastDespawnedTicketId() override;
        void SetLastDespawnedTicketId(const uint32_t& new_lastDespawnedTicketId) override;
        bool GetIsAutonomousClient() const override;
        bool GetIsServer() const override;
        bool GetIsHost() const override;
        bool GetLocallyEnableNetworkSpawner() const override;
        void SetLocallyEnableNetworkSpawner(const bool& new_networkSpawnerEnabled) override;
        void IsAutonomousSoConnect() override;
        void NotAutonomousSoDisconnect() override;

        // SpawnerComponentNotificationBus
        void OnNetworkSpawnerTickStart(const float& deltaTime);
        void OnNetworkSpawnerTickFinish(const float& deltaTime);

    private:
        // TickBus interface
        void OnTick(float deltaTime, AZ::ScriptTimePoint) override;

        // Called on each tick
        void ProcessInput(const float& deltaTime, const AZ::u8& tickTimestepNetwork);

        // NetworkSpawnerControllerNotificationBus
        void OnNetworkTickStart(const float& deltaTime, const bool& server, const AZ::EntityId& entity);
        void OnNetworkTickFinish(const float& deltaTime, const bool& server, const AZ::EntityId& entity);

        // Various methods used to implement the Spawner functionality
        void SetNetworkSpawnerProperties() const;
        void GetNetworkSpawnerProperties();

        // NetworkSpawner object
        NetworkSpawner* m_networkSpawnerObject = nullptr;
        NetworkSpawnerController* m_networkSpawnerControllerObject = nullptr;

        // Stores the previous NetworkSpawner tick deltaTime
        float m_prevDeltaTime = 1.f / 60.f;
        float m_prevNetworkSpawnerDeltaTime = 0.033f;

        // Various data structures
        uint32_t m_lastSpawnedTicketId = 0;
        uint32_t m_lastDespawnedTicketId = 0;

        // Vector of all spawnable paths
        AZStd::vector<AZStd::string> m_spawnablePaths;

        // A set of all spawn tickets, used to despawn at some point
        AZStd::unordered_set<AzFramework::EntitySpawnTicket*> m_spawnTickets;

        // Networking related variables
        bool m_networkSpawnerEnabled = false;
        bool m_isServer = false;
        bool m_isHost = false;
        bool m_isAutonomousClient = false;
    };
} // namespace Hermes
