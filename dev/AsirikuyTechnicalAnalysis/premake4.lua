project "AsirikuyTechnicalAnalysis"
  location("../../build/" .. _ACTION .. "/projects")
  kind "StaticLib"
  language "C"
  files{
    "**.h", 
	"**.c", 
	"**.hpp"
  }
  vpaths{
	["Header Files"] = "../../../dev/AsirikuyTechnicalAnalysis/include/**.*", 
	["Source Files"] = "../../../dev/AsirikuyTechnicalAnalysis/src/**.*", 
	["Unit Tests"]   = "../../../dev/AsirikuyTechnicalAnalysis/tests/**.*"
  }
  includedirs{
    "src"
  }