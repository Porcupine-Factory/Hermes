#pragma once

#include <AzCore/Component/ComponentBus.h>
#include <AzCore/RTTI/BehaviorContext.h>

namespace Hermes
{
    class NetworkCharacterNameControllerRequests : public AZ::ComponentBus
    {
    public:
        ~NetworkCharacterNameControllerRequests() override = default;

        virtual bool GetEnabled() const = 0;
        virtual void SetEnabled(const bool&) = 0;
    };

    using NetworkCharacterNameControllerRequestBus = AZ::EBus<NetworkCharacterNameControllerRequests>;

    class NetworkCharacterNameControllerNotifications : public AZ::ComponentBus
    {
    public:
        virtual void OnNetworkTickStart(const float&, const bool&, const AZ::EntityId&) = 0;
        virtual void OnNetworkTickFinish(const float&, const bool&, const AZ::EntityId&) = 0;
    };

    using NetworkCharacterNameControllerNotificationBus = AZ::EBus<NetworkCharacterNameControllerNotifications>;

    class NetworkCharacterNameControllerNotificationHandler
        : public NetworkCharacterNameControllerNotificationBus::Handler
        , public AZ::BehaviorEBusHandler
    {
    public:
        AZ_EBUS_BEHAVIOR_BINDER(
            NetworkCharacterNameControllerNotificationHandler,
            "{27484109-f9d6-493b-85a8-e353e2083f55}",
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
