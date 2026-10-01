#include <Multiplayer/NetworkExample.h>

namespace Hermes
{
    NetworkExampleController::NetworkExampleController(NetworkExample& parent)
        : NetworkExampleControllerBase(parent)
        , m_enableNetworkExampleChangedEvent(
              [this](bool enable)
              {
                  OnEnableNetworkExampleChanged(enable);
              })
    {
    }

    void NetworkExampleController::OnActivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        NetworkExampleControllerRequestBus::Handler::BusConnect(GetEntityId());

        // Subscribe to EnableNetworkExample change events
        EnableNetworkExampleAddEvent(m_enableNetworkExampleChangedEvent);

        // Get access to the ExampleComponent object and its members
        const AZ::Entity* entity = GetParent().GetEntity();
        m_exampleObject = entity->FindComponent<ExampleComponent>();
        m_exampleObject->m_networkExampleEnabled = GetEnableNetworkExample();

        if (IsNetEntityRoleAutonomous())
        {
            m_autonomousNotDetermined = false;
            m_exampleObject->IsAutonomousSoConnect();
            if (IsNetEntityRoleAuthority())
                m_exampleObject->m_isHost = true;
            else
                m_exampleObject->m_isAutonomousClient = true;
        }
    }

    void NetworkExampleController::OnDeactivate([[maybe_unused]] Multiplayer::EntityIsMigrating entityIsMigrating)
    {
        m_enableNetworkExampleChangedEvent.Disconnect();
        NetworkExampleControllerRequestBus::Handler::BusDisconnect();
    }

    void NetworkExampleController::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC_CE("ExampleService"));
    }

    void NetworkExampleController::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }

    void NetworkExampleController::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("NetworkExampleService"));
    }

    void NetworkExampleController::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("NetworkExampleService"));
    }

    void NetworkExampleController::CreateInput([[maybe_unused]] Multiplayer::NetworkInput& input, [[maybe_unused]] float deltaTime)
    {
    }

    void NetworkExampleController::ProcessInput([[maybe_unused]] Multiplayer::NetworkInput& input, float deltaTime)
    {
        if (m_disabled)
            return;

        // Disconnect from various buses when the NetworkExampleController is not autonomous, and only do this once
        if (m_autonomousNotDetermined)
        {
            m_exampleObject->NotAutonomousSoDisconnect();
            if (IsNetEntityRoleAuthority())
                m_exampleObject->m_isServer = true;
            else
                m_disabled = true;
            m_autonomousNotDetermined = false;
        }

        if (m_init && (m_exampleObject->m_isServer || m_exampleObject->m_isHost))
        {
            m_init = false;
            SetExampTemporaryVar(m_exampleObject->m_exampTemporaryVar);
        }

        NetworkExampleControllerNotificationBus::Broadcast(
            &NetworkExampleControllerNotificationBus::Events::OnNetworkTickStart, deltaTime, m_exampleObject->m_isServer, GetEntityId());

        // Do work every network tick here

        NetworkExampleControllerNotificationBus::Broadcast(
            &NetworkExampleControllerNotificationBus::Events::OnNetworkTickFinish, deltaTime, m_exampleObject->m_isServer, GetEntityId());
    }

#if AZ_TRAIT_SERVER
    void NetworkExampleController::HandleSendExampTemporaryVarRPC(
        [[maybe_unused]] AzNetworking::IConnection* invokingConnection, const AZ::u32& exampTemporaryVar)
    {
        m_exampleObject->SetExampTemporaryVarRPC(exampTemporaryVar);
    }
#endif

    void NetworkExampleController::OnEnableNetworkExampleChanged(const bool enable)
    {
        m_disabled = !enable;
        m_exampleObject->m_networkExampleEnabled = enable;
    }

    // Request Bus getter and setter methods for use in scripts
    bool NetworkExampleController::GetEnabled() const
    {
        return !m_disabled;
    }
    void NetworkExampleController::SetEnabled(const bool enabled)
    {
        m_disabled = !enabled;
        m_exampleObject->m_networkExampleEnabled = enabled;
    }
} // namespace Hermes
