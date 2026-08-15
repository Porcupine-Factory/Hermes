#pragma once

#include <AzCore/Component/ComponentBus.h>
#include <AzCore/RTTI/BehaviorContext.h>

namespace Hermes
{
    class NetworkExampleControllerRequests : public AZ::ComponentBus
    {
    public:
        ~NetworkExampleControllerRequests() override = default;

        virtual bool GetEnabled() const = 0;
        virtual void SetEnabled(const bool) = 0;
    };

    using NetworkExampleControllerRequestBus = AZ::EBus<NetworkExampleControllerRequests>;

    class NetworkExampleControllerNotifications : public AZ::ComponentBus
    {
    public:
        virtual void OnNetworkTickStart(const float, const bool, const AZ::EntityId&) = 0;
        virtual void OnNetworkTickFinish(const float, const bool, const AZ::EntityId&) = 0;
    };

    using NetworkExampleControllerNotificationBus = AZ::EBus<NetworkExampleControllerNotifications>;

    class NetworkExampleControllerNotificationHandler
        : public NetworkExampleControllerNotificationBus::Handler
        , public AZ::BehaviorEBusHandler
    {
    public:
        AZ_EBUS_BEHAVIOR_BINDER(
            NetworkExampleControllerNotificationHandler,
            "{networkcontrollerbusuuid}",
            AZ::SystemAllocator,
            OnNetworkTickStart,
            OnNetworkTickFinish);

        void OnNetworkTickStart(const float deltaTime, const bool server, const AZ::EntityId& entityId) override
        {
            Call(FN_OnNetworkTickStart, deltaTime, server, entityId);
        }
        void OnNetworkTickFinish(const float deltaTime, const bool server, const AZ::EntityId& entityId) override
        {
            Call(FN_OnNetworkTickFinish, deltaTime, server, entityId);
        }
    };
} // namespace Hermes
