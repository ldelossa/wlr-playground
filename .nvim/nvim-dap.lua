local dap = require('dap')

dap.adapters.lldb = {
  type = 'executable',
  command = '/usr/bin/lldb-vscode', -- adjust as needed, must be absolute path
  name = 'lldb'
}

dap.configurations.c = {
  {
    name = 'main',
    type = 'lldb',
    request = 'launch',
    program = "main",
    cwd = '${workspaceFolder}',
    stopOnEntry = false,
    args = {},
  },
}
