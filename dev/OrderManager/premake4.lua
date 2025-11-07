project "OrderManager"
  location("../../build/" .. _ACTION .. "/projects")
  kind "StaticLib"
  language "C"
  files{
    "**.h", 
	"**.c", 
	"**.hpp"
  }
  vpaths{
	["Header Files"] = "../../../dev/OrderManager/include/**.*", 
	["Source Files"] = "../../../dev/OrderManager/src/**.*", 
	["Unit Tests"]   = "../../../dev/OrderManager/tests/**.*"
  }
  includedirs{
    "src"
  }