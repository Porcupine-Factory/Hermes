#pragma once

#include <AzCore/Component/ComponentBus.h>
#include <AzCore/RTTI/BehaviorContext.h>

#if __has_include(<FirstPersonController/FirstPersonControllerComponentBus.h>)
#include <FirstPersonController/FirstPersonControllerComponentBus.h>
#endif

namespace Hermes
{
    class HermesComponentRequests : public AZ::ComponentBus
    {
    public:
        ~HermesComponentRequests() override = default;

        virtual AZ::u32 GetNumber() const = 0;
        virtual void SetNumber(const AZ::u32&) = 0;
        virtual void SetNumberDelta(const AZ::s64&) = 0;
        virtual bool GetIsAutonomousClient() const = 0;
        virtual bool GetIsServer() const = 0;
        virtual bool GetIsHost() const = 0;
        virtual bool GetLocallyEnableNetworkHermes() const = 0;
        virtual void SetLocallyEnableNetworkHermes(const bool& new_networkHermesEnabled) = 0;
        virtual void IsAutonomousSoConnect() = 0;
        virtual void NotAutonomousSoDisconnect() = 0;
    };

    using HermesComponentRequestBus = AZ::EBus<HermesComponentRequests>;

    class HermesComponentNotifications : public AZ::ComponentBus
    {
    public:
        virtual void OnNetworkHermesTickStart(const float&) = 0;
        virtual void OnNetworkHermesTickFinish(const float&) = 0;
    };

    using HermesComponentNotificationBus = AZ::EBus<HermesComponentNotifications>;

    class HermesComponentNotificationHandler
        : public HermesComponentNotificationBus::Handler
        , public AZ::BehaviorEBusHandler
    {
    public:
        AZ_EBUS_BEHAVIOR_BINDER(
            HermesComponentNotificationHandler,
            "{c1fe5cd7-01fd-4a45-b1af-790136314692}",
            AZ::SystemAllocator,
            OnNetworkHermesTickStart,
            OnNetworkHermesTickFinish);

        void OnNetworkHermesTickStart(const float& deltaTime) override
        {
            Call(FN_OnNetworkHermesTickStart, deltaTime);
        }
        void OnNetworkHermesTickFinish(const float& deltaTime) override
        {
            Call(FN_OnNetworkHermesTickFinish, deltaTime);
        }
    };
} // namespace Hermes
