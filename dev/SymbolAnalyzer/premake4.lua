project "SymbolAnalyzer"
  location("../../build/" .. _ACTION .. "/projects")
  kind "StaticLib"
  language "C"
  files{
    "**.h", 
	"**.c", 
	"**.hpp"
  }
  vpaths{
	["Header Files"] = "../../../dev/SymbolAnalyzer/include/**.*", 
	["Source Files"] = "../../../dev/SymbolAnalyzer/src/**.*", 
	["Unit Tests"]   = "../../../dev/SymbolAnalyzer/tests/**.*"
  }
  includedirs{
    "src"
  }