#include <Clients/ExampleComponent.h>
#include <Multiplayer/NetworkExample.h>

#include <AzCore/Component/ComponentApplicationBus.h>
#include <AzCore/Component/Entity.h>
#include <AzCore/Serialization/EditContext.h>

namespace Hermes
{
    void ExampleComponent::Reflect(AZ::ReflectContext* rc)
    {
        if (auto sc = azrtti_cast<AZ::SerializeContext*>(rc))
        {
            sc->Class<ExampleComponent, AZ::Component>()
                // Data
                ->Field("ExampTemporaryVar", &ExampleComponent::m_exampTemporaryVar)
                ->Attribute(AZ::Edit::Attributes::Min, 0)
                ->Version(1);

            if (AZ::EditContext* ec = sc->GetEditContext())
            {
                using namespace AZ::Edit::Attributes;
                ec->Class<ExampleComponent>("Example", "Transfers data between clients and server for any entity with this component")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AppearsInAddComponentMenu, AZ_CRC_CE("Game"))
                    ->Attribute(Category, "Hermes")
                    ->Attribute(AZ::Edit::Attributes::HelpPageURL, "https://www.youtube.com/watch?v=O7rtXNlCNQQ")

                    ->ClassElement(AZ::Edit::ClassElements::Group, "Data")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, false)
                    ->DataElement(nullptr, &ExampleComponent::m_exampTemporaryVar, "ExampTemporaryVar", "Initial exampTemporaryVar");
            }
        }

        if (auto bc = azrtti_cast<AZ::BehaviorContext*>(rc))
        {
            bc->EBus<ExampleComponentNotificationBus>("ExampleComponentNotificationBus")->Handler<ExampleComponentNotificationHandler>();

            bc->EBus<ExampleComponentRequestBus>("ExampleComponentRequestBus")
                ->Attribute(AZ::Script::Attributes::Scope, AZ::Script::Attributes::ScopeFlags::Common)
                ->Attribute(AZ::Script::Attributes::Module, "datarelay")
                ->Attribute(AZ::Script::Attributes::Category, "Example")
                ->Event("Get ExampTemporaryVar", &ExampleComponentRequests::GetExampTemporaryVar)
                ->Event("Set ExampTemporaryVar", &ExampleComponentRequests::SetExampTemporaryVar)
                ->Event("Set ExampTemporaryVar RPC", &ExampleComponentRequests::SetExampTemporaryVarRPC)
                ->Event("Get Is Autonomous Client", &ExampleComponentRequests::GetIsAutonomousClient)
                ->Event("Get Is Server", &ExampleComponentRequests::GetIsServer)
                ->Event("Get Is Host", &ExampleComponentRequests::GetIsHost)
                ->Event("Get Locally Enable NetworkExample", &ExampleComponentRequests::GetLocallyEnableNetworkExample)
                ->Event("Set Locally Enable NetworkExample", &ExampleComponentRequests::SetLocallyEnableNetworkExample);

            bc->Class<ExampleComponent>()->RequestBus("ExampleComponentRequestBus");
        }
    }

    void ExampleComponent::Activate()
    {
        AZ::TickBus::Handler::BusConnect();
        ExampleComponentRequestBus::Handler::BusConnect(GetEntityId());
        NetworkExampleControllerNotificationBus::Handler::BusConnect(GetEntityId());
        AZ::EntityBus::Handler::BusConnect(GetEntityId());
    }

    void ExampleComponent::Deactivate()
    {
        AZ::EntityBus::Handler::BusDisconnect();
        NetworkExampleControllerNotificationBus::Handler::BusDisconnect();
        ExampleComponentRequestBus::Handler::BusDisconnect();
        AZ::TickBus::Handler::BusDisconnect();
    }

    void ExampleComponent::OnEntityActivated([[maybe_unused]] const AZ::EntityId& entityId)
    {
        // Get access to the NetworkExample object and its member
        const AZ::Entity* entity = GetEntity();
        m_networkExampleObject = entity->FindComponent<NetworkExample>();

        // Determine if the NetworkExample is enabled
        if (m_networkExampleObject != nullptr)
        {
            m_networkExampleControllerObject = static_cast<NetworkExampleController*>(m_networkExampleObject->GetController());
            NetworkExampleControllerRequestBus::BroadcastResult(
                m_networkExampleEnabled, &NetworkExampleControllerRequestBus::Events::GetEnabled);
        }
    }

    void ExampleComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
    }

    void ExampleComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
#ifdef FIRST_PERSON_CONTROLLER
        dependent.push_back(AZ_CRC_CE("FirstPersonControllerService"));
        dependent.push_back(AZ_CRC_CE("NetworkFPCService"));
#endif
    }

    void ExampleComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("ExampleService"));
    }

    void ExampleComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("ExampleService"));
    }

    void ExampleComponent::OnTick(float deltaTime, AZ::ScriptTimePoint)
    {
        ProcessInput(deltaTime, 0);
    }

    void ExampleComponent::OnNetworkTickStart(const float deltaTime, const bool server, const AZ::EntityId& entityId)
    {
        if (!m_isAutonomousClient && !m_isServer && !m_isHost)
        {
            NotAutonomousSoDisconnect();
            return;
        }
        if (entityId != GetEntityId())
            return;
        if (!((m_isHost && server) || (m_isServer && !server)))
        {
            ExampleComponentNotificationBus::Broadcast(&ExampleComponentNotificationBus::Events::OnNetworkExampleTickStart, deltaTime);
            if (!m_networkExampleEnabled)
                NetworkExampleControllerRequestBus::BroadcastResult(
                    m_networkExampleEnabled, &NetworkExampleControllerRequestBus::Events::GetEnabled);
            ProcessInput(deltaTime, 2);
        }
    }

    void ExampleComponent::OnNetworkTickFinish(const float deltaTime, const bool server, const AZ::EntityId& entityId)
    {
        if ((!m_isAutonomousClient && !m_isServer && !m_isHost) || (entityId != GetEntityId()))
            return;
        if (!((m_isHost && server) || (m_isServer && !server)))
            ExampleComponentNotificationBus::Broadcast(&ExampleComponentNotificationBus::Events::OnNetworkExampleTickFinish, deltaTime);
    }

    void ExampleComponent::GetNetworkExampleProperties()
    {
        if (m_networkExampleControllerObject != nullptr)
        {
            m_exampTemporaryVar = m_networkExampleControllerObject->GetExampTemporaryVar();
        }
    }

    void ExampleComponent::SetNetworkExampleProperties() const
    {
        if (m_networkExampleControllerObject != nullptr)
        {
            m_networkExampleControllerObject->SetExampTemporaryVar(m_exampTemporaryVar);
        }
    }

    // Frame tick == 0, physics fixed timestep == 1 (not used in Example), network tick == 2
    void ExampleComponent::ProcessInput([[maybe_unused]] const float deltaTime, const AZ::u8& tickTimestepNetwork)
    {
        if (tickTimestepNetwork == 2)
        {
            GetNetworkExampleProperties();
            SetNetworkExampleProperties();
        }
    }

    // Request Bus getter and setter methods for use in scripts
    AZ::u32 ExampleComponent::GetExampTemporaryVar() const
    {
        if (m_networkExampleEnabled && m_networkExampleControllerObject != nullptr)
            return m_networkExampleControllerObject->GetExampTemporaryVar();
        else
            return m_exampTemporaryVar;
    }
    void ExampleComponent::SetExampTemporaryVar(const AZ::u32& new_exampTemporaryVar)
    {
        m_exampTemporaryVar = new_exampTemporaryVar;
        if (m_networkExampleEnabled && m_networkExampleControllerObject != nullptr)
            m_networkExampleControllerObject->SetExampTemporaryVar(m_exampTemporaryVar);
    }
    void ExampleComponent::SetExampTemporaryVarRPC(const AZ::u32& new_exampTemporaryVar)
    {
        m_exampTemporaryVar = new_exampTemporaryVar;
        if (m_networkExampleEnabled && m_networkExampleControllerObject != nullptr)
            m_networkExampleControllerObject->SetExampTemporaryVar(m_exampTemporaryVar);
    }
    bool ExampleComponent::GetIsAutonomousClient() const
    {
        return m_isAutonomousClient;
    }
    bool ExampleComponent::GetIsServer() const
    {
        return m_isServer;
    }
    bool ExampleComponent::GetIsHost() const
    {
        return m_isHost;
    }
    bool ExampleComponent::GetLocallyEnableNetworkExample() const
    {
        return m_networkExampleEnabled;
    }
    void ExampleComponent::SetLocallyEnableNetworkExample(const bool new_networkExampleEnabled)
    {
        m_networkExampleEnabled = new_networkExampleEnabled;
        NetworkExampleControllerRequestBus::Event(
            GetEntityId(), &NetworkExampleControllerRequestBus::Events::SetEnabled, m_networkExampleEnabled);
    }
    void ExampleComponent::IsAutonomousSoConnect()
    {
        AZ::TickBus::Handler::BusConnect();
    }
    void ExampleComponent::NotAutonomousSoDisconnect()
    {
        AZ::TickBus::Handler::BusDisconnect();
    }
} // namespace Hermes
