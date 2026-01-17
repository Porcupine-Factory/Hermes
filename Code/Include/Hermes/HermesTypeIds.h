
#pragma once

namespace Hermes
{
    // System Component TypeIds
    inline constexpr const char* HermesSystemComponentTypeId = "{C33906CC-88D5-4138-B85C-F68DCF5914D0}";
    inline constexpr const char* HermesEditorSystemComponentTypeId = "{C7E6C824-4349-4336-AEF1-24E9244A2FCC}";

    // Module derived classes TypeIds
    inline constexpr const char* HermesModuleInterfaceTypeId = "{AA33DD48-8704-48A7-AEFC-38659E830886}";
    inline constexpr const char* HermesModuleTypeId = "{CFB46D88-CA11-4267-8B95-11D067386708}";
    // The Editor Module by default is mutually exclusive with the Client Module
    // so they use the Same TypeId
    inline constexpr const char* HermesEditorModuleTypeId = HermesModuleTypeId;

    // Interface TypeIds
    inline constexpr const char* HermesRequestsTypeId = "{BFC4D322-A6D3-4929-BA34-316451C81AA1}";
} // namespace Hermes
