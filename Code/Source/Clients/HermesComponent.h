#pragma once
#include <Hermes/HermesComponentBus.h>
#include <Hermes/NetworkHermesControllerBus.h>

#include <AzCore/Component/Component.h>
#include <AzCore/Component/EntityBus.h>
#include <AzCore/Component/TickBus.h>

namespace Hermes
{
    class NetworkHermes;
    class NetworkHermesController;

    class HermesComponent
        : public AZ::Component
        , public AZ::TickBus::Handler
        , public NetworkHermesControllerNotificationBus::Handler
        , public HermesComponentRequestBus::Handler
        , public AZ::EntityBus::Handler
    {
        friend class NetworkHermesController;

    public:
        AZ_COMPONENT(HermesComponent, "{bbf76879-7fe5-4428-b8ae-6310297c8da0}");

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

        // HermesComponentRequestBus
        AZ::u32 GetNumber() const override;
        void SetNumber(const AZ::u32 number) override;
        void SetNumberDelta(const AZ::s64 numberDelta) override;
        bool GetIsAutonomousClient() const override;
        bool GetIsServer() const override;
        bool GetIsHost() const override;
        bool GetLocallyEnableNetworkHermes() const override;
        void SetLocallyEnableNetworkHermes(const bool networkHermesEnabled) override;
        void IsAutonomousSoConnect() override;
        void NotAutonomousSoDisconnect() override;

    private:
        // TickBus interface
        void OnTick(float deltaTime, AZ::ScriptTimePoint) override;

        // Called on each tick
        void ProcessInput(const float deltaTime, const AZ::u8 tickTimestepNetwork);

        // NetworkHermesControllerNotificationBus
        void OnNetworkTickStart(const float deltaTime, const bool server, const AZ::EntityId& entityId);
        void OnNetworkTickFinish(const float deltaTime, const bool server, const AZ::EntityId& entityId);

        // Various methods used to implement the Hermes functionality
        void GetNetworkHermesProperties();
        void SetNetworkHermesProperties() const;

        // NetworkHermes object
        NetworkHermes* m_networkHermesObject = nullptr;
        NetworkHermesController* m_networkHermesControllerObject = nullptr;

        // Various data structures
        AZ::u32 m_number = 0;

        // Networking related variables
        bool m_networkHermesEnabled = false;
        bool m_isServer = false;
        bool m_isHost = false;
        bool m_isAutonomousClient = false;
    };
} // namespace Hermes
