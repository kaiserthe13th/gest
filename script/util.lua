-- SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
-- SPDX-FileCopyrightText: Copyright 2026 Kerem Göksu

local M = {}

--- prepares a command to be run
---@param command string
---@return string
function M.prepareCommand(command)
    if os.host() == 'windows' then
        command = '"' .. command .. '"'
    end
    return command
end

--- returns path relative to the location of the main script
---@param p string
---@return string
function M.mpath(p, ...)
    return path.join(_MAIN_SCRIPT_DIR, p, ...)
end

return M