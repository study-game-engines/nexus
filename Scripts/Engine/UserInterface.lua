project "UserInterface"
    kind "SharedLib"
    language "C++"
    location (EngDir.."UserInterface")

    defines "NEXUS_USER_INTERFACE_DLL"

    targetdir(BinDir)
    objdir(IntDir)

    includedirs
    {
        IncludeDir["UserInterface"],

        IncludeDir["Platform"],
        IncludeDir["Graphics"],
        IncludeDir["DebugUtils"],

        IncludeDir["glfw"],
        IncludeDir["spdlog"],
    }

    files
    {
        (EngDir.."UserInterface/**.h"),
        (EngDir.."UserInterface/**.cpp")
    }

    links
    {
        "Platform",
        "Graphics",
        "DebugUtils",

        "vulkan-1.lib",
        "glfw"
    }

    libdirs
    {
        LibDir["vulkanSDK"]
    }

    filter "system:windows"
        defines "NEXUS_SYSTEM_WINDOWS"
        cppdialect "C++20"
        systemversion "latest"
        disablewarnings { "4251" }

    filter "configurations:Debug"
        defines "NEXUS_DEBUG"
        optimize "Off"
        symbols "Full"

    filter "configurations:Release"
        defines "NEXUS_RELEASE"
        optimize "Speed"
        symbols "FastLink"

    filter "configurations:Dist"
        defines "NEXUS_DIST"
        optimize "Full"
        symbols "Off"
