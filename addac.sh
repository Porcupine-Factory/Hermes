#!/bin/bash
# Use this script to create a new regular component and an accompanying networking auto-component

# Ensure that the script is running from where its located
cd "$(dirname "${BASH_SOURCE[0]}")" || exit

# Choose a type for the first variable
read -rp "Enter a type for the first variable (NOTE: You will need to change its init values yourself!): " var_type

rg "ExampTemporaryVar" ./Template/ -l0 | xargs -0 sed -i "s/AZ::u32/$var_type/g"

# Choose a name for the (auto) component
read -rp "Enter a name for the new (auto) component: " ac_name
if [[ "$ac_name" =~ ^[[:lower:]] ]]; then
  ac_name="${ac_name^}"
fi

# Copy the template files
cp ./Template/NetworkExample.AutoComponent.xml ./Code/Source/AutoGen/Network"$ac_name".AutoComponent.xml
cp ./Template/NetworkExample.cpp ./Code/Source/Multiplayer/Network"$ac_name".cpp
cp ./Template/NetworkExample.h ./Code/Source/Multiplayer/Network"$ac_name".h
cp ./Template/ExampleComponent.cpp ./Code/Source/Clients/"$ac_name"Component.cpp
cp ./Template/ExampleComponent.h ./Code/Source/Clients/"$ac_name"Component.h
cp ./Template/ExampleBus.h ./Code/Include/Hermes/"$ac_name"Bus.h
cp ./Template/ExampleComponentBus.h ./Code/Include/Hermes/"$ac_name"ComponentBus.h
cp ./Template/NetworkExampleBus.h ./Code/Include/Hermes/Network"$ac_name"Bus.h
cp ./Template/NetworkExampleControllerBus.h ./Code/Include/Hermes/Network"$ac_name"ControllerBus.h

# Restore all the files in Template back to using AZ::u32 for the first variable
git restore ./Template/

# Rename everything using the component name that was chosen
sed -i "0,/^$/{/^$/i\
\ \ \ \ Source/Clients/""$ac_name""Component.cpp\\n\ \ \ \ Source/Clients/""$ac_name""Component.h\\n\ \ \ \ Source/Multiplayer/Network""$ac_name"".cpp\\n\ \ \ \ Source/Multiplayer/Network""$ac_name"".h
}" ./Code/hermes_private_files.cmake

line_num=$(wc -l < ./Code/hermes_private_files.cmake)
sed -i "${line_num}i\    Source/AutoGen/Network""$ac_name"".AutoComponent.xml" ./Code/hermes_private_files.cmake

line_num=$(wc -l < ./Code/hermes_api_files.cmake)
sed -i "${line_num}i\    Include/Hermes/""$ac_name""Bus.h" ./Code/hermes_api_files.cmake
sed -i "${line_num}i\    Include/Hermes/""$ac_name""ComponentBus.h" ./Code/hermes_api_files.cmake

rg --files-with-matches "Example" -g '!Template/' -g '!addac.sh' | xargs sed -i "s/Example/$ac_name/g"
lower_ac_name=${ac_name,}
rg --files-with-matches "example" -g '!Template/' -g '!addac.sh' | xargs sed -i "s/example/$lower_ac_name/g"

# Include in module interface header
sed -i '0,/#if\ __has_include(<Source\/AutoGen\/AutoComponentTypes\.h>)/{s//#include <Clients\/'"$ac_name"'Component\.h>\
&/};' ./Code/Source/HermesModuleInterface.h

# Include in system component header
sed -i "$(grep -n 'include' ./Code/Source/Clients/HermesSystemComponent.h | tail -1 | cut -f1 -d':')a\
#include\ <Hermes/""$ac_name""Bus\.h>\n\
#include\ <Hermes/Network""$ac_name""Bus\.h>\n\
#include\ <Hermes/Network""$ac_name""ControllerBus\.h>
" ./Code/Source/Clients/HermesSystemComponent.h

# Inherit the request buses
sed -i "$(grep -n ', protected' ./Code/Source/Clients/HermesSystemComponent.h | tail -1 | cut -f1 -d':')a\
\ \ \ \ \ \ \ \ ,\ protected\ ""$ac_name""RequestBus::Handler\n\
\ \ \ \ \ \ \ \ ,\ protected\ Network""$ac_name""RequestBus::Handler
" ./Code/Source/Clients/HermesSystemComponent.h

# Edit Context
sed -i "$(grep -n 'AutoExpand' ./Code/Source/Clients/HermesSystemComponent.cpp | tail -1 | cut -f1 -d':')a\
\\
\\
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ ec->Class<HermesSystemComponent>(\n\
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \"Network$ac_name\",\ \"[Description\ of\ functionality\ provided\ by\ this\ System\ Component]\")\n\
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ ->ClassElement(AZ::Edit::ClassElements::EditorData,\ \"\")\n\
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu,\ AZ_CRC(\"System\"))\n\
\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ ->Attribute(AZ::Edit::Attributes::AutoExpand,\ true);
" ./Code/Source/Clients/HermesSystemComponent.cpp

# GetProvidedServices(...)
sed -i "$(grep -n 'provided\.push_back' ./Code/Source/Clients/HermesSystemComponent.cpp | tail -1 | cut -f1 -d':')a\
\ \ \ \ \ \ \ \ provided.push_back(AZ_CRC_CE(\"$ac_name""Service\"));\n\
\ \ \ \ \ \ \ \ provided.push_back(AZ_CRC_CE(\"Network""$ac_name""Service\"));
" ./Code/Source/Clients/HermesSystemComponent.cpp

# GetIncompatibleServices(...)
sed -i "$(grep -n 'incompatible\.push_back' ./Code/Source/Clients/HermesSystemComponent.cpp | tail -1 | cut -f1 -d':')a\
\ \ \ \ \ \ \ \ incompatible.push_back(AZ_CRC_CE(\"$ac_name""Service\"));\n\
\ \ \ \ \ \ \ \ incompatible.push_back(AZ_CRC_CE(\"Network""$ac_name""Service\"));
" ./Code/Source/Clients/HermesSystemComponent.cpp

# Insert descriptor
line_num=$(grep -n -m 1 "});" ./Code/Source/HermesModuleInterface.cpp | cut -d: -f1)
sed -i "${line_num}i \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ ""$ac_name""Component::CreateDescriptor()," ./Code/Source/HermesModuleInterface.cpp

# Activate()
line_num=$(grep -n -m 1 "TickBus" ./Code/Source/Clients/HermesSystemComponent.cpp | cut -d: -f1)
sed -i "${line_num}i \ \ \ \ \ \ \ \ ""$ac_name""RequestBus::Handler::BusDisconnect();" ./Code/Source/Clients/HermesSystemComponent.cpp

line_num=$(grep -n -m 1 "TickBus" ./Code/Source/Clients/HermesSystemComponent.cpp | cut -d: -f1)
sed -i "${line_num}i \ \ \ \ \ \ \ \ Network""$ac_name""RequestBus::Handler::BusDisconnect();" ./Code/Source/Clients/HermesSystemComponent.cpp

# Deactivate()
sed -i "$(grep -n 'TickBus' ./Code/Source/Clients/HermesSystemComponent.cpp | tail -1 | cut -f1 -d':')a\
\ \ \ \ \ \ \ \ Network""$ac_name""RequestBus::Handler::BusDisconnect();\n\
\ \ \ \ \ \ \ \ ""$ac_name""RequestBus::Handler::BusDisconnect();
" ./Code/Source/Clients/HermesSystemComponent.cpp

# Generate UUIDs
UUIDGEN='uuidgen'
component_uuid=$($UUIDGEN)
componentbus_uuid=$($UUIDGEN)
networkbus_uuid=$($UUIDGEN | tr "[:lower:]" "[:upper:]")
networkcontrollerbus_uuid=$($UUIDGEN)

sed -i 's/componentuuid/'"$component_uuid"'/' ./Code/Source/Clients/"$ac_name"Component.h
sed -i 's/componentbusuuid/'"$componentbus_uuid"'/' ./Code/Include/Hermes/"$ac_name"ComponentBus.h
sed -i 's/networkbusuuid/'"$networkbus_uuid"'/' ./Code/Include/Hermes/Network"$ac_name"Bus.h
sed -i 's/networkcontrollerbusuuid/'"$networkcontrollerbus_uuid"'/' ./Code/Include/Hermes/Network"$ac_name"ControllerBus.h

read -rp "Enter a name for the first variable: " var_name

var_name=${var_name^}
rg "ExampTemporaryVar" -l0 -g '!Template/' -g '!addac.sh' | xargs -0 sed -i "s/ExampTemporaryVar/$var_name/g"
var_name=${var_name,}
rg "exampTemporaryVar" -l0 -g '!Template/' -g '!addac.sh' | xargs -0 sed -i "s/exampTemporaryVar/$var_name/g"
