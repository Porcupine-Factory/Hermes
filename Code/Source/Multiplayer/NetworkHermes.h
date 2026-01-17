#pragma once
#include <Hermes/NetworkHermesControllerBus.h>

#include <Source/AutoGen/NetworkHermes.AutoComponent.h>

#include <Clients/HermesComponent.h>

namespace Hermes
{
    class HermesComponent;

    class NetworkHermesController
        : public NetworkHermesControllerBase
        , public NetworkHermesControllerRequestBus::Handler
    {
        friend class HermesComponent;

    public:
        NetworkHermesController(NetworkHermes& parent);

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

        // NetworkHermesControllerRequestBus
        bool GetEnabled() const override;
        void SetEnabled(const bool enabled) override;

#if AZ_TRAIT_SERVER
        void HandleSendNumberDelta(AzNetworking::IConnection* invokingConnection, const AZ::s64& numberDelta) override;
#endif

    private:
        // Used to initialize Network Properties from initial values in the Hermes component
        bool m_init = true;

        // HermesComponent objects
        HermesComponent* m_hermesObject = nullptr;

        // Signals when the controller is determined to be autonomous or not
        bool m_autonomousNotDetermined = true;

        // EnableNetworkHermes Changed Event
        AZ::Event<bool>::Handler m_enableNetworkHermesChangedEvent;
        void OnEnableNetworkHermesChanged(const bool enable);
        bool m_disabled = false;
    };
} // namespace Hermes
