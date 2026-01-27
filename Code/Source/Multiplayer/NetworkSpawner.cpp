#include <Multiplayer/NetworkSpawner.h>

namespace Hermes
{
    NetworkSpawnerController::NetworkSpawnerController(NetworkSpawner& parent)
        : NetworkSpawnerControllerBase(parent)
        , m_enableNetworkSpawnerChangedEvent(
              [this](bool enable)
              {
                  OnEnableNetworkSpawnerChanged(enable);
              })
    {
    }

    void NetworkSpawnerController::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        NetworkSpawnerControllerRequestBus::Handler::BusConnect(GetEntityId());

        // Subscribe to EnableNetworkSpawner change events
        EnableNetworkSpawnerAddEvent(m_enableNetworkSpawnerChangedEvent);

        // Get access to the SpawnerComponent object and its members
        const AZ::Entity* entity = GetParent().GetEntity();
        m_spawnerObject = entity->FindComponent<SpawnerComponent>();
        m_spawnerObject->m_networkSpawnerEnabled = GetEnableNetworkSpawner();

        if (IsNetEntityRoleAutonomous())
        {
            m_autonomousNotDetermined = false;
            m_spawnerObject->IsAutonomousSoConnect();
            if (IsNetEntityRoleAuthority())
                m_spawnerObject->m_isHost = true;
            else
                m_spawnerObject->m_isAutonomousClient = true;
        }
    }

    void NetworkSpawnerController::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        m_enableNetworkSpawnerChangedEvent.Disconnect();
        NetworkSpawnerControllerRequestBus::Handler::BusDisconnect();
    }

    void NetworkSpawnerController::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC_CE("SpawnerService"));
    }

    void NetworkSpawnerController::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }

    void NetworkSpawnerController::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("NetworkSpawnerService"));
    }

    void NetworkSpawnerController::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("NetworkSpawnerService"));
    }

    void NetworkSpawnerController::CreateInput([[maybe_unused]] Multiplayer::NetworkInput& input, [[maybe_unused]] float deltaTime)
    {
    }

    void NetworkSpawnerController::ProcessInput([[maybe_unused]] Multiplayer::NetworkInput& input, float deltaTime)
    {
        if (m_disabled)
            return;

        if (m_init)
        {
            m_init = false;
        }

        // Disconnect from various buses when the NetworkSpawnerController is not autonomous, and only do this once
        if (m_autonomousNotDetermined)
        {
            m_spawnerObject->NotAutonomousSoDisconnect();
            if (IsNetEntityRoleAuthority())
                m_spawnerObject->m_isServer = true;
            else
                m_disabled = true;
            m_autonomousNotDetermined = false;
        }

        NetworkSpawnerControllerNotificationBus::Broadcast(
            &NetworkSpawnerControllerNotificationBus::Events::OnNetworkTickStart, deltaTime, m_spawnerObject->m_isServer, GetEntityId());

        // Do work every network tick here

        NetworkSpawnerControllerNotificationBus::Broadcast(
            &NetworkSpawnerControllerNotificationBus::Events::OnNetworkTickFinish, deltaTime, m_spawnerObject->m_isServer, GetEntityId());
    }

    // RPC methods
    void NetworkSpawnerController::HandleSpawnPrefabName(
        [[maybe_unused]] AzNetworking::IConnection* invokingConnection,
        const AZStd::string& prefabName,
        const AZ::Transform& transform,
        const bool& makeChild)
    {
        if (!m_spawnerObject->SpawnPrefabName(prefabName, transform, makeChild))
            AZ_Error("NetworkSpawnerController", false, "Spawning the specified prefab name failed.");
    }

    void NetworkSpawnerController::HandleSpawnPrefabPath(
        [[maybe_unused]] AzNetworking::IConnection* invokingConnection,
        const AZStd::string& prefabName,
        const AZ::Transform& transform,
        const bool& makeChild)
    {
        if (!m_spawnerObject->SpawnPrefabPath(prefabName, transform, makeChild))
            AZ_Error("NetworkSpawnerController", false, "Spawning the specified prefab path failed.");
    }

    void NetworkSpawnerController::HandleDepawnPrefabTicketId(
        [[maybe_unused]] AzNetworking::IConnection* invokingConnection, const uint32_t& prefabTicketId)
    {
        if (!m_spawnerObject->DepawnPrefabTicketId(prefabTicketId))
            AZ_Error("NetworkSpawnerController", false, "Despawning %lu prefab ticketId failed.", prefabTicketId);
    }

    // Event Notification methods for use in scripts
    void NetworkSpawnerController::OnNetworkTickStart(
        [[maybe_unused]] const float& deltaTime, [[maybe_unused]] const bool& server, [[maybe_unused]] const AZ::EntityId& entity)
    {
    }
    void NetworkSpawnerController::OnNetworkTickFinish(
        [[maybe_unused]] const float& deltaTime, [[maybe_unused]] const bool& server, [[maybe_unused]] const AZ::EntityId& entity)
    {
    }

    void NetworkSpawnerController::OnEnableNetworkSpawnerChanged(const bool& enable)
    {
        m_disabled = !enable;
        m_spawnerObject->m_networkSpawnerEnabled = enable;
    }

    // Request Bus getter and setter methods for use in scripts
    bool NetworkSpawnerController::GetEnabled() const
    {
        return !m_disabled;
    }
    void NetworkSpawnerController::SetEnabled(const bool& new_enabled)
    {
        m_disabled = !new_enabled;
        m_spawnerObject->m_networkSpawnerEnabled = new_enabled;
    }
} // namespace Hermes
