// Copyright (c) 2014, Thomas Goyne <plorkyeran@aegisub.org>
//
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//
// Aegisub Project http://www.aegisub.org/

using Microsoft.Build.Framework;
using Microsoft.Build.Utilities;
using System;
using System.IO;
using System.Linq;

namespace BuildTasks {
    public class ShellWrapper : ToolTask {
        private ITaskItem _conf;

        public string Arguments { get; set; }
        public string WorkingDirectory { get; set; }

        protected override string ToolName { get { return "sh.exe"; } }
        protected override string GenerateFullPathToTool() { return _conf.GetMetadata("Sh"); }
        protected override string GenerateCommandLineCommands() { return this.Arguments; }
        protected override string GetWorkingDirectory() { return this.WorkingDirectory; }

        public ShellWrapper(ITaskItem conf) {
            _conf = conf;
        }

        public override bool Execute() {
            if (!File.Exists(this.GenerateFullPathToTool()))
                throw new Exception("sh.exe not found. Make sure the MSYS root is set to a correct location.");
            if (!Directory.Exists(this.WorkingDirectory))
                Directory.CreateDirectory(this.WorkingDirectory);

            this.UseCommandProcessor = false;
            this.StandardOutputImportance = "High";
            this.EnvironmentVariables =
                new string[] {"CC", "CPP", "CFLAGS", "PATH", "INCLUDE", "LIB"}
                .Select(x => string.Format("{0}={1}", x, _conf.GetMetadata(x).Replace("\n", "").Replace("        ", "")))
                .ToArray();

            return base.Execute();
        }
    }
}
