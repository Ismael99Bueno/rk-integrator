project "rk-integrator"
   language "C++"
   cppdialect "C++17"
   
   kind "StaticLib"
   staticruntime "off"

   targetdir("bin/" .. outputdir)
   objdir("build/" .. outputdir)

   files {"src/**.cpp", "include/**.hpp"}

   includedirs "../**/include"