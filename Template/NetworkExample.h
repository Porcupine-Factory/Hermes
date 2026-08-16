#pragma once
#include <Hermes/NetworkExampleControllerBus.h>

#include <Source/AutoGen/NetworkExample.AutoComponent.h>

#include <Clients/ExampleComponent.h>

namespace Hermes
{
    class ExampleComponent;

    class NetworkExampleController
        : public NetworkExampleControllerBase
        , public NetworkExampleControllerRequestBus::Handler
    {
        friend class ExampleComponent;

    public:
        NetworkExampleController(NetworkExample& parent);

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

        // NetworkExampleControllerRequestBus
        bool GetEnabled() const override;
        void SetEnabled(const bool enabled) override;

#if AZ_TRAIT_SERVER
        void HandleSendExampTemporaryVarRPC(AzNetworking::IConnection* invokingConnection, const AZ::u32& exampTemporaryVar) override;
#endif

    private:
        // NetworkExampleControllerNotificationBus
        void OnNetworkTickStart(const float deltaTime, const bool server, const AZ::EntityId& entityId);
        void OnNetworkTickFinish(const float deltaTime, const bool server, const AZ::EntityId& entityId);

        // Used to initialize Network Properties from initial values in the Example component
        bool m_init = true;

        // ExampleComponent objects
        ExampleComponent* m_exampleObject = nullptr;

        // Signals when the controller is determined to be autonomous or not
        bool m_autonomousNotDetermined = true;

        // EnableNetworkExample Changed Event
        AZ::Event<bool>::Handler m_enableNetworkExampleChangedEvent;
        void OnEnableNetworkExampleChanged(const bool enable);
        bool m_disabled = false;
    };
} // namespace Hermes
