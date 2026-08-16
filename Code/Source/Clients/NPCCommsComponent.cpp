#include <Clients/NPCCommsComponent.h>
#include <Multiplayer/NetworkNPCComms.h>

#include <AzCore/Component/ComponentApplicationBus.h>
#include <AzCore/Component/Entity.h>
#include <AzCore/Serialization/EditContext.h>

namespace Hermes
{
    void NPCCommsComponent::Reflect(AZ::ReflectContext* rc)
    {
        if (auto sc = azrtti_cast<AZ::SerializeContext*>(rc))
        {
            sc->Class<NPCCommsComponent, AZ::Component>()
                // Data
                ->Field("Aggroed", &NPCCommsComponent::m_aggroed)
                ->Attribute(AZ::Edit::Attributes::Min, 0)
                ->Version(1);

            if (AZ::EditContext* ec = sc->GetEditContext())
            {
                using namespace AZ::Edit::Attributes;
                ec->Class<NPCCommsComponent>("NPC Comms", "Transfers data between clients and server for any entity with this component")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AppearsInAddComponentMenu, AZ_CRC_CE("Game"))
                    ->Attribute(Category, "Hermes")
                    ->Attribute(AZ::Edit::Attributes::HelpPageURL, "https://www.youtube.com/watch?v=O7rtXNlCNQQ")

                    ->ClassElement(AZ::Edit::ClassElements::Group, "Data")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, false)
                    ->DataElement(nullptr, &NPCCommsComponent::m_aggroed, "Aggroed", "Initial aggroed");
            }
        }

        if (auto bc = azrtti_cast<AZ::BehaviorContext*>(rc))
        {
            bc->EBus<NPCCommsComponentNotificationBus>("NPCCommsComponentNotificationBus")->Handler<NPCCommsComponentNotificationHandler>();

            bc->EBus<NPCCommsComponentRequestBus>("NPCCommsComponentRequestBus")
                ->Attribute(AZ::Script::Attributes::Scope, AZ::Script::Attributes::ScopeFlags::Common)
                ->Attribute(AZ::Script::Attributes::Module, "datarelay")
                ->Attribute(AZ::Script::Attributes::Category, "NPC Comms")
                ->Event("Get Aggroed", &NPCCommsComponentRequests::GetAggroed)
                ->Event("Set Aggroed", &NPCCommsComponentRequests::SetAggroed)
                ->Event("Set Aggroed RPC", &NPCCommsComponentRequests::SetAggroedRPC)
                ->Event("Get Is Autonomous Client", &NPCCommsComponentRequests::GetIsAutonomousClient)
                ->Event("Get Is Server", &NPCCommsComponentRequests::GetIsServer)
                ->Event("Get Is Host", &NPCCommsComponentRequests::GetIsHost)
                ->Event("Get Locally Enable NetworkNPCComms", &NPCCommsComponentRequests::GetLocallyEnableNetworkNPCComms)
                ->Event("Set Locally Enable NetworkNPCComms", &NPCCommsComponentRequests::SetLocallyEnableNetworkNPCComms);

            bc->Class<NPCCommsComponent>()->RequestBus("NPCCommsComponentRequestBus");
        }
    }

    void NPCCommsComponent::Activate()
    {
        AZ::TickBus::Handler::BusConnect();
        NPCCommsComponentRequestBus::Handler::BusConnect(GetEntityId());
        NetworkNPCCommsControllerNotificationBus::Handler::BusConnect(GetEntityId());
        AZ::EntityBus::Handler::BusConnect(GetEntityId());
    }

    void NPCCommsComponent::Deactivate()
    {
        AZ::EntityBus::Handler::BusDisconnect();
        NetworkNPCCommsControllerNotificationBus::Handler::BusDisconnect();
        NPCCommsComponentRequestBus::Handler::BusDisconnect();
        AZ::TickBus::Handler::BusDisconnect();
    }

    void NPCCommsComponent::OnEntityActivated([[maybe_unused]] const AZ::EntityId& entityId)
    {
        // Get access to the NetworkNPCComms object and its member
        const AZ::Entity* entity = GetEntity();
        m_networkNPCCommsObject = entity->FindComponent<NetworkNPCComms>();

        // Determine if the NetworkNPCComms is enabled
        if (m_networkNPCCommsObject != nullptr)
        {
            m_networkNPCCommsControllerObject = static_cast<NetworkNPCCommsController*>(m_networkNPCCommsObject->GetController());
            NetworkNPCCommsControllerRequestBus::BroadcastResult(
                m_networkNPCCommsEnabled, &NetworkNPCCommsControllerRequestBus::Events::GetEnabled);
        }
    }

    void NPCCommsComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
    }

    void NPCCommsComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
#ifdef FIRST_PERSON_CONTROLLER
        dependent.push_back(AZ_CRC_CE("FirstPersonControllerService"));
        dependent.push_back(AZ_CRC_CE("NetworkFPCService"));
#endif
    }

    void NPCCommsComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("NPCCommsService"));
    }

    void NPCCommsComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("NPCCommsService"));
    }

    void NPCCommsComponent::OnTick(float deltaTime, AZ::ScriptTimePoint)
    {
        ProcessInput(deltaTime, 0);
    }

    void NPCCommsComponent::OnNetworkTickStart(const float deltaTime, const bool server, const AZ::EntityId& entityId)
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
            NPCCommsComponentNotificationBus::Broadcast(&NPCCommsComponentNotificationBus::Events::OnNetworkNPCCommsTickStart, deltaTime);
            if (!m_networkNPCCommsEnabled)
                NetworkNPCCommsControllerRequestBus::BroadcastResult(
                    m_networkNPCCommsEnabled, &NetworkNPCCommsControllerRequestBus::Events::GetEnabled);
            ProcessInput(deltaTime, 2);
        }
    }

    void NPCCommsComponent::OnNetworkTickFinish(const float deltaTime, const bool server, const AZ::EntityId& entityId)
    {
        if ((!m_isAutonomousClient && !m_isServer && !m_isHost) || (entityId != GetEntityId()))
            return;
        if (!((m_isHost && server) || (m_isServer && !server)))
            NPCCommsComponentNotificationBus::Broadcast(&NPCCommsComponentNotificationBus::Events::OnNetworkNPCCommsTickFinish, deltaTime);
    }

    void NPCCommsComponent::GetNetworkNPCCommsProperties()
    {
        if (m_networkNPCCommsControllerObject != nullptr)
        {
            m_aggroed = m_networkNPCCommsControllerObject->GetAggroed();
        }
    }

    void NPCCommsComponent::SetNetworkNPCCommsProperties() const
    {
        if (m_networkNPCCommsControllerObject != nullptr)
        {
            m_networkNPCCommsControllerObject->SetAggroed(m_aggroed);
        }
    }

    // Frame tick == 0, physics fixed timestep == 1 (not used in NPCComms), network tick == 2
    void NPCCommsComponent::ProcessInput([[maybe_unused]] const float deltaTime, const AZ::u8 tickTimestepNetwork)
    {
        if (tickTimestepNetwork == 2)
        {
            GetNetworkNPCCommsProperties();
            SetNetworkNPCCommsProperties();
        }
    }

    // Event Notification methods for use in scripts
    void NPCCommsComponent::OnNetworkNPCCommsTickStart([[maybe_unused]] const float deltaTime)
    {
    }
    void NPCCommsComponent::OnNetworkNPCCommsTickFinish([[maybe_unused]] const float deltaTime)
    {
    }

    // Request Bus getter and setter methods for use in scripts
    bool NPCCommsComponent::GetAggroed() const
    {
        if (m_networkNPCCommsEnabled && m_networkNPCCommsControllerObject != nullptr)
            return m_networkNPCCommsControllerObject->GetAggroed();
        else
            return m_aggroed;
    }
    void NPCCommsComponent::SetAggroed(const bool aggroed)
    {
        m_aggroed = aggroed;
        if (m_networkNPCCommsEnabled && m_networkNPCCommsControllerObject != nullptr)
            m_networkNPCCommsControllerObject->SetAggroed(m_aggroed);
    }
    void NPCCommsComponent::SetAggroedRPC(const bool aggroed)
    {
        m_aggroed = aggroed;
        if (m_networkNPCCommsEnabled && m_networkNPCCommsControllerObject != nullptr)
            m_networkNPCCommsControllerObject->SetAggroed(m_aggroed);
    }
    bool NPCCommsComponent::GetIsAutonomousClient() const
    {
        return m_isAutonomousClient;
    }
    bool NPCCommsComponent::GetIsServer() const
    {
        return m_isServer;
    }
    bool NPCCommsComponent::GetIsHost() const
    {
        return m_isHost;
    }
    bool NPCCommsComponent::GetLocallyEnableNetworkNPCComms() const
    {
        return m_networkNPCCommsEnabled;
    }
    void NPCCommsComponent::SetLocallyEnableNetworkNPCComms(const bool networkNPCCommsEnabled)
    {
        m_networkNPCCommsEnabled = networkNPCCommsEnabled;
        NetworkNPCCommsControllerRequestBus::Event(
            GetEntityId(), &NetworkNPCCommsControllerRequestBus::Events::SetEnabled, m_networkNPCCommsEnabled);
    }
    void NPCCommsComponent::IsAutonomousSoConnect()
    {
        AZ::TickBus::Handler::BusConnect();
    }
    void NPCCommsComponent::NotAutonomousSoDisconnect()
    {
        AZ::TickBus::Handler::BusDisconnect();
    }
} // namespace Hermes
