project "AsirikuyEasyTrade"
  location("../../build/" .. _ACTION .. "/projects")
  kind "StaticLib"
  language "C++"
  files{
    "**.hpp", 
	"**.cpp"
  }
  includedirs{
    "src"
  }
  configuration{"macosx"}
    libdirs{
      "/Users/andym/homebrew/opt/curl/lib",
      "/usr/local/lib",
      "/usr/lib"
    }
    links{"curl"}
  -- Enable C++11 for Boost compatibility
  buildoptions{"-std=c++11"}