-- Copyright (c) Kerem Göksu 2026
-- SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

local util = require("script.util")

local channels = {
    unknown = "GEST_VERCHAN_UNKNOWN",
    nightly = "GEST_VERCHAN_NIGHTLY",
    alpha   = "GEST_VERCHAN_ALPHA",
    beta    = "GEST_VERCHAN_BETA",
    rc      = "GEST_VERCHAN_RELEASE_CAND",
    stable  = "GEST_VERCHAN_STABLE"
}

local version = {0, 1, 0}
local candidate = 0

newoption {
    trigger     = "test",
    description = "Enable Building Tests",
    category    = "Build Options",
}

newoption {
    trigger     = "no-uv",
    description = "Use python directly where possible",
    category    = "Build Options",
}

newoption {
    trigger     = "python",
    value       = "PY_LOCATION",
    description = "Location of the python executable",
    default     = "python",
    category    = "Build Options",
}

PYTHON = _OPTIONS["python"] or "python"

if _OPTIONS['no-uv'] then
    print("WARNING: Not using the --no-uv option is recommended to simplify setup and execution.")
    print("WARNING: Without uv, there might be environment errors, and you need to ensure")
    print("       : yourself that your virtual environment is set up correctly.")
end

newoption {
    trigger     = "channel",
    value       = "CHANNEL",
    description = "Release Channel of the Build",
    default     = "unknown",
    category    = "Build Options",
    allowed     = {
        {"unknown", "Unknown/Local Build"},
        {"nightly", "Nightly Build"},
        {"alpha",   "Alpha Release"},
        {"beta",    "Beta Release"},
        {"rc",      "Release Candidate"},
        {"stable",  "Stable Release"},
    },
}

include "script/uv-setup.lua"
include "script/unicode.lua"

local channel = _OPTIONS['channel'] or print("defaulting channel to unknown") or "unknown"
local channelVal = channels[channel] or print("defaulting channel to unknown") or channels["unknown"]
local versionString = table.concat(version, ".")

workspace "Gest"
    configurations { "StaticDebug", "StaticRelease", "SharedDebug", "SharedRelease" }
    location "build"

    includedirs { "include" }
    platforms { "bits64", "bits32" }

    cdialect "C11"
    
    floatingpoint "Default"

    filter "toolset:gcc* or toolset:clang*"
        -- Ensures NaNs are treated as non-finite 
        -- and prevents the compiler from assuming math is finite
        buildoptions { "-fno-finite-math-only", "-fno-rounding-math", "-fno-signaling-nans" }
    filter "toolset:msc*"
        buildoptions { "/experimental:c11atomics" }
        forceincludes { "include/gest-polyfills/msvc.h" }
    filter {"toolset:msc*", "configurations:*Debug"}
        editandcontinue "Off"
        flags { "NoIncrementalLink" }
    filter {"toolset:msc*", "configurations:*Debug", "platforms:bits64"}
        -- For 64-bit builds
        postbuildcommands {
            "{COPYFILE} \"$(VCToolsInstallDir)bin\\Hostx64\\x64\\clang_rt.asan_*.dll\" \"$(TargetDir)\"",
        }
    filter {"toolset:msc*", "configurations:*Debug", "platforms:bits32"}
        -- For 32-bit builds
        postbuildcommands {
            "{COPYFILE} \"$(VCToolsInstallDir)bin\\Hostx64\\x86\\clang_rt.asan_*.dll\" \"$(TargetDir)\"",
        }
    filter {}

    defines {
        "GEST_VERSION_MAJOR=" .. version[1],
        "GEST_VERSION_MINOR=" .. version[2],
        "GEST_VERSION_PATCH=" .. version[3],
        "GEST_VERSION_CHANNEL=" .. channelVal,
        "GEST_VERSION_CANDIDATE=" .. candidate,
        "GEST_VERSION_STRING=" .. string.format("%q", versionString),
    }

    filter "configurations:*Debug"
        defines { "DEBUG" }
        sanitize { "Address" }
        symbols "On"
    
    filter "configurations:*Release"
        defines { "NDEBUG" }
        optimize "On"

    filter "configurations:Static*"
        defines { "GEST_STATIC" }
    
    filter "configurations:Shared*"
        defines { "GEST_SHARED" }

    filter "platforms:bits32"
        architecture "x32"

    filter "platforms:bits64"
        architecture "x64"
    
    filter {}

project "gest"
    kind "StaticLib"
    language "C"
    location "build/gest"

    defines { "GEST_BUILD" }

    files { "include/gest/**.h", "src/gest/**.c" }

    filter "configurations:Shared*"
        kind "SharedLib"
    
    filter {}

project "gestc"
    kind "ConsoleApp"
    language "C"
    location "build/gestc"

    files { "include/gestc/**.h", "src/gestc/**.c" }

    links { "gest" }

    filter {}

if _OPTIONS['test'] then
    project "gest-test"
        kind  "ConsoleApp"
        language "C"
        location "build/gest-test"

        files { "include/gest-test/**.h", "src/gest-test/**.c" }

        links { "gest" }
        
        filter {}
end
