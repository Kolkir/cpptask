scripts_path = "build_scripts"

if _ACTION == "clean" then
	os.rmdir("bin")
	os.rmdir(scripts_path)
end   

solution "CppTaskTest"
	location(scripts_path)
	configurations { "Debug", "Release" }
    platforms { "x32", "x64" }
    if (linux) then
        package.linkoptions = { "-pthread" }
    end
    
	configuration { "Debug" }
		targetdir "bin/debug"
 
	configuration { "Release" }
		targetdir "bin/release"
      
	project "CppTaskTest"  	    
		kind "ConsoleApp"
		language "C++"        
		files { "**.h", "**.cpp", "../include/**.h" } 
        includedirs { "../include" }

		configuration "Debug"
			defines { "DEBUG" }
			flags { "Symbols", "ExtraWarnings" }            

		configuration "Release"
			defines { "NDEBUG" }
			flags { "OptimizeSpeed", "ExtraWarnings"  }

	