project "AsirikuyEasyTrade"
  location("../../build/" .. _ACTION .. "/projects")
  kind "StaticLib"
  language "C++"
  files{
	"**.hpp", 
	"**.cpp"
  }
  vpaths{
	["Header Files"] = "../../../dev/AsirikuyEasyTrade/include/**.*", 
	["Source Files"] = "../../../dev/AsirikuyEasyTrade/src/**.*", 
	["Unit Tests"]   = "../../../dev/AsirikuyEasyTrade/tests/**.*"
  }
  includedirs{
    "src"
  }