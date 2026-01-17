#pragma once

#include <AzCore/Component/ComponentBus.h>
#include <AzCore/RTTI/BehaviorContext.h>

namespace Hermes
{
    class NetworkHermesControllerRequests : public AZ::ComponentBus
    {
    public:
        ~NetworkHermesControllerRequests() override = default;

        virtual bool GetEnabled() const = 0;
        virtual void SetEnabled(const bool&) = 0;
    };

    using NetworkHermesControllerRequestBus = AZ::EBus<NetworkHermesControllerRequests>;

    class NetworkHermesControllerNotifications : public AZ::ComponentBus
    {
    public:
        virtual void OnNetworkTickStart(const float&, const bool&, const AZ::EntityId&) = 0;
        virtual void OnNetworkTickFinish(const float&, const bool&, const AZ::EntityId&) = 0;
    };

    using NetworkHermesControllerNotificationBus = AZ::EBus<NetworkHermesControllerNotifications>;

    class NetworkHermesControllerNotificationHandler
        : public NetworkHermesControllerNotificationBus::Handler
        , public AZ::BehaviorEBusHandler
    {
    public:
        AZ_EBUS_BEHAVIOR_BINDER(
            NetworkHermesControllerNotificationHandler,
            "{d7479423-1ba2-4d70-abc7-882c97ade7c6}",
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
