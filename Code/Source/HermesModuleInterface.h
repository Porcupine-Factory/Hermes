
#include <AzCore/Memory/Memory_fwd.h>
#include <AzCore/Module/Module.h>
#include <AzCore/RTTI/RTTIMacros.h>
#include <AzCore/RTTI/TypeInfoSimple.h>
#include <Clients/CharacterNameComponent.h>
#include <Clients/HermesComponent.h>
#include <Clients/NPCCommsComponent.h>
#include <Clients/SpawnerComponent.h>
#if __has_include(<Source/AutoGen/AutoComponentTypes.h>)
#include <Source/AutoGen/AutoComponentTypes.h>
#endif

namespace Hermes
{
    class HermesModuleInterface : public AZ::Module
    {
    public:
        AZ_TYPE_INFO_WITH_NAME_DECL(HermesModuleInterface)
        AZ_RTTI_NO_TYPE_INFO_DECL()
        AZ_CLASS_ALLOCATOR_DECL

        HermesModuleInterface();

        /**
         * Add required SystemComponents to the SystemEntity.
         */
        AZ::ComponentTypeList GetRequiredSystemComponents() const override;
    };
} // namespace Hermes
