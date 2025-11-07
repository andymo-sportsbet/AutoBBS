project "NTPClient"
  location("../../build/" .. _ACTION .. "/projects")
  kind "StaticLib"
  language "C++"
  uses "Boost"
  files{
    "**.hpp", 
	"**.cpp"
  }
  vpaths{
	["Header Files"] = "../../../dev/NTPClient/include/**.*", 
	["Source Files"] = "../../../dev/NTPClient/src/**.*", 
	["Unit Tests"]   = "../../../dev/NTPClient/tests/**.*"
  }
  includedirs{
    "src"
  }
  
usage "NTPClient"
  kind "StaticLib"
  uses "Boost"
  includedirs "include"
  links "NTPClient"