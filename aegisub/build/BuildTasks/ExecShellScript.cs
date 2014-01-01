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

namespace BuildTasks {
    public class ExecShellScript : Task {
        [Required] public string WorkingDirectory { get; set; }
        [Required] public string Command { get; set; }
        [Required] public ITaskItem Configuration { get; set; }
        public string Arguments { get; set; }

        private string RealArgs() {
            return string.Format("-c '{0} {1}'", Utils.MungePath(this.Command),
                (this.Arguments ?? "").Replace("\r", "").Replace('\n', ' '));
        }

        public override bool Execute() {
            try {
                var sw = new ShellWrapper(this.Configuration);
                sw.BuildEngine = this.BuildEngine;
                sw.HostObject = this.HostObject;
                sw.Arguments = this.RealArgs();
                sw.WorkingDirectory = this.WorkingDirectory;
                return sw.Execute();
            }
            catch (Exception e) {
                this.Log.LogErrorFromException(e);
                return false;
            }
        }
    }
}
