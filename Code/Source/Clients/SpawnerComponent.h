#pragma once
#include <Hermes/NetworkSpawnerControllerBus.h>
#include <Hermes/SpawnerComponentBus.h>
#include <Multiplayer/NetworkSpawner.h>

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
        bool SpawnPrefabName(const AZStd::string& prefabName, const AZ::Transform& transform, const bool makeChild = false) override;
        bool SpawnPrefabPath(const AZStd::string& prefabPath, const AZ::Transform& transform, const bool makeChild = false) override;
        bool SpawnNetPrefabName(const AZStd::string& prefabName, const AZ::Transform& transform) override;
        bool SpawnNetPrefabPath(const AZStd::string& prefabPath, const AZ::Transform& transform) override;
        bool DepawnPrefabTicketId(const uint32_t prefabTicketId) override;
        bool DepawnNetEntityIdAndChildren(const AZStd::string& netEntityId) override;
        AZStd::pair<uint32_t, AZStd::string> GetLastSpawnedTicketIdByStringNetEntityId() override;
        void SetLastSpawnedTicketIdByStringNetEntityId(
            const AZStd::pair<uint32_t, AZStd::string>& new_lastSpawnedTicketIdByStringNetEntityId) override;
        uint32_t GetLastSpawnedTicketId() override;
        void SetLastSpawnedTicketId(const uint32_t new_lastSpawnedTicketId) override;
        AZStd::vector<AZStd::string> GetLastSpawnedStringNetEntityIds() override;
        void SetLastSpawnedStringNetEntityIds(const AZStd::vector<AZStd::string>& new_lastSpawnedStringNetEntityIds) override;
        uint32_t GetLastDespawnedTicketId() override;
        void SetLastDespawnedTicketId(const uint32_t new_lastDespawnedTicketId) override;
        bool GetIsAutonomousClient() const override;
        bool GetIsServer() const override;
        bool GetIsHost() const override;
        bool GetLocallyEnableNetworkSpawner() const override;
        void SetLocallyEnableNetworkSpawner(const bool new_networkSpawnerEnabled) override;
        void IsAutonomousSoConnect() override;
        void NotAutonomousSoDisconnect() override;

    private:
        // TickBus interface
        void OnTick(float deltaTime, AZ::ScriptTimePoint) override;

        // Called on each tick
        void ProcessInput(const float deltaTime, const AZ::u8& tickTimestepNetwork);

        // NetworkSpawnerControllerNotificationBus
        void OnNetworkTickStart(const float deltaTime, const bool server, const AZ::EntityId& entityId);
        void OnNetworkTickFinish(const float deltaTime, const bool server, const AZ::EntityId& entityId);

        // Various methods used to implement the Spawner functionality
        void GetNetworkSpawnerProperties();
        void SetNetworkSpawnerProperties() const;

        // NetworkSpawner object
        NetworkSpawner* m_networkSpawnerObject = nullptr;
        NetworkSpawnerController* m_networkSpawnerControllerObject = nullptr;

        // Various data structures
        uint32_t m_lastSpawnedTicketId = 0;
        AZStd::pair<uint32_t, AZStd::string> m_lastSpawnedTicketIdByStringNetEntityId = AZStd::pair<uint32_t, AZStd::string>();
        AZStd::vector<AZStd::string> m_lastSpawnedStringNetEntityIds = AZStd::vector<AZStd::string>();
        uint32_t m_lastDespawnedTicketId = 0;

        // Vector of all spawnable paths
        AZStd::vector<AZStd::string> m_spawnablePaths;

        // A set of all spawn tickets, used to despawn at some point
        AZStd::unordered_set<AzFramework::EntitySpawnTicket*> m_spawnTickets;

        // All of the spawned NetEntityHandles
        AZStd::vector<Multiplayer::NetworkEntityHandle> m_netEntityHandles;

        // Networking related variables
        bool m_networkSpawnerEnabled = false;
        bool m_isServer = false;
        bool m_isHost = false;
        bool m_isAutonomousClient = false;
    };
} // namespace Hermes
