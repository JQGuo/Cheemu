#!lua

includeDirList = {
    "../external/imgui",
    "../external/imgui/examples/libs/gl3w",
    "../external/imgui-sdl",
    "../external/spdlog/include",
    "../build/include",
}

libDirectories = { 
    "../build/lib"
}


if os.get() == "linux" then
    linkLibs = {
        "imgui",
        "imgui-sdl",
        "SDL2",
        "SDL2_mixer",
        "GL",
        "Xinerama",
        "Xcursor",
        "Xxf86vm",
        "Xi",
        "Xrandr",
        "X11",
        "stdc++",
        "dl",
        "pthread"
    }
end

buildOptions = {"-std=c++14"}

solution "Cheemu"
    configurations { "Debug", "Release" }

    project "Cheemu"
        kind "ConsoleApp"
        language "C++"
        location "build"
        objdir "build"
        targetdir "."
        buildoptions (buildOptions)
        libdirs (libDirectories)
        links (linkLibs)
        linkoptions (linkOptionList)
        includedirs (includeDirList)
        includedirs ("include")
        files { "src/*.cpp" }

    project "CheemuTest"
        kind "ConsoleApp"
        language "C++"
        location "build"
        objdir "build"
        targetdir "."
        buildoptions (buildOptions)
        libdirs (libDirectories)
        links (linkLibs)
        linkoptions (linkOptionList)
        includedirs (includeDirList)
        includedirs ("include")
        includedirs ("../external/Catch/single_include")
        files {
            "src/Chip8.cpp",
            "test/*.cpp"
        }
        defines { "NO_LOG" }

    configuration "Debug"
        defines { "DEBUG" }
        flags { "Symbols" }

    configuration "Release"
        defines { "NDEBUG" }
        flags { "Optimize" }
