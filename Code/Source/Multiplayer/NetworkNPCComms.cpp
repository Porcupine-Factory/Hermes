#include <Multiplayer/NetworkNPCComms.h>

namespace Hermes
{
    NetworkNPCCommsController::NetworkNPCCommsController(NetworkNPCComms& parent)
        : NetworkNPCCommsControllerBase(parent)
        , m_enableNetworkNPCCommsChangedEvent(
              [this](bool enable)
              {
                  OnEnableNetworkNPCCommsChanged(enable);
              })
    {
    }

    void NetworkNPCCommsController::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        NetworkNPCCommsControllerRequestBus::Handler::BusConnect(GetEntityId());

        // Subscribe to EnableNetworkNPCComms change events
        EnableNetworkNPCCommsAddEvent(m_enableNetworkNPCCommsChangedEvent);

        // Get access to the NPCCommsComponent object and its members
        const AZ::Entity* entity = GetParent().GetEntity();
        m_nPCCommsObject = entity->FindComponent<NPCCommsComponent>();
        m_nPCCommsObject->m_networkNPCCommsEnabled = GetEnableNetworkNPCComms();

        if (IsNetEntityRoleAutonomous())
        {
            m_autonomousNotDetermined = false;
            m_nPCCommsObject->IsAutonomousSoConnect();
            if (IsNetEntityRoleAuthority())
                m_nPCCommsObject->m_isHost = true;
            else
                m_nPCCommsObject->m_isAutonomousClient = true;
        }
    }

    void NetworkNPCCommsController::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        m_enableNetworkNPCCommsChangedEvent.Disconnect();
        NetworkNPCCommsControllerRequestBus::Handler::BusDisconnect();
    }

    void NetworkNPCCommsController::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC_CE("NPCCommsService"));
    }

    void NetworkNPCCommsController::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }

    void NetworkNPCCommsController::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("NetworkNPCCommsService"));
    }

    void NetworkNPCCommsController::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("NetworkNPCCommsService"));
    }

    void NetworkNPCCommsController::CreateInput([[maybe_unused]] Multiplayer::NetworkInput& input, [[maybe_unused]] float deltaTime)
    {
    }

    void NetworkNPCCommsController::ProcessInput([[maybe_unused]] Multiplayer::NetworkInput& input, float deltaTime)
    {
        if (m_disabled)
            return;

        if (m_init)
        {
            m_init = false;
            SetAggroed(m_nPCCommsObject->m_aggroed);
        }

        // Disconnect from various buses when the NetworkNPCCommsController is not autonomous, and only do this once
        if (m_autonomousNotDetermined)
        {
            m_nPCCommsObject->NotAutonomousSoDisconnect();
            if (IsNetEntityRoleAuthority())
                m_nPCCommsObject->m_isServer = true;
            else
                m_disabled = true;
            m_autonomousNotDetermined = false;
        }

        NetworkNPCCommsControllerNotificationBus::Broadcast(
            &NetworkNPCCommsControllerNotificationBus::Events::OnNetworkTickStart, deltaTime, m_nPCCommsObject->m_isServer, GetEntityId());

        // Do work every network tick here

        NetworkNPCCommsControllerNotificationBus::Broadcast(
            &NetworkNPCCommsControllerNotificationBus::Events::OnNetworkTickFinish, deltaTime, m_nPCCommsObject->m_isServer, GetEntityId());
    }

#if AZ_TRAIT_SERVER
    void NetworkNPCCommsController::HandleSendAggroedRPC(
        [[maybe_unused]] AzNetworking::IConnection* invokingConnection, const bool& aggroed)
    {
        m_nPCCommsObject->SetAggroedRPC(aggroed);
    }
#endif

    // Event Notification methods for use in scripts
    void NetworkNPCCommsController::OnNetworkTickStart(
        [[maybe_unused]] const float& deltaTime, [[maybe_unused]] const bool& server, [[maybe_unused]] const AZ::EntityId& entityId)
    {
    }
    void NetworkNPCCommsController::OnNetworkTickFinish(
        [[maybe_unused]] const float& deltaTime, [[maybe_unused]] const bool& server, [[maybe_unused]] const AZ::EntityId& entityId)
    {
    }

    void NetworkNPCCommsController::OnEnableNetworkNPCCommsChanged(const bool& enable)
    {
        m_disabled = !enable;
        m_nPCCommsObject->m_networkNPCCommsEnabled = enable;
    }

    // Request Bus getter and setter methods for use in scripts
    bool NetworkNPCCommsController::GetEnabled() const
    {
        return !m_disabled;
    }
    void NetworkNPCCommsController::SetEnabled(const bool& new_enabled)
    {
        m_disabled = !new_enabled;
        m_nPCCommsObject->m_networkNPCCommsEnabled = new_enabled;
    }
} // namespace Hermes
