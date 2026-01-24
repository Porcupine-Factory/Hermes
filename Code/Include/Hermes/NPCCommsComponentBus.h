#pragma once

#include <AzCore/Component/ComponentBus.h>
#include <AzCore/RTTI/BehaviorContext.h>

#if __has_include(<FirstPersonController/FirstPersonControllerComponentBus.h>)
#include <FirstPersonController/FirstPersonControllerComponentBus.h>
#endif

namespace Hermes
{
    class NPCCommsComponentRequests : public AZ::ComponentBus
    {
    public:
        ~NPCCommsComponentRequests() override = default;

        virtual bool GetAggroed() const = 0;
        virtual void SetAggroed(const bool&) = 0;
        virtual void SetAggroedRPC(const bool&) = 0;
        virtual bool GetIsAutonomousClient() const = 0;
        virtual bool GetIsServer() const = 0;
        virtual bool GetIsHost() const = 0;
        virtual bool GetLocallyEnableNetworkNPCComms() const = 0;
        virtual void SetLocallyEnableNetworkNPCComms(const bool& new_networkNPCCommsEnabled) = 0;
        virtual void IsAutonomousSoConnect() = 0;
        virtual void NotAutonomousSoDisconnect() = 0;
    };

    using NPCCommsComponentRequestBus = AZ::EBus<NPCCommsComponentRequests>;

    class NPCCommsComponentNotifications : public AZ::ComponentBus
    {
    public:
        virtual void OnNetworkNPCCommsTickStart(const float&) = 0;
        virtual void OnNetworkNPCCommsTickFinish(const float&) = 0;
    };

    using NPCCommsComponentNotificationBus = AZ::EBus<NPCCommsComponentNotifications>;

    class NPCCommsComponentNotificationHandler
        : public NPCCommsComponentNotificationBus::Handler
        , public AZ::BehaviorEBusHandler
    {
    public:
        AZ_EBUS_BEHAVIOR_BINDER(
            NPCCommsComponentNotificationHandler,
            "{a4d69e55-47e4-4e33-8dbf-53ad325e5157}",
            AZ::SystemAllocator,
            OnNetworkNPCCommsTickStart,
            OnNetworkNPCCommsTickFinish);

        void OnNetworkNPCCommsTickStart(const float& deltaTime) override
        {
            Call(FN_OnNetworkNPCCommsTickStart, deltaTime);
        }
        void OnNetworkNPCCommsTickFinish(const float& deltaTime) override
        {
            Call(FN_OnNetworkNPCCommsTickFinish, deltaTime);
        }
    };
} // namespace Hermes
