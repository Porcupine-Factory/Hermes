#pragma once
#include <Hermes/NetworkSpawnerControllerBus.h>

#include <Source/AutoGen/NetworkSpawner.AutoComponent.h>

#include <Clients/SpawnerComponent.h>

namespace Hermes
{
    class SpawnerComponent;

    class NetworkSpawnerController
        : public NetworkSpawnerControllerBase
        , public NetworkSpawnerControllerRequestBus::Handler
    {
        friend class SpawnerComponent;

    public:
        NetworkSpawnerController(NetworkSpawner& parent);

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

        // NetworkSpawnerControllerRequestBus
        bool GetEnabled() const override;
        void SetEnabled(const bool& new_enabled) override;

        void HandleSpawnPrefabName(
            AzNetworking::IConnection* invokingConnection,
            const AZStd::string& prefabName,
            const AZ::Transform& transform,
            const bool& makeChild = false) override;
        void HandleSpawnPrefabPath(
            AzNetworking::IConnection* invokingConnection,
            const AZStd::string& prefabPath,
            const AZ::Transform& transform,
            const bool& makeChild = false) override;
        void HandleDepawnPrefabTicketId(AzNetworking::IConnection* invokingConnection, const uint32_t& prefabTicketId) override;

    private:
        // NetworkSpawnerControllerNotificationBus
        void OnNetworkTickStart(const float& deltaTime, const bool& server, const AZ::EntityId& entity);
        void OnNetworkTickFinish(const float& deltaTime, const bool& server, const AZ::EntityId& entity);

        // Used to initialize Network Properties from initial values in the Spawner component
        bool m_init = true;

        // SpawnerComponent objects
        SpawnerComponent* m_spawnerObject = nullptr;

        // Signals when the controller is determined to be autonomous or not
        bool m_autonomousNotDetermined = true;

        // EnableNetworkSpawner Changed Event
        AZ::Event<bool>::Handler m_enableNetworkSpawnerChangedEvent;
        void OnEnableNetworkSpawnerChanged(const bool& enable);
        bool m_disabled = false;
    };
} // namespace Hermes
