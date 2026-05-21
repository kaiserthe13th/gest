-- Copyright (c) Kerem Göksu 2026
-- SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

local util = require("script.util")
local M = {}

M.completionFile = util.mpath("data/.githooks_complete")

function M.setupGithooks()
    local success, exitCode = os.execute("git config core.hooksPath .githooks")
    if not success then
        print("Error: Unable to configure git hooks automatically: git exited with " .. exitCode)
    end
    print("Creating completion file...")
    io.close(io.open(M.completionFile, "w"))
end

newaction {
    trigger     = "githooks-setup",
    description = "Sets up git hooks",
    execute     = M.setupGithooks,
}

return M
