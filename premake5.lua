project "rk-integrator"
language "C++"
cppdialect "C++17"

filter "system:macosx"
   buildoptions {
      "-Wall",
      "-Wextra",
      "-Wpedantic",
      "-Wconversion",
      "-Wno-unused-parameter"
   }
filter {}

pchheader "rk/pch.hpp"
pchsource "src/pch.cpp"

staticruntime "off"
kind "StaticLib"

targetdir("bin/" .. outputdir)
objdir("build/" .. outputdir)
removedefines "HAS_GLM"

files {
   "src/**.cpp",
   "include/**.hpp"
}

includedirs {
   "include",
   "%{wks.location}/debug-tools/include",
   "%{wks.location}/profile-tools/include",
   "%{wks.location}/vendor/yaml-cpp/include"
}
