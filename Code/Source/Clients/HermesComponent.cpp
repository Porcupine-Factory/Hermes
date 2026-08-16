#include <Clients/HermesComponent.h>
#include <Multiplayer/NetworkHermes.h>

#include <AzCore/Component/ComponentApplicationBus.h>
#include <AzCore/Component/Entity.h>
#include <AzCore/Serialization/EditContext.h>

namespace Hermes
{
    void HermesComponent::Reflect(AZ::ReflectContext* rc)
    {
        if (auto sc = azrtti_cast<AZ::SerializeContext*>(rc))
        {
            sc->Class<HermesComponent, AZ::Component>()
                // Data
                ->Field("Number", &HermesComponent::m_number)
                ->Attribute(AZ::Edit::Attributes::Min, 0)
                ->Version(1);

            if (AZ::EditContext* ec = sc->GetEditContext())
            {
                using namespace AZ::Edit::Attributes;
                ec->Class<HermesComponent>("Hermes", "Transfers data between clients and server for any entity with this component")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AppearsInAddComponentMenu, AZ_CRC_CE("Game"))
                    ->Attribute(Category, "Hermes")
                    ->Attribute(AZ::Edit::Attributes::HelpPageURL, "https://www.youtube.com/watch?v=O7rtXNlCNQQ")

                    ->ClassElement(AZ::Edit::ClassElements::Group, "Data")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, false)
                    ->DataElement(nullptr, &HermesComponent::m_number, "Number", "Initial number");
            }
        }

        if (auto bc = azrtti_cast<AZ::BehaviorContext*>(rc))
        {
            bc->EBus<HermesComponentNotificationBus>("HermesComponentNotificationBus")->Handler<HermesComponentNotificationHandler>();

            bc->EBus<HermesComponentRequestBus>("HermesComponentRequestBus")
                ->Attribute(AZ::Script::Attributes::Scope, AZ::Script::Attributes::ScopeFlags::Common)
                ->Attribute(AZ::Script::Attributes::Module, "datarelay")
                ->Attribute(AZ::Script::Attributes::Category, "Hermes")
                ->Event("Get Number", &HermesComponentRequests::GetNumber)
                ->Event("Set Number", &HermesComponentRequests::SetNumber)
                ->Event("Set Number Delta", &HermesComponentRequests::SetNumberDelta)
                ->Event("Get Is Autonomous Client", &HermesComponentRequests::GetIsAutonomousClient)
                ->Event("Get Is Server", &HermesComponentRequests::GetIsServer)
                ->Event("Get Is Host", &HermesComponentRequests::GetIsHost)
                ->Event("Get Locally Enable NetworkHermes", &HermesComponentRequests::GetLocallyEnableNetworkHermes)
                ->Event("Set Locally Enable NetworkHermes", &HermesComponentRequests::SetLocallyEnableNetworkHermes);

            bc->Class<HermesComponent>()->RequestBus("HermesComponentRequestBus");
        }
    }

    void HermesComponent::Activate()
    {
        AZ::TickBus::Handler::BusConnect();
        HermesComponentRequestBus::Handler::BusConnect(GetEntityId());
        NetworkHermesControllerNotificationBus::Handler::BusConnect(GetEntityId());
        AZ::EntityBus::Handler::BusConnect(GetEntityId());
    }

    void HermesComponent::Deactivate()
    {
        AZ::EntityBus::Handler::BusDisconnect();
        NetworkHermesControllerNotificationBus::Handler::BusDisconnect();
        HermesComponentRequestBus::Handler::BusDisconnect();
        AZ::TickBus::Handler::BusDisconnect();
    }

    void HermesComponent::OnEntityActivated([[maybe_unused]] const AZ::EntityId& entityId)
    {
        // Get access to the NetworkHermes object and its member
        const AZ::Entity* entity = GetEntity();
        m_networkHermesObject = entity->FindComponent<NetworkHermes>();

        // Determine if the NetworkHermes is enabled
        if (m_networkHermesObject != nullptr)
        {
            m_networkHermesControllerObject = static_cast<NetworkHermesController*>(m_networkHermesObject->GetController());
            NetworkHermesControllerRequestBus::BroadcastResult(
                m_networkHermesEnabled, &NetworkHermesControllerRequestBus::Events::GetEnabled);
        }
    }

    void HermesComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
    }

    void HermesComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
#ifdef FIRST_PERSON_CONTROLLER
        dependent.push_back(AZ_CRC_CE("FirstPersonControllerService"));
        dependent.push_back(AZ_CRC_CE("NetworkFPCService"));
#endif
    }

    void HermesComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("HermesService"));
    }

    void HermesComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("HermesService"));
    }

    void HermesComponent::OnTick(float deltaTime, AZ::ScriptTimePoint)
    {
        ProcessInput(deltaTime, 0);
    }

    void HermesComponent::OnNetworkTickStart(const float deltaTime, const bool server, const AZ::EntityId& entityId)
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
            HermesComponentNotificationBus::Broadcast(&HermesComponentNotificationBus::Events::OnNetworkHermesTickStart, deltaTime);
            if (!m_networkHermesEnabled)
                NetworkHermesControllerRequestBus::BroadcastResult(
                    m_networkHermesEnabled, &NetworkHermesControllerRequestBus::Events::GetEnabled);
            ProcessInput(deltaTime, 2);
        }
    }

    void HermesComponent::OnNetworkTickFinish(const float deltaTime, const bool server, const AZ::EntityId& entityId)
    {
        if ((!m_isAutonomousClient && !m_isServer && !m_isHost) || (entityId != GetEntityId()))
            return;
        if (!((m_isHost && server) || (m_isServer && !server)))
            HermesComponentNotificationBus::Broadcast(&HermesComponentNotificationBus::Events::OnNetworkHermesTickFinish, deltaTime);
    }

    void HermesComponent::GetNetworkHermesProperties()
    {
        if (m_networkHermesControllerObject != nullptr)
        {
            m_number = m_networkHermesControllerObject->GetNumber();
        }
    }

    void HermesComponent::SetNetworkHermesProperties() const
    {
        if (m_networkHermesControllerObject != nullptr)
        {
            m_networkHermesControllerObject->SetNumber(m_number);
        }
    }

    // Frame tick == 0, physics fixed timestep == 1 (not used in Hermes), network tick == 2
    void HermesComponent::ProcessInput([[maybe_unused]] const float deltaTime, const AZ::u8 tickTimestepNetwork)
    {
        if (tickTimestepNetwork == 2)
        {
            GetNetworkHermesProperties();
            SetNetworkHermesProperties();
        }
    }

    // Request Bus getter and setter methods for use in scripts
    AZ::u32 HermesComponent::GetNumber() const
    {
        if (m_networkHermesEnabled && m_networkHermesControllerObject != nullptr)
            return m_networkHermesControllerObject->GetNumber();
        else
            return m_number;
    }
    void HermesComponent::SetNumber(const AZ::u32& number)
    {
        m_number = number;
        if (m_networkHermesEnabled && m_networkHermesControllerObject != nullptr)
            m_networkHermesControllerObject->SetNumber(m_number);
    }
    void HermesComponent::SetNumberDelta(const AZ::s64& numberDelta)
    {
        if (numberDelta > 0 && numberDelta > AZStd::numeric_limits<AZ::u32>::max() - m_number)
            m_number = AZStd::numeric_limits<AZ::u32>::max();
        else
        {
            const AZ::s64 currentNumber = static_cast<AZ::s64>(m_number);
            const AZ::s64 newNumber =
                AZStd::clamp(currentNumber + numberDelta, 0LL, static_cast<AZ::s64>(AZStd::numeric_limits<AZ::u32>::max()));
            m_number = static_cast<AZ::u32>(newNumber);
        }
        if (m_networkHermesEnabled && m_networkHermesControllerObject != nullptr)
            m_networkHermesControllerObject->SetNumber(m_number);
    }
    bool HermesComponent::GetIsAutonomousClient() const
    {
        return m_isAutonomousClient;
    }
    bool HermesComponent::GetIsServer() const
    {
        return m_isServer;
    }
    bool HermesComponent::GetIsHost() const
    {
        return m_isHost;
    }
    bool HermesComponent::GetLocallyEnableNetworkHermes() const
    {
        return m_networkHermesEnabled;
    }
    void HermesComponent::SetLocallyEnableNetworkHermes(const bool networkHermesEnabled)
    {
        m_networkHermesEnabled = networkHermesEnabled;
        NetworkHermesControllerRequestBus::Event(
            GetEntityId(), &NetworkHermesControllerRequestBus::Events::SetEnabled, m_networkHermesEnabled);
    }
    void HermesComponent::IsAutonomousSoConnect()
    {
        AZ::TickBus::Handler::BusConnect();
    }
    void HermesComponent::NotAutonomousSoDisconnect()
    {
        AZ::TickBus::Handler::BusDisconnect();
    }
} // namespace Hermes
