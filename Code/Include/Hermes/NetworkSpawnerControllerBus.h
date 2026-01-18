#pragma once

#include <AzCore/Component/ComponentBus.h>
#include <AzCore/RTTI/BehaviorContext.h>

namespace Hermes
{
    class NetworkSpawnerControllerRequests : public AZ::ComponentBus
    {
    public:
        ~NetworkSpawnerControllerRequests() override = default;

        virtual bool GetEnabled() const = 0;
        virtual void SetEnabled(const bool&) = 0;
    };

    using NetworkSpawnerControllerRequestBus = AZ::EBus<NetworkSpawnerControllerRequests>;

    class NetworkSpawnerControllerNotifications : public AZ::ComponentBus
    {
    public:
        virtual void OnNetworkTickStart(const float&, const bool&, const AZ::EntityId&) = 0;
        virtual void OnNetworkTickFinish(const float&, const bool&, const AZ::EntityId&) = 0;
    };

    using NetworkSpawnerControllerNotificationBus = AZ::EBus<NetworkSpawnerControllerNotifications>;

    class NetworkSpawnerControllerNotificationHandler
        : public NetworkSpawnerControllerNotificationBus::Handler
        , public AZ::BehaviorEBusHandler
    {
    public:
        AZ_EBUS_BEHAVIOR_BINDER(
            NetworkSpawnerControllerNotificationHandler,
            "{10093b48-c317-4ab8-945e-70cd14d3235f}",
            AZ::SystemAllocator,
            OnNetworkTickStart,
            OnNetworkTickFinish);

        void OnNetworkTickStart(
            [[maybe_unused]] const float& deltaTime,
            [[maybe_unused]] const bool& server,
            [[maybe_unused]] const AZ::EntityId& entity) override
        {
            Call(FN_OnNetworkTickStart);
        }
        void OnNetworkTickFinish(
            [[maybe_unused]] const float& deltaTime,
            [[maybe_unused]] const bool& server,
            [[maybe_unused]] const AZ::EntityId& entity) override
        {
            Call(FN_OnNetworkTickFinish);
        }
    };
} // namespace Hermes
