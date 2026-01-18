#include <Clients/SpawnerComponent.h>
#include <Multiplayer/NetworkSpawner.h>

#include <AzCore/Asset/AssetCommon.h>
#include <AzCore/Asset/AssetManager.h>
#include <AzCore/Component/ComponentApplicationBus.h>
#include <AzCore/Component/Entity.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzFramework/Asset/AssetSystemBus.h>
#include <AzFramework/Components/TransformComponent.h>

namespace Hermes
{
    void SpawnerComponent::Reflect(AZ::ReflectContext* rc)
    {
        if (auto sc = azrtti_cast<AZ::SerializeContext*>(rc))
        {
            sc->Class<SpawnerComponent, AZ::Component>()
                // Data
                ->Version(1);

            if (AZ::EditContext* ec = sc->GetEditContext())
            {
                using namespace AZ::Edit::Attributes;
                ec->Class<SpawnerComponent>("Spawner", "Transfers data between clients and server for any entity with this component")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AppearsInAddComponentMenu, AZ_CRC_CE("Game"))
                    ->Attribute(Category, "Hermes")
                    ->Attribute(AZ::Edit::Attributes::HelpPageURL, "https://www.youtube.com/watch?v=O7rtXNlCNQQ")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, false);
            }
        }

        if (auto bc = azrtti_cast<AZ::BehaviorContext*>(rc))
        {
            bc->EBus<SpawnerComponentNotificationBus>("SpawnerComponentNotificationBus")->Handler<SpawnerComponentNotificationHandler>();

            bc->EBus<SpawnerComponentRequestBus>("SpawnerComponentRequestBus")
                ->Attribute(AZ::Script::Attributes::Scope, AZ::Script::Attributes::ScopeFlags::Common)
                ->Attribute(AZ::Script::Attributes::Module, "datarelay")
                ->Attribute(AZ::Script::Attributes::Category, "Hermes")
                ->Event("Spawn Prefab Name", &SpawnerComponentRequests::SpawnPrefabName)
                ->Event("Despawn Prefab TicketId", &SpawnerComponentRequests::DepawnPrefabTicketId)
                ->Event("Get Last Spawned TicketId", &SpawnerComponentRequests::GetLastSpawnedTicketId)
                ->Event("Set Last Spawned TicketId", &SpawnerComponentRequests::SetLastSpawnedTicketId)
                ->Event("Get Last Despawned TicketId", &SpawnerComponentRequests::GetLastDespawnedTicketId)
                ->Event("Set Last Despawned TicketId", &SpawnerComponentRequests::SetLastDespawnedTicketId)
                ->Event("Get Is Autonomous Client", &SpawnerComponentRequests::GetIsAutonomousClient)
                ->Event("Get Is Server", &SpawnerComponentRequests::GetIsServer)
                ->Event("Get Is Host", &SpawnerComponentRequests::GetIsHost)
                ->Event("Get Locally Enable NetworkSpawner", &SpawnerComponentRequests::GetLocallyEnableNetworkSpawner)
                ->Event("Set Locally Enable NetworkSpawner", &SpawnerComponentRequests::SetLocallyEnableNetworkSpawner);

            bc->Class<SpawnerComponent>()->RequestBus("SpawnerComponentRequestBus");
        }
    }

    void SpawnerComponent::Activate()
    {
        AZ::TickBus::Handler::BusConnect();
        SpawnerComponentRequestBus::Handler::BusConnect(GetEntityId());
        NetworkSpawnerControllerNotificationBus::Handler::BusConnect(GetEntityId());
        AZ::EntityBus::Handler::BusConnect(GetEntityId());
    }

    void SpawnerComponent::Deactivate()
    {
        for (auto ticket : m_spawnTickets)
        {
            if (ticket->IsValid())
            {
                AzFramework::SpawnableEntitiesInterface::Get()->DespawnAllEntities(*ticket);
            }
        }

        AZ::EntityBus::Handler::BusDisconnect();
        NetworkSpawnerControllerNotificationBus::Handler::BusDisconnect();
        AZ::TickBus::Handler::BusDisconnect();
    }

    void SpawnerComponent::OnEntityActivated([[maybe_unused]] const AZ::EntityId& entityId)
    {
        // Get access to the NetworkSpawner object and its member
        const AZ::Entity* entity = GetEntity();
        m_networkSpawnerObject = entity->FindComponent<NetworkSpawner>();

        // Determine if the NetworkSpawner is enabled
        if (m_networkSpawnerObject != nullptr)
        {
            m_networkSpawnerControllerObject = static_cast<NetworkSpawnerController*>(m_networkSpawnerObject->GetController());
            NetworkSpawnerControllerRequestBus::BroadcastResult(
                m_networkSpawnerEnabled, &NetworkSpawnerControllerRequestBus::Events::GetEnabled);
        }

        // Get all of the asset paths
        AZ::Data::AssetCatalogRequestBus::BroadcastResult(
            m_spawnablePaths, &AZ::Data::AssetCatalogRequestBus::Events::GetRegisteredAssetPaths);

        // Remove all asset paths that are not spawnable
        m_spawnablePaths.erase(
            AZStd::remove_if(
                m_spawnablePaths.begin(),
                m_spawnablePaths.end(),
                [&](const AZStd::string& path)
                {
                    return path.find(".spawnable") == AZStd::string::npos;
                }),
            m_spawnablePaths.end());
    }

    void SpawnerComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
    }

    void SpawnerComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
#ifdef FIRST_PERSON_CONTROLLER
        dependent.push_back(AZ_CRC_CE("FirstPersonControllerService"));
        dependent.push_back(AZ_CRC_CE("NetworkFPCService"));
#endif
    }

    void SpawnerComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("SpawnerService"));
    }

    void SpawnerComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("SpawnerService"));
    }

    void SpawnerComponent::OnTick(float deltaTime, AZ::ScriptTimePoint)
    {
        ProcessInput(((deltaTime + m_prevDeltaTime) / 2.f), 0);
        m_prevDeltaTime = deltaTime;
    }

    void SpawnerComponent::OnNetworkTickStart(const float& deltaTime, const bool& server, const AZ::EntityId& entity)
    {
        if (!m_isAutonomousClient && !m_isServer && !m_isHost)
        {
            NotAutonomousSoDisconnect();
            return;
        }
        if (entity != GetEntityId())
            return;
        if (!((m_isHost && server) || (m_isServer && !server)))
        {
            SpawnerComponentNotificationBus::Broadcast(
                &SpawnerComponentNotificationBus::Events::OnNetworkSpawnerTickStart, (deltaTime + m_prevNetworkSpawnerDeltaTime) / 2.f);
            if (!m_networkSpawnerEnabled)
                NetworkSpawnerControllerRequestBus::BroadcastResult(
                    m_networkSpawnerEnabled, &NetworkSpawnerControllerRequestBus::Events::GetEnabled);
            ProcessInput(((deltaTime + m_prevNetworkSpawnerDeltaTime) / 2.f), 2);
        }
    }

    void SpawnerComponent::OnNetworkTickFinish(const float& deltaTime, const bool& server, const AZ::EntityId& entity)
    {
        if ((!m_isAutonomousClient && !m_isServer && !m_isHost) || (entity != GetEntityId()))
            return;
        if (!((m_isHost && server) || (m_isServer && !server)))
            SpawnerComponentNotificationBus::Broadcast(
                &SpawnerComponentNotificationBus::Events::OnNetworkSpawnerTickFinish, (deltaTime + m_prevNetworkSpawnerDeltaTime) / 2.f);
        m_prevNetworkSpawnerDeltaTime = deltaTime;
    }

    void SpawnerComponent::GetNetworkSpawnerProperties()
    {
        if (m_networkSpawnerControllerObject != nullptr)
        {
        }
    }

    void SpawnerComponent::SetNetworkSpawnerProperties() const
    {
        if (m_networkSpawnerControllerObject != nullptr)
        {
        }
    }

    // Frame tick == 0, physics fixed timestep == 1 (not used in Spawner), network tick == 2
    void SpawnerComponent::ProcessInput([[maybe_unused]] const float& deltaTime, const AZ::u8& tickTimestepNetwork)
    {
        if (tickTimestepNetwork == 2)
        {
            GetNetworkSpawnerProperties();
            SetNetworkSpawnerProperties();
        }
    }

    // Event Notification methods for use in scripts
    void SpawnerComponent::OnNetworkSpawnerTickStart([[maybe_unused]] const float& deltaTime)
    {
    }
    void SpawnerComponent::OnNetworkSpawnerTickFinish([[maybe_unused]] const float& deltaTime)
    {
    }

    // Request Bus getter and setter methods for use in scripts
    bool SpawnerComponent::SpawnPrefabName(const AZStd::string& prefabName, const AZ::Transform& transform)
    {
        AZ::Data::AssetId prefabAssetId;
        if (!prefabName.empty())
        {
            // Make the prefab name that was passed in all lowercase
            AZStd::string prefabNameLower = prefabName;
            std::transform(
                prefabNameLower.begin(),
                prefabNameLower.end(),
                prefabNameLower.begin(),
                [](unsigned char c)
                {
                    return std::tolower(c);
                });

            // Find the first spawnable asset with the name provided
            auto pathIt = AZStd::find_if(
                m_spawnablePaths.begin(),
                m_spawnablePaths.end(),
                [&](const AZStd::string& path)
                {
                    return path.find(prefabNameLower + ".spawnable") != std::string::npos;
                });
            AZStd::string prefabPath;
            if (pathIt != m_spawnablePaths.end())
                prefabPath = *pathIt;
            else
            {
                AZ_Error("SpawnerComponent", false, "Asset %s.spawnable not found.", prefabName.c_str());
                return false;
            }

            // Get the asset Id
            AZ::Data::AssetCatalogRequestBus::BroadcastResult(
                prefabAssetId,
                &AZ::Data::AssetCatalogRequestBus::Events::GetAssetIdByPath,
                prefabPath.c_str(),
                AZ::Data::s_invalidAssetType,
                false);
            if (!prefabAssetId.IsValid())
            {
                AZ_Error("SpawnerComponent", false, "Asset '%s' is invalid.", prefabPath.c_str());
                return false;
            }
        }
        else
        {
            AZ_Error("SpawnerComponent", false, "No prefab name provided.");
            return false;
        }

        if (prefabAssetId.IsValid())
        {
            AZ::Data::AssetInfo prefabAssetInfo;
            AZ::Data::AssetCatalogRequestBus::BroadcastResult(
                prefabAssetInfo, &AZ::Data::AssetCatalogRequestBus::Events::GetAssetInfoById, prefabAssetId);
            AZ::Data::Asset<AzFramework::Spawnable> spawnableAsset;
            if (prefabAssetInfo.m_assetType == spawnableAsset.GetType())
            {
                spawnableAsset.Create(prefabAssetId, false);
                spawnableAsset.QueueLoad();
                // AZ::Data::AssetBus::MultiHandler::BusConnect(spawnableAsset.GetId());
                AzFramework::EntitySpawnTicket* ticket = aznew AzFramework::EntitySpawnTicket(spawnableAsset);
                if (ticket->IsValid())
                {
                    // Create a callback for SpawnAllEntities that will set the transform of the root entity to
                    // the correct translation, rotation, and scale for the spawned instance.
                    auto preSpawnCB =
                        [transform](
                            [[maybe_unused]] AzFramework::EntitySpawnTicket::Id ticketId, AzFramework::SpawnableEntityContainerView view)
                    {
                        AZ::Entity* rootEntity = *view.begin();

                        AzFramework::TransformComponent* entityTransform = rootEntity->FindComponent<AzFramework::TransformComponent>();

                        if (entityTransform)
                            entityTransform->SetWorldTM(transform);
                    };

                    m_spawnTickets.emplace(ticket);

                    AzFramework::SpawnAllEntitiesOptionalArgs optionalArgs;
                    optionalArgs.m_preInsertionCallback = AZStd::move(preSpawnCB);
                    AzFramework::SpawnableEntitiesInterface::Get()->SpawnAllEntities(*ticket, AZStd::move(optionalArgs));

                    // Set the ticketId as the one which was most recently spawned, to be tracked outside this component
                    m_lastSpawnedTicketId = ticket->GetId();
                    if (m_networkSpawnerControllerObject != nullptr)
                        m_networkSpawnerControllerObject->SetLastSpawnedTicketId(m_lastSpawnedTicketId);

                    return true;
                }
                else
                {
                    // Something went wrong
                    AZ_Assert(ticket->IsValid(), "Unable to instantiate spawnable asset");
                    delete ticket;
                    return false;
                }
            }
            else
            {
                AZ_Error(
                    "SpawnerComponent",
                    false,
                    "Asset '%s' is of type %s, but expected a Spawnable type.",
                    prefabAssetId.ToString<AZStd::string>().c_str(),
                    prefabAssetInfo.m_assetType.ToString<AZStd::string>().c_str());
                return false;
            }
        }
        else
        {
            AZ_Error("SpawnerComponent", false, "Invalid prefab asset Id.");
            return false;
        }
    }
    bool SpawnerComponent::DepawnPrefabTicketId(const uint32_t& prefabTicketId)
    {
        for (auto ticket : m_spawnTickets)
        {
            if (prefabTicketId == ticket->GetId())
            {
                AzFramework::SpawnableEntitiesInterface::Get()->DespawnAllEntities(*ticket);
                m_spawnTickets.erase(ticket);
                m_lastDespawnedTicketId = prefabTicketId;
                if (m_networkSpawnerControllerObject != nullptr)
                    m_networkSpawnerControllerObject->SetLastDespawnedTicketId(m_lastDespawnedTicketId);
                return true;
            }
        }

        AZ_Error("SpawnerComponent", false, "Failed to find %lu prefab ticketId for despawning.", prefabTicketId);
        return false;
    }
    uint32_t SpawnerComponent::GetLastSpawnedTicketId()
    {
        if (m_networkSpawnerControllerObject != nullptr)
            m_lastSpawnedTicketId = m_networkSpawnerControllerObject->GetLastSpawnedTicketId();
        return m_lastSpawnedTicketId;
    }
    void SpawnerComponent::SetLastSpawnedTicketId(const uint32_t& new_lastSpawnedTicketId)
    {
        m_lastSpawnedTicketId = new_lastSpawnedTicketId;
        if (m_networkSpawnerControllerObject != nullptr)
            m_networkSpawnerControllerObject->SetLastSpawnedTicketId(m_lastSpawnedTicketId);
    }
    uint32_t SpawnerComponent::GetLastDespawnedTicketId()
    {
        if (m_networkSpawnerControllerObject != nullptr)
            m_lastDespawnedTicketId = m_networkSpawnerControllerObject->GetLastDespawnedTicketId();
        return m_lastDespawnedTicketId;
    }
    void SpawnerComponent::SetLastDespawnedTicketId(const uint32_t& new_lastDespawnedTicketId)
    {
        m_lastDespawnedTicketId = new_lastDespawnedTicketId;
        if (m_networkSpawnerControllerObject != nullptr)
            m_networkSpawnerControllerObject->SetLastDespawnedTicketId(m_lastDespawnedTicketId);
    }
    bool SpawnerComponent::GetIsAutonomousClient() const
    {
        return m_isAutonomousClient;
    }
    bool SpawnerComponent::GetIsServer() const
    {
        return m_isServer;
    }
    bool SpawnerComponent::GetIsHost() const
    {
        return m_isHost;
    }
    bool SpawnerComponent::GetLocallyEnableNetworkSpawner() const
    {
        return m_networkSpawnerEnabled;
    }
    void SpawnerComponent::SetLocallyEnableNetworkSpawner(const bool& new_networkSpawnerEnabled)
    {
        m_networkSpawnerEnabled = new_networkSpawnerEnabled;
        NetworkSpawnerControllerRequestBus::Event(
            GetEntityId(), &NetworkSpawnerControllerRequestBus::Events::SetEnabled, m_networkSpawnerEnabled);
    }
    void SpawnerComponent::IsAutonomousSoConnect()
    {
        AZ::TickBus::Handler::BusConnect();
    }
    void SpawnerComponent::NotAutonomousSoDisconnect()
    {
        AZ::TickBus::Handler::BusDisconnect();
    }
} // namespace Hermes
