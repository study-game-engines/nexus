project "NxAsset"
	kind "SharedLib"
	language "C++"
	cppdialect "C++20"
	location (SrcDir.."Engine/%{prj.name}")

	targetdir (BinDir)
	objdir (IntDir)

	files
	{
		(SrcDir.."Engine/%{prj.name}/include/**.h"),
		(SrcDir.."Engine/%{prj.name}/src/**.cpp")
	}

	includedirs
	{
		"$(VULKAN_SDK)/Include",
        IncludePath["stb"],
        IncludePath["tinygltf"],
        IncludePath["yamlcpp"],
        IncludePath["nlohmannJson"],
        
		IncludePath["NxCore"],
		IncludePath["NxGraphics"],
		IncludePath["NxAsset"]
	}

	links
	{
		"yamlcpp",
        "NxCore",
		"NxGraphics"
	}

	defines 
	{
		"NEXUS_ASSET_SHARED_BUILD"
	}

	filter "system:windows"
		systemversion "latest"
		defines "NEXUS_SYSTEM_WINDOWS"
		disablewarnings { "4251","4275" }

	filter "configurations:Debug"
		optimize "Off"
		symbols "Full"
		defines "NEXUS_DEBUG"

	filter "configurations:Release"
		optimize "Speed"
		symbols "FastLink"
		defines "NEXUS_RELEASE"

	filter "configurations:Dist"
		optimize "Full"
		symbols "Off"
		defines "NEXUS_DIST"