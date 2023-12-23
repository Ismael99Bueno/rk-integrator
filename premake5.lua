project "rk-integrator"
language "C++"
cppdialect "C++17"

filter "system:macosx"
   buildoptions {
      "-Wall",
      "-Wextra",
      "-Wpedantic",
      "-Wconversion",
      "-Wno-unused-parameter",
      "-Wno-sign-conversion"
   }
filter {}

pchheader "rk/pch.hpp"
pchsource "src/pch.cpp"

staticruntime "off"
kind "StaticLib"

targetdir("bin/" .. outputdir)
objdir("build/" .. outputdir)
removedefines "YAML_CPP_GLM_COMPAT"

files {
   "src/**.cpp",
   "include/**.hpp"
}

includedirs {
   "include",
   "%{wks.location}/cpp-kit/include",
   "%{wks.location}/vendor/yaml-cpp/include",
   "%{wks.location}/vendor/spdlog/include"
}
