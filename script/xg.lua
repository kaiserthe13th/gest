-- SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
-- SPDX-FileCopyrightText: Copyright 2026 Kerem Göksu

local util = require("script.util")

newaction {
    trigger     = "xg",
    description = "Runs the licensing checker: xg.",
    execute     = function()
        local target_notice = util.mpath(path.join("NOTICE.xg"))
        local script_dir = util.mpath("script")
        local xg_file = util.mpath(path.join("script", "xg.py"))
        
        io.write("Selecting tool... ")
        local cmd
        if _OPTIONS['no-uv'] then
            print("Using: python [" .. PYTHON .. "]")
            cmd = string.format('"%s" "%s" -vall "%s"', PYTHON, xg_file, target_notice)
        else
            print("Using: uv")
            cmd = string.format('uv run --project "%s" "%s" -vall "%s"', script_dir, xg_file, target_notice)
        end
        print("Running script/xg.py")

        cmd = util.prepareCommand(cmd)

        local success = os.execute(cmd)
        if not success then
            print("\nError: Program exit unsuccessfully.")
            os.exit(1)
        end
    end
}