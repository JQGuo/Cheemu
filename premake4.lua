#!lua

includeDirList = { 
    "external/imgui",
    "external/imgui-sdl",
    "external/imgui/examples/libs/gl3w",
    "build/include"
}


buildOptions = {"-std=c++14"}

-- Get the current OS platform
ROOT_DIR = os.getcwd()
PLATFORM = os.get()

--  TODO: How to get this to work on windows...
--  Build and install SDL2
if _ACTION ~= "clean" then
    if not os.isfile("build/lib/libSDL2.a") then
        os.chdir("external/SDL2-2.0.5")
        os.execute(string.format("./configure --prefix=%s/build/", ROOT_DIR))
        os.execute("make")
        os.execute("make install")
        os.chdir(ROOT_DIR)
    end

    -- Build and install SDL2_mixer
    if not os.isfile("build/lib/libSDL2_mixer.a") then
        os.chdir("external/SDL2_mixer-2.0.1")
        os.execute(string.format("./configure --prefix=%s/build/", ROOT_DIR))
        os.execute("make")
        os.execute("make install")
        os.chdir(ROOT_DIR)
    end
end

-- Build imgui and gl3w, and statically link to these
solution "BuildStaticLibs"
    configurations { "Debug", "Release" }

    configuration "Debug"
        defines { "DEBUG" }
        flags { "Symbols" }

    configuration "Release"
        defines { "NDEBUG" }
        flags { "Optimize" }

    -- Build imgui and gl3w
    project "imgui"
        kind "StaticLib"
        language "C++"
        location "build"
        objdir "build"
        targetdir "lib"
        includedirs (includeDirList)
        includedirs {
            "external/imgui/examples/sdl_opengl3_example"
        }
        files { 
            "external/imgui/*.cpp",
            "external/imgui/examples/libs/gl3w/GL/gl3w.c"
        }

    -- Build imgui SDL impl
    project "imgui-sdl"
        kind "StaticLib"
        language "C++"
        location "build"
        objdir "build"
        targetdir "lib"
        includedirs (includeDirList)
        files { 
            "external/imgui-sdl/*.cpp",
        }
    
    -- Premake doesn't seem to build static libs if target dir is in build folder
    postbuildcommands {
        string.format("mv %s/lib/libimgui.a %s/build/lib/",ROOT_DIR, ROOT_DIR), 
        string.format("mv %s/lib/libimgui-sdl.a %s/build/lib/", ROOT_DIR, ROOT_DIR),
        string.format("rmdir %s/lib", ROOT_DIR)
    }
