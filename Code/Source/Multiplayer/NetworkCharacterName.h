#pragma once
#include <Hermes/NetworkCharacterNameControllerBus.h>

#include <Source/AutoGen/NetworkCharacterName.AutoComponent.h>

#include <Clients/CharacterNameComponent.h>

namespace Hermes
{
    class CharacterNameComponent;

    class NetworkCharacterNameController
        : public NetworkCharacterNameControllerBase
        , public NetworkCharacterNameControllerRequestBus::Handler
    {
        friend class CharacterNameComponent;

    public:
        NetworkCharacterNameController(NetworkCharacterName& parent);

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

        // NetworkCharacterNameControllerRequestBus
        bool GetEnabled() const override;
        void SetEnabled(const bool new_enabled) override;

#if AZ_TRAIT_SERVER
        void HandleSendNameOnServerRPC(AzNetworking::IConnection* invokingConnection, const AZStd::string& name) override;
#endif

    private:
        // NetworkCharacterNameControllerNotificationBus
        void OnNetworkTickStart(const float deltaTime, const bool server, const AZ::EntityId& entityId);
        void OnNetworkTickFinish(const float deltaTime, const bool server, const AZ::EntityId& entityId);

        // Used to initialize Network Properties from initial values in the CharacterName component
        bool m_init = true;

        // CharacterNameComponent objects
        CharacterNameComponent* m_characterNameObject = nullptr;

        // Signals when the controller is determined to be autonomous or not
        bool m_autonomousNotDetermined = true;

        // EnableNetworkCharacterName Changed Event
        AZ::Event<bool>::Handler m_enableNetworkCharacterNameChangedEvent;
        void OnEnableNetworkCharacterNameChanged(const bool enable);
        bool m_disabled = false;
    };
} // namespace Hermes
