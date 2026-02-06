#pragma once
#include <Hermes/NetworkNPCCommsControllerBus.h>

#include <Source/AutoGen/NetworkNPCComms.AutoComponent.h>

#include <Clients/NPCCommsComponent.h>

namespace Hermes
{
    class NPCCommsComponent;

    class NetworkNPCCommsController
        : public NetworkNPCCommsControllerBase
        , public NetworkNPCCommsControllerRequestBus::Handler
    {
        friend class NPCCommsComponent;

    public:
        NetworkNPCCommsController(NetworkNPCComms& parent);

        void OnActivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;
        void OnDeactivate(Multiplayer::EntityIsMigrating entityIsMigrating) override;

        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);

        //! Common input creation logic for the NetworkInput.
        //! Fill out the input struct and the MultiplayerInputDriver will send the input data over the network
        //!    to ensure it's processed.
        //! @param input  input structure which to store input data for sending to the authority
        //! @param deltaTime amount of time to integrate the provided inputs over
        void CreateInput(Multiplayer::NetworkInput& input, float deltaTime) override;

        //! Common input processing logic for the NetworkInput.
        //! @param input  input structure to process
        //! @param deltaTime amount of time to integrate the provided inputs over
        void ProcessInput(Multiplayer::NetworkInput& input, float deltaTime) override;

        // NetworkNPCCommsControllerRequestBus
        bool GetEnabled() const override;
        void SetEnabled(const bool& new_enabled) override;

#if AZ_TRAIT_SERVER
        void HandleSendAggroedRPC(AzNetworking::IConnection* invokingConnection, const bool& aggroed) override;
#endif

    private:
        // NetworkNPCCommsControllerNotificationBus
        void OnNetworkTickStart(const float& deltaTime, const bool& server, const AZ::EntityId& entityId);
        void OnNetworkTickFinish(const float& deltaTime, const bool& server, const AZ::EntityId& entityId);

        // Used to initialize Network Properties from initial values in the NPCComms component
        bool m_init = true;

        // NPCCommsComponent objects
        NPCCommsComponent* m_nPCCommsObject = nullptr;

        // Signals when the controller is determined to be autonomous or not
        bool m_autonomousNotDetermined = true;

        // EnableNetworkNPCComms Changed Event
        AZ::Event<bool>::Handler m_enableNetworkNPCCommsChangedEvent;
        void OnEnableNetworkNPCCommsChanged(const bool& enable);
        bool m_disabled = false;
    };
} // namespace Hermes
