project "AsirikuyMachineLearning"
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
	["Header Files"] = "../../../dev/AsirikuyMachineLearning/include/**.*", 
	["Source Files"] = "../../../dev/AsirikuyMachineLearning/src/**.*", 
	["Unit Tests"]   = "../../../dev/AsirikuyMachineLearning/tests/**.*"
  }
  configuration{"macosx"}
    buildoptions{"-fpermissive"}
  configuration{"windows"}
    defines{"NOMINMAX"}
