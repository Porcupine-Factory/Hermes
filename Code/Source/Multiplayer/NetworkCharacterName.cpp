#include <Multiplayer/NetworkCharacterName.h>

namespace Hermes
{
    NetworkCharacterNameController::NetworkCharacterNameController(NetworkCharacterName& parent)
        : NetworkCharacterNameControllerBase(parent)
        , m_enableNetworkCharacterNameChangedEvent(
              [this](bool enable)
              {
                  OnEnableNetworkCharacterNameChanged(enable);
              })
    {
    }

    void NetworkCharacterNameController::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        NetworkCharacterNameControllerRequestBus::Handler::BusConnect(GetEntityId());

        // Subscribe to EnableNetworkCharacterName change events
        EnableNetworkCharacterNameAddEvent(m_enableNetworkCharacterNameChangedEvent);

        // Get access to the CharacterNameComponent object and its members
        const AZ::Entity* entity = GetParent().GetEntity();
        m_characterNameObject = entity->FindComponent<CharacterNameComponent>();
        m_characterNameObject->m_networkCharacterNameEnabled = GetEnableNetworkCharacterName();

        if (IsNetEntityRoleAutonomous())
        {
            m_autonomousNotDetermined = false;
            m_characterNameObject->IsAutonomousSoConnect();
            if (IsNetEntityRoleAuthority())
                m_characterNameObject->m_isHost = true;
            else
                m_characterNameObject->m_isAutonomousClient = true;
        }
    }

    void NetworkCharacterNameController::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        m_enableNetworkCharacterNameChangedEvent.Disconnect();
        NetworkCharacterNameControllerRequestBus::Handler::BusDisconnect();
    }

    void NetworkCharacterNameController::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC_CE("CharacterNameService"));
    }

    void NetworkCharacterNameController::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }

    void NetworkCharacterNameController::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("NetworkCharacterNameService"));
    }

    void NetworkCharacterNameController::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("NetworkCharacterNameService"));
    }

    void NetworkCharacterNameController::CreateInput([[maybe_unused]] Multiplayer::NetworkInput& input, [[maybe_unused]] float deltaTime)
    {
    }

    void NetworkCharacterNameController::ProcessInput([[maybe_unused]] Multiplayer::NetworkInput& input, float deltaTime)
    {
        if (m_disabled)
            return;

        if (m_init)
        {
            m_init = false;
            SetName(m_characterNameObject->m_name);
        }

        // Disconnect from various buses when the NetworkCharacterNameController is not autonomous, and only do this once
        if (m_autonomousNotDetermined)
        {
            m_characterNameObject->NotAutonomousSoDisconnect();
            if (IsNetEntityRoleAuthority())
                m_characterNameObject->m_isServer = true;
            else
                m_disabled = true;
            m_autonomousNotDetermined = false;
        }

        NetworkCharacterNameControllerNotificationBus::Broadcast(
            &NetworkCharacterNameControllerNotificationBus::Events::OnNetworkTickStart,
            deltaTime,
            m_characterNameObject->m_isServer,
            GetEntityId());

        // Do work every network tick here

        NetworkCharacterNameControllerNotificationBus::Broadcast(
            &NetworkCharacterNameControllerNotificationBus::Events::OnNetworkTickFinish,
            deltaTime,
            m_characterNameObject->m_isServer,
            GetEntityId());
    }

#if AZ_TRAIT_SERVER
    void NetworkCharacterNameController::HandleSendNameOnServerRPC(
        [[maybe_unused]] AzNetworking::IConnection* invokingConnection, const AZStd::string& name)
    {
        m_characterNameObject->SetNameOnServerRPC(name);
    }
#endif

    // Event Notification methods for use in scripts
    void NetworkCharacterNameController::OnNetworkTickStart(
        [[maybe_unused]] const float deltaTime, [[maybe_unused]] const bool server, [[maybe_unused]] const AZ::EntityId& entityId)
    {
    }
    void NetworkCharacterNameController::OnNetworkTickFinish(
        [[maybe_unused]] const float deltaTime, [[maybe_unused]] const bool server, [[maybe_unused]] const AZ::EntityId& entityId)
    {
    }

    void NetworkCharacterNameController::OnEnableNetworkCharacterNameChanged(const bool enable)
    {
        m_disabled = !enable;
        m_characterNameObject->m_networkCharacterNameEnabled = enable;
    }

    // Request Bus getter and setter methods for use in scripts
    bool NetworkCharacterNameController::GetEnabled() const
    {
        return !m_disabled;
    }
    void NetworkCharacterNameController::SetEnabled(const bool new_enabled)
    {
        m_disabled = !new_enabled;
        m_characterNameObject->m_networkCharacterNameEnabled = new_enabled;
    }
} // namespace Hermes
