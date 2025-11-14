project "AsirikuyCommon"
  location("../../build/" .. _ACTION .. "/projects")
  kind "StaticLib"
  language "C++"
  files{
    "**.h", 
	"**.c", 
	"**.hpp", 
	"**.cpp"
  }
  -- vpaths not supported in premake4 4.3
  includedirs{
    "src",
    "include"
  }
  -- Enable C++11 for Boost compatibility
  -- Since language is "C++", .cpp files need C++11 for Boost scoped_ptr (noexcept)
  -- C files (.c) will be compiled as C without C++11 flag
  -- Note: premake4 doesn't support file-specific buildoptions well, so we apply to CXXFLAGS
  -- The Makefile will use CXXFLAGS for .cpp files and CFLAGS for .c files
  buildoptions{"-std=c++11"}
  -- Override: remove C++11 from CFLAGS, keep in CXXFLAGS only
  -- This is handled by premake4's language="C++" which separates C and C++ compilation
  --pchheader "Precompiled.h"
  --pchsource "Precompiled.c"
  --pchheader "Precompiled.hpp"
  --pchsource "Precompiled.cpp"