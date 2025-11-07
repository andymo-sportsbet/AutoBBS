project "Log"
  location("../../build/" .. _ACTION .. "/projects")
  kind "StaticLib"
  language "C"
  files{
    "**.h", 
	"**.c", 
	"**.hpp"
  }
  vpaths{
	["Header Files"] = "../../../dev/Log/include/**.*", 
	["Source Files"] = "../../../dev/Log/src/**.*", 
	["Unit Tests"]   = "../../../dev/Log/tests/**.*"
  }
  includedirs{
    "src"
  }