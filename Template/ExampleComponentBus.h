#pragma once

#include <AzCore/Component/ComponentBus.h>
#include <AzCore/RTTI/BehaviorContext.h>

#if __has_include(<FirstPersonController/FirstPersonControllerComponentBus.h>)
#include <FirstPersonController/FirstPersonControllerComponentBus.h>
#endif

namespace Hermes
{
    class ExampleComponentRequests : public AZ::ComponentBus
    {
    public:
        ~ExampleComponentRequests() override = default;

        virtual AZ::u32 GetExampTemporaryVar() const = 0;
        virtual void SetExampTemporaryVar(const AZ::u32&) = 0;
        virtual void SetExampTemporaryVarRPC(const AZ::u32&) = 0;
        virtual bool GetIsAutonomousClient() const = 0;
        virtual bool GetIsServer() const = 0;
        virtual bool GetIsHost() const = 0;
        virtual bool GetLocallyEnableNetworkExample() const = 0;
        virtual void SetLocallyEnableNetworkExample(const bool networkExampleEnabled) = 0;
        virtual void IsAutonomousSoConnect() = 0;
        virtual void NotAutonomousSoDisconnect() = 0;
    };

    using ExampleComponentRequestBus = AZ::EBus<ExampleComponentRequests>;

    class ExampleComponentNotifications : public AZ::ComponentBus
    {
    public:
        virtual void OnNetworkExampleTickStart(const float) = 0;
        virtual void OnNetworkExampleTickFinish(const float) = 0;
    };

    using ExampleComponentNotificationBus = AZ::EBus<ExampleComponentNotifications>;

    class ExampleComponentNotificationHandler
        : public ExampleComponentNotificationBus::Handler
        , public AZ::BehaviorEBusHandler
    {
    public:
        AZ_EBUS_BEHAVIOR_BINDER(
            ExampleComponentNotificationHandler,
            "{componentbusuuid}",
            AZ::SystemAllocator,
            OnNetworkExampleTickStart,
            OnNetworkExampleTickFinish);

        void OnNetworkExampleTickStart(const float deltaTime) override
        {
            Call(FN_OnNetworkExampleTickStart, deltaTime);
        }
        void OnNetworkExampleTickFinish(const float deltaTime) override
        {
            Call(FN_OnNetworkExampleTickFinish, deltaTime);
        }
    };
} // namespace Hermes
