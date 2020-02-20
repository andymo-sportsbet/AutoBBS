project "TradingStrategies"
  location("../../build/" .. _ACTION .. "/projects")
  kind "StaticLib"
  language "C"
  files{
    "**.h", 
	"**.c", 
	"**.hpp"
  }
  vpaths{
	["Header Files/*"] = "../../../dev/TradingStrategies/include/**.*", 
	["Source Files/*"] = "../../../dev/TradingStrategies/src/**.*", 
	["Unit Tests"]   = "../../../dev/TradingStrategies/tests/**.*"
  }
  includedirs{
    "src"
  }
  defines{"FANN_NO_DLL"}

