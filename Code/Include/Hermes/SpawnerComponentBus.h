#pragma once

#include <AzCore/Component/ComponentBus.h>
#include <AzCore/RTTI/BehaviorContext.h>
#include <Multiplayer/MultiplayerTypes.h>

#if __has_include(<FirstPersonController/FirstPersonControllerComponentBus.h>)
#include <FirstPersonController/FirstPersonControllerComponentBus.h>
#endif

namespace Hermes
{
    class SpawnerComponentRequests : public AZ::ComponentBus
    {
    public:
        ~SpawnerComponentRequests() override = default;

        virtual bool SpawnPrefabName(const AZStd::string&, const AZ::Transform&, const bool) = 0;
        virtual bool SpawnPrefabPath(const AZStd::string&, const AZ::Transform&, const bool) = 0;
        virtual bool SpawnNetPrefabName(const AZStd::string&, const AZ::Transform&) = 0;
        virtual bool SpawnNetPrefabPath(const AZStd::string&, const AZ::Transform&) = 0;
        virtual bool DepawnPrefabTicketId(const uint32_t) = 0;
        virtual bool DepawnNetEntityIdAndChildren(const AZStd::string&) = 0;
        virtual AZStd::pair<uint32_t, AZStd::string> GetLastSpawnedTicketIdByStringNetEntityId() = 0;
        virtual void SetLastSpawnedTicketIdByStringNetEntityId(const AZStd::pair<uint32_t, AZStd::string>&) = 0;
        virtual uint32_t GetLastSpawnedTicketId() = 0;
        virtual void SetLastSpawnedTicketId(const uint32_t) = 0;
        virtual AZStd::vector<AZStd::string> GetLastSpawnedStringNetEntityIds() = 0;
        virtual void SetLastSpawnedStringNetEntityIds(const AZStd::vector<AZStd::string>&) = 0;
        virtual uint32_t GetLastDespawnedTicketId() = 0;
        virtual void SetLastDespawnedTicketId(const uint32_t) = 0;
        virtual bool GetIsAutonomousClient() const = 0;
        virtual bool GetIsServer() const = 0;
        virtual bool GetIsHost() const = 0;
        virtual bool GetLocallyEnableNetworkSpawner() const = 0;
        virtual void SetLocallyEnableNetworkSpawner(const bool new_networkSpawnerEnabled) = 0;
        virtual void IsAutonomousSoConnect() = 0;
        virtual void NotAutonomousSoDisconnect() = 0;
    };

    using SpawnerComponentRequestBus = AZ::EBus<SpawnerComponentRequests>;

    class SpawnerComponentNotifications : public AZ::ComponentBus
    {
    public:
        virtual void OnNetworkSpawnerTickStart(const float) = 0;
        virtual void OnNetworkSpawnerTickFinish(const float) = 0;
    };

    using SpawnerComponentNotificationBus = AZ::EBus<SpawnerComponentNotifications>;

    class SpawnerComponentNotificationHandler
        : public SpawnerComponentNotificationBus::Handler
        , public AZ::BehaviorEBusHandler
    {
    public:
        AZ_EBUS_BEHAVIOR_BINDER(
            SpawnerComponentNotificationHandler,
            "{49da4e36-c920-4616-8abb-a8ac0636160b}",
            AZ::SystemAllocator,
            OnNetworkSpawnerTickStart,
            OnNetworkSpawnerTickFinish);

        void OnNetworkSpawnerTickStart(const float deltaTime) override
        {
            Call(FN_OnNetworkSpawnerTickStart, deltaTime);
        }
        void OnNetworkSpawnerTickFinish(const float deltaTime) override
        {
            Call(FN_OnNetworkSpawnerTickFinish, deltaTime);
        }
    };
} // namespace Hermes
