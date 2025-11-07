project "AsirikuyRWrapper"
  location("../../build/" .. _ACTION .. "/projects")
  kind "StaticLib"
  language "C++"
  files{
	"**.hpp", 
	"**.cpp"
  }
  vpaths{
	["Header Files"] = "**.hpp", 
	["Source Files"] = "**.cpp", 
  }
