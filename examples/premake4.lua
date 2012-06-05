scripts_path = "build_scripts"

if _ACTION == "clean" then
	os.rmdir("bin")
	os.rmdir(scripts_path)
end   

solution "CppTaskTest"
	location(scripts_path)
	configurations { "Debug", "Release" }
	platforms { "x32", "x64" }
    
	configuration { "Debug" }
		targetdir "bin/debug"
 
	configuration { "Release" }
		targetdir "bin/release"

	configuration( "windows" )
		defines({"NOMINMAX"})    
	configuration "linux"
		links { "pthread" }

	project "CppTaskTest"  	    
		kind "ConsoleApp"
		language "C++"        
		files { "**.h", "**.cpp", "../include/**.h" } 
		includedirs { "../include" }
		if (_OPTIONS["os"] == "linux") then
			links  { "pthread","rt" }
		end

		configuration "Debug"
			defines { "DEBUG" }
			flags { "Symbols", "ExtraWarnings" }            

		configuration "Release"
			defines { "NDEBUG" }
			flags { "OptimizeSpeed", "ExtraWarnings"  }

	