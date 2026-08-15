#include <Multiplayer/NetworkHermes.h>

namespace Hermes
{
    NetworkHermesController::NetworkHermesController(NetworkHermes& parent)
        : NetworkHermesControllerBase(parent)
        , m_enableNetworkHermesChangedEvent(
              [this](bool enable)
              {
                  OnEnableNetworkHermesChanged(enable);
              })
    {
    }

    void NetworkHermesController::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        NetworkHermesControllerRequestBus::Handler::BusConnect(GetEntityId());

        // Subscribe to EnableNetworkHermes change events
        EnableNetworkHermesAddEvent(m_enableNetworkHermesChangedEvent);

        // Get access to the HermesComponent object and its members
        const AZ::Entity* entity = GetParent().GetEntity();
        m_hermesObject = entity->FindComponent<HermesComponent>();
        m_hermesObject->m_networkHermesEnabled = GetEnableNetworkHermes();

        if (IsNetEntityRoleAutonomous())
        {
            m_autonomousNotDetermined = false;
            m_hermesObject->IsAutonomousSoConnect();
            if (IsNetEntityRoleAuthority())
                m_hermesObject->m_isHost = true;
            else
                m_hermesObject->m_isAutonomousClient = true;
        }
    }

    void NetworkHermesController::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        m_enableNetworkHermesChangedEvent.Disconnect();
        NetworkHermesControllerRequestBus::Handler::BusDisconnect();
    }

    void NetworkHermesController::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC_CE("HermesService"));
    }

    void NetworkHermesController::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }

    void NetworkHermesController::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("NetworkHermesService"));
    }

    void NetworkHermesController::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("NetworkHermesService"));
    }

    void NetworkHermesController::CreateInput([[maybe_unused]] Multiplayer::NetworkInput& input, [[maybe_unused]] float deltaTime)
    {
    }

    void NetworkHermesController::ProcessInput([[maybe_unused]] Multiplayer::NetworkInput& input, float deltaTime)
    {
        if (m_disabled)
            return;

        if (m_init)
        {
            m_init = false;
            SetNumber(m_hermesObject->m_number);
        }

        // Disconnect from various buses when the NetworkHermesController is not autonomous, and only do this once
        if (m_autonomousNotDetermined)
        {
            m_hermesObject->NotAutonomousSoDisconnect();
            if (IsNetEntityRoleAuthority())
                m_hermesObject->m_isServer = true;
            else
                m_disabled = true;
            m_autonomousNotDetermined = false;
        }

        NetworkHermesControllerNotificationBus::Broadcast(
            &NetworkHermesControllerNotificationBus::Events::OnNetworkTickStart, deltaTime, m_hermesObject->m_isServer, GetEntityId());

        // Do work every network tick here

        NetworkHermesControllerNotificationBus::Broadcast(
            &NetworkHermesControllerNotificationBus::Events::OnNetworkTickFinish, deltaTime, m_hermesObject->m_isServer, GetEntityId());
    }

#if AZ_TRAIT_SERVER
    void NetworkHermesController::HandleSendNumberDelta(
        [[maybe_unused]] AzNetworking::IConnection* invokingConnection, const AZ::s64& numberDelta)
    {
        m_hermesObject->SetNumberDelta(numberDelta);
    }
#endif

    // Event Notification methods for use in scripts
    void NetworkHermesController::OnNetworkTickStart(
        [[maybe_unused]] const float deltaTime, [[maybe_unused]] const bool server, [[maybe_unused]] const AZ::EntityId& entityId)
    {
    }
    void NetworkHermesController::OnNetworkTickFinish(
        [[maybe_unused]] const float deltaTime, [[maybe_unused]] const bool server, [[maybe_unused]] const AZ::EntityId& entityId)
    {
    }

    void NetworkHermesController::OnEnableNetworkHermesChanged(const bool enable)
    {
        m_disabled = !enable;
        m_hermesObject->m_networkHermesEnabled = enable;
    }

    // Request Bus getter and setter methods for use in scripts
    bool NetworkHermesController::GetEnabled() const
    {
        return !m_disabled;
    }
    void NetworkHermesController::SetEnabled(const bool new_enabled)
    {
        m_disabled = !new_enabled;
        m_hermesObject->m_networkHermesEnabled = new_enabled;
    }
} // namespace Hermes
