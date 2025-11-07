project "AsirikuyCommon"
  location("../../build/" .. _ACTION .. "/projects")
  kind "StaticLib"
  language "C++"
  files{
    "**.h", 
	"**.c", 
	"**.hpp", 
	"**.cpp"
  }
  vpaths{
	["Header Files"] = "../../../dev/AsirikuyCommon/include/**.*", 
	["Source Files"] = "../../../dev/AsirikuyCommon/src/**.*", 
	["Unit Tests"]   = "../../../dev/AsirikuyCommon/tests/**.*"
  }
  includedirs{
    "src"
  }
  --pchheader "Precompiled.h"
  --pchsource "Precompiled.c"
  --pchheader "Precompiled.hpp"
  --pchsource "Precompiled.cpp"