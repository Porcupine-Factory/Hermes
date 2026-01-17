
#include "HermesSystemComponent.h"
#include <Hermes/HermesTypeIds.h>
#include <HermesModuleInterface.h>

namespace Hermes
{
    class HermesModule : public HermesModuleInterface
    {
    public:
        AZ_RTTI(HermesModule, HermesModuleTypeId, HermesModuleInterface);
        AZ_CLASS_ALLOCATOR(HermesModule, AZ::SystemAllocator);
    };
} // namespace Hermes

#if defined(O3DE_GEM_NAME)
AZ_DECLARE_MODULE_CLASS(AZ_JOIN(Gem_, O3DE_GEM_NAME), Hermes::HermesModule)
#else
AZ_DECLARE_MODULE_CLASS(Gem_Hermes, Hermes::HermesModule)
#endif
