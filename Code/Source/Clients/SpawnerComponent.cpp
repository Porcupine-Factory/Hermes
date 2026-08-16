#include <AzCore/Math/MathStringConversions.h>
#include <Clients/SpawnerComponent.h>

#include <AzCore/Asset/AssetCommon.h>
#include <AzCore/Asset/AssetManager.h>
#include <AzCore/Component/ComponentApplicationBus.h>
#include <AzCore/Component/Entity.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/std/string/regex.h>
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
                ->Attribute(AZ::Script::Attributes::Category, "Spawner")
                ->Event("Spawn Prefab Name", &SpawnerComponentRequests::SpawnPrefabName)
                ->Event("Spawn Prefab Path", &SpawnerComponentRequests::SpawnPrefabPath)
                ->Event("Spawn Net Prefab Name", &SpawnerComponentRequests::SpawnNetPrefabName)
                ->Event("Spawn Net Prefab Path", &SpawnerComponentRequests::SpawnNetPrefabPath)
                ->Event("Despawn Prefab TicketId", &SpawnerComponentRequests::DepawnPrefabTicketId)
                ->Event("Despawn NetEntityId And Children", &SpawnerComponentRequests::DepawnNetEntityIdAndChildren)
                ->Event(
                    "Get Last Spawned TicketId By NetEntityId String", &SpawnerComponentRequests::GetLastSpawnedTicketIdByStringNetEntityId)
                ->Event(
                    "Set Last Spawned TicketId By NetEntityId String", &SpawnerComponentRequests::SetLastSpawnedTicketIdByStringNetEntityId)
                ->Event("Get Last Spawned TicketId", &SpawnerComponentRequests::GetLastSpawnedTicketId)
                ->Event("Set Last Spawned TicketId", &SpawnerComponentRequests::SetLastSpawnedTicketId)
                ->Event("Get Last Spawned NetEntityId Strings", &SpawnerComponentRequests::GetLastSpawnedStringNetEntityIds)
                ->Event("Set Last Spawned NetEntityId Strings", &SpawnerComponentRequests::SetLastSpawnedStringNetEntityIds)
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
        SpawnerComponentRequestBus::Handler::BusDisconnect();
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
        ProcessInput(deltaTime, 0);
    }

    void SpawnerComponent::OnNetworkTickStart(const float deltaTime, const bool server, const AZ::EntityId& entityId)
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
            SpawnerComponentNotificationBus::Broadcast(&SpawnerComponentNotificationBus::Events::OnNetworkSpawnerTickStart, deltaTime);
            if (!m_networkSpawnerEnabled)
                NetworkSpawnerControllerRequestBus::BroadcastResult(
                    m_networkSpawnerEnabled, &NetworkSpawnerControllerRequestBus::Events::GetEnabled);
            ProcessInput(deltaTime, 2);
        }
    }

    void SpawnerComponent::OnNetworkTickFinish(const float deltaTime, const bool server, const AZ::EntityId& entityId)
    {
        if ((!m_isAutonomousClient && !m_isServer && !m_isHost) || (entityId != GetEntityId()))
            return;
        if (!((m_isHost && server) || (m_isServer && !server)))
            SpawnerComponentNotificationBus::Broadcast(&SpawnerComponentNotificationBus::Events::OnNetworkSpawnerTickFinish, deltaTime);
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
    void SpawnerComponent::ProcessInput([[maybe_unused]] const float deltaTime, const AZ::u8 tickTimestepNetwork)
    {
        if (tickTimestepNetwork == 2)
        {
            GetNetworkSpawnerProperties();
            SetNetworkSpawnerProperties();
        }
    }

    // Request Bus getter and setter methods for use in scripts
    bool SpawnerComponent::SpawnPrefabName(const AZStd::string& prefabName, const AZ::Transform& transform, const bool makeChild)
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
                    // Clear the previous list of entity Ids that were spawned
                    m_lastSpawnedStringNetEntityIds.clear();

                    // Create a callback for SpawnAllEntities that will set the transform of the root entity to
                    // the correct translation, rotation, and scale for the spawned instance.
                    auto preSpawnCB =
                        [this, transform, makeChild](
                            [[maybe_unused]] AzFramework::EntitySpawnTicket::Id ticketId, AzFramework::SpawnableEntityContainerView view)
                    {
                        AZ::Entity* rootEntity = *view.begin();

                        for (AZ::Entity* entity : view)
                        {
                            const Multiplayer::INetworkEntityManager* networkEntityManager =
                                Multiplayer::GetMultiplayer()->GetNetworkEntityManager();
                            m_lastSpawnedStringNetEntityIds.push_back(
                                AZStd::to_string(networkEntityManager->GetNetEntityIdById(entity->GetId())));
                        }
                        m_networkSpawnerControllerObject->SetLastSpawnedStringNetEntityIds(m_lastSpawnedStringNetEntityIds);

                        AzFramework::TransformComponent* entityTransform = rootEntity->FindComponent<AzFramework::TransformComponent>();

                        AzFramework::TransformComponentConfiguration transformConfig;
                        // Make the parent of the spawned prefab the entity with the spawner component if instructed
                        if (makeChild)
                        {
                            transformConfig.m_parentId = GetEntityId();
                            transformConfig.m_localTransform = transform;
                        }
                        else
                            transformConfig.m_worldTransform = transform;
                        entityTransform->SetConfiguration(transformConfig);

                        if (entityTransform)
                            entityTransform->SetConfiguration(transformConfig);
                    };

                    m_spawnTickets.emplace(ticket);

                    AzFramework::SpawnAllEntitiesOptionalArgs optionalArgs;
                    optionalArgs.m_preInsertionCallback = AZStd::move(preSpawnCB);
                    AzFramework::SpawnableEntitiesInterface::Get()->SpawnAllEntities(*ticket, AZStd::move(optionalArgs));

                    // Set the TicketId and EntityIds as the most recently spawned, to be tracked outside this component
                    m_lastSpawnedTicketId = ticket->GetId();
                    if (m_networkSpawnerControllerObject != nullptr)
                    {
                        m_networkSpawnerControllerObject->SetLastSpawnedTicketId(m_lastSpawnedTicketId);
                        const Multiplayer::INetworkEntityManager* networkEntityManager =
                            Multiplayer::GetMultiplayer()->GetNetworkEntityManager();
                        m_networkSpawnerControllerObject->SetNetEntityIdStringCausedLastSpawn(
                            AZStd::to_string(networkEntityManager->GetNetEntityIdById(GetEntityId())));
                        m_lastSpawnedTicketIdByStringNetEntityId = AZStd::pair<uint32_t, AZStd::string>(
                            m_lastSpawnedTicketId, AZStd::to_string(networkEntityManager->GetNetEntityIdById(GetEntityId())));
                        m_networkSpawnerControllerObject->SetLastSpawnedTicketIdByStringNetEntityId(
                            m_lastSpawnedTicketIdByStringNetEntityId);
                    }

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
    bool SpawnerComponent::SpawnPrefabPath(const AZStd::string& prefabPath, const AZ::Transform& transform, const bool makeChild)
    {
        AZ::Data::AssetId prefabAssetId;
        if (!prefabPath.empty())
        {
            // Make the prefab path that was passed in all lowercase
            AZStd::string prefabPathLower = prefabPath;
            std::transform(
                prefabPathLower.begin(),
                prefabPathLower.end(),
                prefabPathLower.begin(),
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
                    return path.find(prefabPathLower) != std::string::npos;
                });
            AZStd::string foundPrefabPath;
            if (pathIt != m_spawnablePaths.end())
                foundPrefabPath = *pathIt;
            else
            {
                AZ_Error("SpawnerComponent", false, "Asset %s.spawnable not found.", prefabPath.c_str());
                return false;
            }

            // Get the asset Id
            AZ::Data::AssetCatalogRequestBus::BroadcastResult(
                prefabAssetId,
                &AZ::Data::AssetCatalogRequestBus::Events::GetAssetIdByPath,
                foundPrefabPath.c_str(),
                AZ::Data::s_invalidAssetType,
                false);
            if (!prefabAssetId.IsValid())
            {
                AZ_Error("SpawnerComponent", false, "Asset '%s' is invalid.", foundPrefabPath.c_str());
                return false;
            }
        }
        else
        {
            AZ_Error("SpawnerComponent", false, "No prefab path provided.");
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
                    // Clear the previous list of entity Ids that were spawned
                    m_lastSpawnedStringNetEntityIds.clear();

                    // Create a callback for SpawnAllEntities that will set the transform of the root entity to
                    // the correct translation, rotation, and scale for the spawned instance.
                    auto preSpawnCB =
                        [this, transform, makeChild](
                            [[maybe_unused]] AzFramework::EntitySpawnTicket::Id ticketId, AzFramework::SpawnableEntityContainerView view)
                    {
                        AZ::Entity* rootEntity = *view.begin();

                        for (AZ::Entity* entity : view)
                        {
                            const Multiplayer::INetworkEntityManager* networkEntityManager =
                                Multiplayer::GetMultiplayer()->GetNetworkEntityManager();
                            m_lastSpawnedStringNetEntityIds.push_back(
                                AZStd::to_string(networkEntityManager->GetNetEntityIdById(entity->GetId())));
                        }
                        m_networkSpawnerControllerObject->SetLastSpawnedStringNetEntityIds(m_lastSpawnedStringNetEntityIds);

                        AzFramework::TransformComponent* entityTransform = rootEntity->FindComponent<AzFramework::TransformComponent>();

                        AzFramework::TransformComponentConfiguration transformConfig;
                        // Make the parent of the spawned prefab the entity with the spawner component if instructed
                        if (makeChild)
                        {
                            transformConfig.m_parentId = GetEntityId();
                            transformConfig.m_localTransform = transform;
                        }
                        else
                            transformConfig.m_worldTransform = transform;
                        entityTransform->SetConfiguration(transformConfig);

                        if (entityTransform)
                            entityTransform->SetConfiguration(transformConfig);
                    };

                    m_spawnTickets.emplace(ticket);

                    AzFramework::SpawnAllEntitiesOptionalArgs optionalArgs;
                    optionalArgs.m_preInsertionCallback = AZStd::move(preSpawnCB);
                    AzFramework::SpawnableEntitiesInterface::Get()->SpawnAllEntities(*ticket, AZStd::move(optionalArgs));

                    // Set the TicketId and EntityIds as the most recently spawned, to be tracked outside this component
                    m_lastSpawnedTicketId = ticket->GetId();
                    if (m_networkSpawnerControllerObject != nullptr)
                    {
                        m_networkSpawnerControllerObject->SetLastSpawnedTicketId(m_lastSpawnedTicketId);
                        const Multiplayer::INetworkEntityManager* networkEntityManager =
                            Multiplayer::GetMultiplayer()->GetNetworkEntityManager();
                        m_networkSpawnerControllerObject->SetNetEntityIdStringCausedLastSpawn(
                            AZStd::to_string(networkEntityManager->GetNetEntityIdById(GetEntityId())));
                        m_lastSpawnedTicketIdByStringNetEntityId = AZStd::pair<uint32_t, AZStd::string>(
                            m_lastSpawnedTicketId, AZStd::to_string(networkEntityManager->GetNetEntityIdById(GetEntityId())));
                        m_networkSpawnerControllerObject->SetLastSpawnedTicketIdByStringNetEntityId(
                            m_lastSpawnedTicketIdByStringNetEntityId);
                    }

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
    bool SpawnerComponent::SpawnNetPrefabName(const AZStd::string& prefabName, const AZ::Transform& transform)
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
                const Multiplayer::PrefabEntityId prefabEntityId(AZ::Name(spawnableAsset.GetHint().c_str()));
                Multiplayer::INetworkEntityManager* networkEntityManager = Multiplayer::GetMultiplayer()->GetNetworkEntityManager();
                Multiplayer::INetworkEntityManager::EntityList entityList =
                    networkEntityManager->CreateEntitiesImmediate(prefabEntityId, Multiplayer::NetEntityRole::Authority, transform);
                m_lastSpawnedStringNetEntityIds.clear();
                for (auto entity : entityList)
                    m_lastSpawnedStringNetEntityIds.push_back(AZStd::to_string(entity.GetNetEntityId()));

                m_netEntityHandles.push_back(entityList[0]);

                // There aren't spawn tickets when using CreateEntitiesImmediate(...), so just leaving this here for reference
                // m_lastSpawnedTicketId = entityList.front().GetEntity()->GetEntitySpawnTicketId();
                // AzFramework::SpawnableEntitiesInterface::Get()->RetrieveTicket(
                //    m_lastSpawnedTicketId,
                //    [this](AzFramework::EntitySpawnTicket&& ticket)
                //    {
                //        m_spawnTickets.emplace(*ticket);
                //    });

                if (m_networkSpawnerControllerObject != nullptr)
                {
                    m_networkSpawnerControllerObject->SetLastSpawnedStringNetEntityIds(m_lastSpawnedStringNetEntityIds);
                    networkEntityManager = Multiplayer::GetMultiplayer()->GetNetworkEntityManager();
                    m_networkSpawnerControllerObject->SetNetEntityIdStringCausedLastSpawn(
                        AZStd::to_string(networkEntityManager->GetNetEntityIdById(GetEntityId())));
                    // There aren't spawn tickets when using CreateEntitiesImmediate(...), so just leaving this here for reference
                    // m_networkSpawnerControllerObject->SetLastSpawnedTicketId(m_lastSpawnedTicketId);
                    // m_lastSpawnedTicketIdByStringNetEntityId = AZStd::pair<uint32_t, AZStd::string>(
                    //    m_lastSpawnedTicketId, AZStd::to_string(networkEntityManager->GetNetEntityIdById(GetEntityId())));
                    // m_networkSpawnerControllerObject->SetLastSpawnedTicketIdByStringNetEntityId(
                    //    m_lastSpawnedTicketIdByStringNetEntityId);
                }
                return true;
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
    bool SpawnerComponent::SpawnNetPrefabPath(const AZStd::string& prefabPath, const AZ::Transform& transform)
    {
        AZ::Data::AssetId prefabAssetId;
        if (!prefabPath.empty())
        {
            // Make the prefab path that was passed in all lowercase
            AZStd::string prefabPathLower = prefabPath;
            std::transform(
                prefabPathLower.begin(),
                prefabPathLower.end(),
                prefabPathLower.begin(),
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
                    return path.find(prefabPathLower) != std::string::npos;
                });
            AZStd::string foundPrefabPath;
            if (pathIt != m_spawnablePaths.end())
                foundPrefabPath = *pathIt;
            else
            {
                AZ_Error("SpawnerComponent", false, "Asset %s.spawnable not found.", prefabPath.c_str());
                return false;
            }

            foundPrefabPath = AZStd::regex_replace(foundPrefabPath, AZStd::regex("\\.spawnable"), ".network.spawnable");

            // Get the asset Id
            AZ::Data::AssetCatalogRequestBus::BroadcastResult(
                prefabAssetId,
                &AZ::Data::AssetCatalogRequestBus::Events::GetAssetIdByPath,
                foundPrefabPath.c_str(),
                AZ::Data::s_invalidAssetType,
                false);
            if (!prefabAssetId.IsValid())
            {
                AZ_Error("SpawnerComponent", false, "Asset '%s' is invalid.", foundPrefabPath.c_str());
                return false;
            }
        }
        else
        {
            AZ_Error("SpawnerComponent", false, "No prefab path provided.");
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
                const Multiplayer::PrefabEntityId prefabEntityId(AZ::Name(spawnableAsset.GetHint().c_str()));
                Multiplayer::INetworkEntityManager* networkEntityManager = Multiplayer::GetMultiplayer()->GetNetworkEntityManager();
                Multiplayer::INetworkEntityManager::EntityList entityList =
                    networkEntityManager->CreateEntitiesImmediate(prefabEntityId, Multiplayer::NetEntityRole::Authority, transform);
                m_lastSpawnedStringNetEntityIds.clear();
                for (auto entity : entityList)
                    m_lastSpawnedStringNetEntityIds.push_back(AZStd::to_string(entity.GetNetEntityId()));

                m_netEntityHandles.push_back(entityList[0]);

                // There aren't spawn tickets when using CreateEntitiesImmediate(...), so just leaving this here for reference
                // m_lastSpawnedTicketId = entityList.front().GetEntity()->GetEntitySpawnTicketId();
                // AzFramework::SpawnableEntitiesInterface::Get()->RetrieveTicket(
                //    m_lastSpawnedTicketId,
                //    [this](AzFramework::EntitySpawnTicket&& ticket)
                //    {
                //        m_spawnTickets.emplace(*ticket);
                //    });

                if (m_networkSpawnerControllerObject != nullptr)
                {
                    m_networkSpawnerControllerObject->SetLastSpawnedStringNetEntityIds(m_lastSpawnedStringNetEntityIds);
                    networkEntityManager = Multiplayer::GetMultiplayer()->GetNetworkEntityManager();
                    m_networkSpawnerControllerObject->SetNetEntityIdStringCausedLastSpawn(
                        AZStd::to_string(networkEntityManager->GetNetEntityIdById(GetEntityId())));
                    // There aren't spawn tickets when using CreateEntitiesImmediate(...), so just leaving this here for reference
                    // m_networkSpawnerControllerObject->SetLastSpawnedTicketId(m_lastSpawnedTicketId);
                    // m_lastSpawnedTicketIdByStringNetEntityId = AZStd::pair<uint32_t, AZStd::string>(
                    //    m_lastSpawnedTicketId, AZStd::to_string(networkEntityManager->GetNetEntityIdById(GetEntityId())));
                    // m_networkSpawnerControllerObject->SetLastSpawnedTicketIdByStringNetEntityId(
                    //    m_lastSpawnedTicketIdByStringNetEntityId);
                }
                return true;
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
    bool SpawnerComponent::DepawnPrefabTicketId(const uint32_t prefabTicketId)
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
    bool SpawnerComponent::DepawnNetEntityIdAndChildren(const AZStd::string& netEntityId)
    {
        Multiplayer::INetworkEntityManager* networkEntityManager = Multiplayer::GetMultiplayer()->GetNetworkEntityManager();
        for (auto netEntityHandleIt = m_netEntityHandles.begin(); netEntityHandleIt != m_netEntityHandles.end(); ++netEntityHandleIt)
        {
            if (AZStd::to_string(netEntityHandleIt->GetNetEntityId()) == netEntityId)
            {
                AZStd::vector<AZ::EntityId> children;
                AZ::TransformBus::EventResult(children, netEntityHandleIt->GetEntity()->GetId(), &AZ::TransformBus::Events::GetChildren);
                for (auto child : children)
                    networkEntityManager->MarkForRemoval(networkEntityManager->GetEntity(networkEntityManager->GetNetEntityIdById(child)));

                networkEntityManager->MarkForRemoval(*netEntityHandleIt);
                m_netEntityHandles.erase(netEntityHandleIt);
                return true;
            }
        }

        AZ_Error("SpawnerComponent", false, "Failed to find %s NetEntityId for despawning.", netEntityId.c_str());
        return false;
    }
    AZStd::pair<uint32_t, AZStd::string> SpawnerComponent::GetLastSpawnedTicketIdByStringNetEntityId()
    {
        if (m_networkSpawnerControllerObject != nullptr)
            m_lastSpawnedTicketIdByStringNetEntityId = m_networkSpawnerControllerObject->GetLastSpawnedTicketIdByStringNetEntityId();
        return m_lastSpawnedTicketIdByStringNetEntityId;
    }
    void SpawnerComponent::SetLastSpawnedTicketIdByStringNetEntityId(
        const AZStd::pair<uint32_t, AZStd::string>& lastSpawnedTicketIdByStringNetEntityId)
    {
        m_lastSpawnedTicketIdByStringNetEntityId = lastSpawnedTicketIdByStringNetEntityId;
        if (m_networkSpawnerControllerObject != nullptr)
            m_networkSpawnerControllerObject->SetLastSpawnedTicketIdByStringNetEntityId(m_lastSpawnedTicketIdByStringNetEntityId);
    }
    uint32_t SpawnerComponent::GetLastSpawnedTicketId()
    {
        if (m_networkSpawnerControllerObject != nullptr)
            m_lastSpawnedTicketId = m_networkSpawnerControllerObject->GetLastSpawnedTicketId();
        return m_lastSpawnedTicketId;
    }
    void SpawnerComponent::SetLastSpawnedTicketId(const uint32_t lastSpawnedTicketId)
    {
        m_lastSpawnedTicketId = lastSpawnedTicketId;
        if (m_networkSpawnerControllerObject != nullptr)
            m_networkSpawnerControllerObject->SetLastSpawnedTicketId(m_lastSpawnedTicketId);
    }
    AZStd::vector<AZStd::string> SpawnerComponent::GetLastSpawnedStringNetEntityIds()
    {
        if (m_networkSpawnerControllerObject != nullptr)
            m_lastSpawnedStringNetEntityIds = m_networkSpawnerControllerObject->GetLastSpawnedStringNetEntityIds();
        return m_lastSpawnedStringNetEntityIds;
    }
    void SpawnerComponent::SetLastSpawnedStringNetEntityIds(const AZStd::vector<AZStd::string>& lastSpawnedStringNetEntityIds)
    {
        m_lastSpawnedStringNetEntityIds = lastSpawnedStringNetEntityIds;
        if (m_networkSpawnerControllerObject != nullptr)
            m_networkSpawnerControllerObject->SetLastSpawnedStringNetEntityIds(m_lastSpawnedStringNetEntityIds);
    }
    uint32_t SpawnerComponent::GetLastDespawnedTicketId()
    {
        if (m_networkSpawnerControllerObject != nullptr)
            m_lastDespawnedTicketId = m_networkSpawnerControllerObject->GetLastDespawnedTicketId();
        return m_lastDespawnedTicketId;
    }
    void SpawnerComponent::SetLastDespawnedTicketId(const uint32_t lastDespawnedTicketId)
    {
        m_lastDespawnedTicketId = lastDespawnedTicketId;
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
    void SpawnerComponent::SetLocallyEnableNetworkSpawner(const bool networkSpawnerEnabled)
    {
        m_networkSpawnerEnabled = networkSpawnerEnabled;
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
