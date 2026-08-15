#pragma once

#include <AzCore/Component/ComponentBus.h>
#include <AzCore/RTTI/BehaviorContext.h>

#if __has_include(<FirstPersonController/FirstPersonControllerComponentBus.h>)
#include <FirstPersonController/FirstPersonControllerComponentBus.h>
#endif

namespace Hermes
{
    class CharacterNameComponentRequests : public AZ::ComponentBus
    {
    public:
        ~CharacterNameComponentRequests() override = default;

        virtual AZStd::string GetName() const = 0;
        virtual void SetName(const AZStd::string&) = 0;
        virtual void SetNameOnServerRPC(const AZStd::string&) = 0;
        virtual bool GetIsAutonomousClient() const = 0;
        virtual bool GetIsServer() const = 0;
        virtual bool GetIsHost() const = 0;
        virtual bool GetLocallyEnableNetworkCharacterName() const = 0;
        virtual void SetLocallyEnableNetworkCharacterName(const bool new_networkCharacterNameEnabled) = 0;
        virtual void IsAutonomousSoConnect() = 0;
        virtual void NotAutonomousSoDisconnect() = 0;
    };

    using CharacterNameComponentRequestBus = AZ::EBus<CharacterNameComponentRequests>;

    class CharacterNameComponentNotifications : public AZ::ComponentBus
    {
    public:
        virtual void OnNetworkCharacterNameTickStart(const float) = 0;
        virtual void OnNetworkCharacterNameTickFinish(const float) = 0;
    };

    using CharacterNameComponentNotificationBus = AZ::EBus<CharacterNameComponentNotifications>;

    class CharacterNameComponentNotificationHandler
        : public CharacterNameComponentNotificationBus::Handler
        , public AZ::BehaviorEBusHandler
    {
    public:
        AZ_EBUS_BEHAVIOR_BINDER(
            CharacterNameComponentNotificationHandler,
            "{d085976d-8958-4af4-aa19-4ea3a9438eb2}",
            AZ::SystemAllocator,
            OnNetworkCharacterNameTickStart,
            OnNetworkCharacterNameTickFinish);

        void OnNetworkCharacterNameTickStart(const float deltaTime) override
        {
            Call(FN_OnNetworkCharacterNameTickStart, deltaTime);
        }
        void OnNetworkCharacterNameTickFinish(const float deltaTime) override
        {
            Call(FN_OnNetworkCharacterNameTickFinish, deltaTime);
        }
    };
} // namespace Hermes
