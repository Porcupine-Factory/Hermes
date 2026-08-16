#include <Clients/CharacterNameComponent.h>
#include <Multiplayer/NetworkCharacterName.h>

#include <AzCore/Component/ComponentApplicationBus.h>
#include <AzCore/Component/Entity.h>
#include <AzCore/Serialization/EditContext.h>

namespace Hermes
{
    void CharacterNameComponent::Reflect(AZ::ReflectContext* rc)
    {
        if (auto sc = azrtti_cast<AZ::SerializeContext*>(rc))
        {
            sc->Class<CharacterNameComponent, AZ::Component>()
                // Data
                ->Field("Name", &CharacterNameComponent::m_name)
                ->Attribute(AZ::Edit::Attributes::Min, 0)
                ->Version(1);

            if (AZ::EditContext* ec = sc->GetEditContext())
            {
                using namespace AZ::Edit::Attributes;
                ec->Class<CharacterNameComponent>(
                      "Character Name", "Transfers data between clients and server for any entity with this component")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AppearsInAddComponentMenu, AZ_CRC_CE("Game"))
                    ->Attribute(Category, "Hermes")
                    ->Attribute(AZ::Edit::Attributes::HelpPageURL, "https://www.youtube.com/watch?v=O7rtXNlCNQQ")

                    ->ClassElement(AZ::Edit::ClassElements::Group, "Data")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, false)
                    ->DataElement(nullptr, &CharacterNameComponent::m_name, "Name", "Initial name");
            }
        }

        if (auto bc = azrtti_cast<AZ::BehaviorContext*>(rc))
        {
            bc->EBus<CharacterNameComponentNotificationBus>("CharacterNameComponentNotificationBus")
                ->Handler<CharacterNameComponentNotificationHandler>();

            bc->EBus<CharacterNameComponentRequestBus>("CharacterNameComponentRequestBus")
                ->Attribute(AZ::Script::Attributes::Scope, AZ::Script::Attributes::ScopeFlags::Common)
                ->Attribute(AZ::Script::Attributes::Module, "datarelay")
                ->Attribute(AZ::Script::Attributes::Category, "Character Name")
                ->Event("Get Name", &CharacterNameComponentRequests::GetName)
                ->Event("Set Name", &CharacterNameComponentRequests::SetName)
                ->Event("Get Is Autonomous Client", &CharacterNameComponentRequests::GetIsAutonomousClient)
                ->Event("Get Is Server", &CharacterNameComponentRequests::GetIsServer)
                ->Event("Get Is Host", &CharacterNameComponentRequests::GetIsHost)
                ->Event("Get Locally Enable NetworkCharacterName", &CharacterNameComponentRequests::GetLocallyEnableNetworkCharacterName)
                ->Event("Set Locally Enable NetworkCharacterName", &CharacterNameComponentRequests::SetLocallyEnableNetworkCharacterName);

            bc->Class<CharacterNameComponent>()->RequestBus("CharacterNameComponentRequestBus");
        }
    }

    void CharacterNameComponent::Activate()
    {
        AZ::TickBus::Handler::BusConnect();
        CharacterNameComponentRequestBus::Handler::BusConnect(GetEntityId());
        NetworkCharacterNameControllerNotificationBus::Handler::BusConnect(GetEntityId());
        AZ::EntityBus::Handler::BusConnect(GetEntityId());
    }

    void CharacterNameComponent::Deactivate()
    {
        AZ::EntityBus::Handler::BusDisconnect();
        NetworkCharacterNameControllerNotificationBus::Handler::BusDisconnect();
        CharacterNameComponentRequestBus::Handler::BusDisconnect();
        AZ::TickBus::Handler::BusDisconnect();
    }

    void CharacterNameComponent::OnEntityActivated([[maybe_unused]] const AZ::EntityId& entityId)
    {
        // Get access to the NetworkCharacterName object and its member
        const AZ::Entity* entity = GetEntity();
        m_networkCharacterNameObject = entity->FindComponent<NetworkCharacterName>();

        // Determine if the NetworkCharacterName is enabled
        if (m_networkCharacterNameObject != nullptr)
        {
            m_networkCharacterNameControllerObject =
                static_cast<NetworkCharacterNameController*>(m_networkCharacterNameObject->GetController());
            NetworkCharacterNameControllerRequestBus::BroadcastResult(
                m_networkCharacterNameEnabled, &NetworkCharacterNameControllerRequestBus::Events::GetEnabled);
        }
    }

    void CharacterNameComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
    }

    void CharacterNameComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
#ifdef FIRST_PERSON_CONTROLLER
        dependent.push_back(AZ_CRC_CE("FirstPersonControllerService"));
        dependent.push_back(AZ_CRC_CE("NetworkFPCService"));
#endif
    }

    void CharacterNameComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("CharacterNameService"));
    }

    void CharacterNameComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("CharacterNameService"));
    }

    void CharacterNameComponent::OnTick(float deltaTime, AZ::ScriptTimePoint)
    {
        ProcessInput(deltaTime, 0);
    }

    void CharacterNameComponent::OnNetworkTickStart(const float deltaTime, const bool server, const AZ::EntityId& entityId)
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
            CharacterNameComponentNotificationBus::Broadcast(
                &CharacterNameComponentNotificationBus::Events::OnNetworkCharacterNameTickStart, deltaTime);
            if (!m_networkCharacterNameEnabled)
                NetworkCharacterNameControllerRequestBus::BroadcastResult(
                    m_networkCharacterNameEnabled, &NetworkCharacterNameControllerRequestBus::Events::GetEnabled);
            ProcessInput(deltaTime, 2);
        }
    }

    void CharacterNameComponent::OnNetworkTickFinish(const float deltaTime, const bool server, const AZ::EntityId& entityId)
    {
        if ((!m_isAutonomousClient && !m_isServer && !m_isHost) || (entityId != GetEntityId()))
            return;
        if (!((m_isHost && server) || (m_isServer && !server)))
            CharacterNameComponentNotificationBus::Broadcast(
                &CharacterNameComponentNotificationBus::Events::OnNetworkCharacterNameTickFinish, deltaTime);
    }

    void CharacterNameComponent::GetNetworkCharacterNameProperties()
    {
        if (m_networkCharacterNameControllerObject != nullptr)
        {
        }
    }

    void CharacterNameComponent::SetNetworkCharacterNameProperties()
    {
        if (m_networkCharacterNameControllerObject != nullptr)
        {
        }
    }

    // Frame tick == 0, physics fixed timestep == 1 (not used in CharacterName), network tick == 2
    void CharacterNameComponent::ProcessInput([[maybe_unused]] const float deltaTime, [[maybe_unused]] const AZ::u8 tickTimestepNetwork)
    {
        // No need to get and set the name network property every tick since it's handled with the RPC call
        /*if (m_isServer && tickTimestepNetwork == 2)
        {
            GetNetworkCharacterNameProperties();
            SetNetworkCharacterNameProperties();
        }*/
    }

    // Request Bus getter and setter methods for use in scripts
    AZStd::string CharacterNameComponent::GetName() const
    {
        if (m_networkCharacterNameEnabled && m_networkCharacterNameControllerObject != nullptr)
            return m_networkCharacterNameControllerObject->GetName();
        else
            return m_name;
    }
    void CharacterNameComponent::SetName(const AZStd::string& name)
    {
        m_name = name;
        if (m_networkCharacterNameEnabled && m_networkCharacterNameControllerObject != nullptr)
        {
            m_networkCharacterNameControllerObject->SetName(m_name);
            m_networkCharacterNameControllerObject->SendNameOnServerRPC(m_name);
        }
    }
    void CharacterNameComponent::SetNameOnServerRPC(const AZStd::string& name)
    {
        m_name = name;
        if (m_networkCharacterNameEnabled && m_networkCharacterNameControllerObject != nullptr)
            m_networkCharacterNameControllerObject->SetName(m_name);
    }
    bool CharacterNameComponent::GetIsAutonomousClient() const
    {
        return m_isAutonomousClient;
    }
    bool CharacterNameComponent::GetIsServer() const
    {
        return m_isServer;
    }
    bool CharacterNameComponent::GetIsHost() const
    {
        return m_isHost;
    }
    bool CharacterNameComponent::GetLocallyEnableNetworkCharacterName() const
    {
        return m_networkCharacterNameEnabled;
    }
    void CharacterNameComponent::SetLocallyEnableNetworkCharacterName(const bool networkCharacterNameEnabled)
    {
        m_networkCharacterNameEnabled = networkCharacterNameEnabled;
        NetworkCharacterNameControllerRequestBus::Event(
            GetEntityId(), &NetworkCharacterNameControllerRequestBus::Events::SetEnabled, m_networkCharacterNameEnabled);
    }
    void CharacterNameComponent::IsAutonomousSoConnect()
    {
        AZ::TickBus::Handler::BusConnect();
    }
    void CharacterNameComponent::NotAutonomousSoDisconnect()
    {
        AZ::TickBus::Handler::BusDisconnect();
    }
} // namespace Hermes
