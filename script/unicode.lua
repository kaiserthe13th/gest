-- SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
-- SPDX-FileCopyrightText: Copyright 2026 Kerem Göksu

local util = require("script.util")
local M = {}

M.targetDir = util.mpath(path.join("data", "unicode"))
M.downloadCompletionFile = path.join(M.targetDir, ".unicode_dl")
M.generationCompletionFile = path.join(M.targetDir, ".unicode_gen")

M.dlPyFile = util.mpath(path.join("script", "unicode", "dl.py"))

function M.downloadUnicode()
    local script_dir = util.mpath("script")

    print("Creating data/unicode...")
    os.mkdir(M.targetDir)
    
    io.write("Selecting tool... ")
    local cmd
    if _OPTIONS['no-uv'] then
        print("Using: python [" .. PYTHON .. "]")
        cmd = string.format('"%s" "%s" "%s"', PYTHON, M.dlPyFile, M.targetDir)
    else
        print("Using: uv")
        cmd = string.format('uv run --project "%s" "%s" "%s"', script_dir, M.dlPyFile, M.targetDir)
    end
    print("Running script/unicode/dl.py")

    cmd = util.prepareCommand(cmd)

    local success = os.execute(cmd)
    if not success then
        print("\nError: Program exit unsuccessfully.")
        os.exit(1)
    end

    print("Creating completion file...")
    local cacheFile = io.open(M.downloadCompletionFile, "w")
    local hash = util.contentHashOf(M.dlPyFile)
    cacheFile:write(hash)
    cacheFile:close()
end

newaction {
    trigger     = "download-unicode",
    description = "Download the unicode files necessary to compile the project.",
    execute     = M.downloadUnicode,
}

M.genPyFile = util.mpath(path.join("script", "unicode", "gen.py"))

function M.generateUnicodeHeaders()
    local target_header_dir = util.mpath(path.join("include", "gest", "_generated"))
    local target_code_dir = util.mpath(path.join("src", "gest", "_generated"))
    local script_dir = util.mpath("script")
    local data_dir = util.mpath("data")

    print("Creating include/gest/_generated...")
    os.mkdir(target_header_dir)
    print("Creating src/gest/_generated...")
    os.mkdir(target_code_dir)
    
    io.write("Selecting tool... ")
    local cmd
    if _OPTIONS['no-uv'] then
        print("Using: python [" .. PYTHON .. "]")
        cmd = string.format('"%s" "%s" -s -D "%s" -H "%s" -c "%s"', PYTHON, M.genPyFile, data_dir, path.join(target_header_dir, "_unicode.h"), path.join(target_code_dir, "_unicode.c"))
    else
        print("Using: uv")
        cmd = string.format('"uv" run --project "%s" "%s" -s -D "%s" -H "%s" -c "%s"', script_dir, M.genPyFile, data_dir, path.join(target_header_dir, "_unicode.h"), path.join(target_code_dir, "_unicode.c"))
    end
    print("Running script/unicode/gen.py")

    cmd = util.prepareCommand(cmd)
    
    local success = os.execute(cmd)
    if not success then
        print("\nError: Program exit unsuccessfully.")
        os.exit(1)
    end
    local cacheFile = io.open(M.generationCompletionFile, "w")
    local hash = util.contentHashOf(M.genPyFile)
    cacheFile:write(hash)
    cacheFile:close()
end

newaction {
    trigger     = "gen-unicode-headers",
    description = "Generate the unicode headers necessary to compile the project from the unicode files downloaded.",
    execute     = M.generateUnicodeHeaders,
}

return M
