-- SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
-- SPDX-FileCopyrightText: Copyright 2026 Kerem Göksu

local util = require("script.util")

newaction {
    trigger     = "uv-setup",
    description = "Use uv to set up a python virtual environment",
    execute     = function()
        local old_dir = os.getcwd()
        local target_dir = util.mpath("script")
        print("Moving to: " .. target_dir)
        local ok, err = os.chdir(target_dir)
        if ok then
            print("Initializing the virtual environment...")
            os.execute("uv venv --allow-existing")
            print("Running uv sync...")
            os.execute("uv sync")
            print("Moving back to: " .. old_dir)
            os.chdir(old_dir)
        else
            print("Error: Could not find the script directory! " .. (err or ""))
        end
    end
}
