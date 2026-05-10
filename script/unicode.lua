-- SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
-- SPDX-FileCopyrightText: Copyright 2026 Kerem Göksu

local util = require("script.util")

newaction {
    trigger     = "download-unicode",
    description = "Download the unicode files necessary to compile the project.",
    execute     = function()
        local target_dir = util.mpath(path.join("data", "unicode"))
        local script_dir = util.mpath("script")
        local dl_py_file = util.mpath(path.join("script", "unicode", "gen.py"))

        print("Creating data/unicode...")
        os.mkdir(target_dir)
        
        io.write("Selecting tool... ")
        local cmd
        if _OPTIONS['no-uv'] then
            print("Using: python [" .. PYTHON .. "]")
            cmd = string.format('"%s" "%s" "%s"', PYTHON, dl_py_file, target_dir)
        else
            print("Using: uv")
            cmd = string.format('uv run --project "%s" "%s" "%s"', script_dir, dl_py_file, target_dir)
        end
        print("Running script/unicode/dl.py")

        cmd = util.prepareCommand(cmd)

        local success = os.execute(cmd)
        if not success then
            print("\nError: Program exit unsuccessfully.")
            os.exit(1)
        end
    end
}

newaction {
    trigger     = "gen-unicode-headers",
    description = "Generate the unicode headers necessary to compile the project from the unicode files downloaded.",
    execute     = function()
        local target_header_dir = util.mpath(path.join("include", "gest", "_generated"))
        local target_code_dir = util.mpath(path.join("src", "gest", "_generated"))
        local script_dir = util.mpath("script")
        local data_dir = util.mpath("data")
        local gen_py_file = util.mpath(path.join("script", "unicode", "gen.py"))

        print("Creating include/gest/_generated...")
        os.mkdir(target_header_dir)
        print("Creating src/gest/_generated...")
        os.mkdir(target_code_dir)
        
        io.write("Selecting tool... ")
        local cmd
        if _OPTIONS['no-uv'] then
            print("Using: python [" .. PYTHON .. "]")
            cmd = string.format('"%s" "%s" -s -D "%s" -H "%s" -c "%s"', PYTHON, gen_py_file, data_dir, path.join(target_header_dir, "_unicode.h"), path.join(target_code_dir, "_unicode.c"))
        else
            print("Using: uv")
            cmd = string.format('"uv" run --project "%s" "%s" -s -D "%s" -H "%s" -c "%s"', script_dir, gen_py_file, data_dir, path.join(target_header_dir, "_unicode.h"), path.join(target_code_dir, "_unicode.c"))
        end
        print("Running script/unicode/gen.py")

        cmd = util.prepareCommand(cmd)
        
        local success = os.execute(cmd)
        if not success then
            print("\nError: Program exit unsuccessfully.")
            os.exit(1)
        end
    end
}