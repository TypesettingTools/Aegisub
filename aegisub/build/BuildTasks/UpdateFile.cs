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

using Microsoft.Build.Utilities;
using System;

namespace BuildTasks {
    public class UpdateFile : Task {
        public string File { get; set; }
        public string Find { get; set; }
        public string Replacement { get; set; }

        public override bool Execute() {
            try {
              this.Log.LogMessage("Replacing '{0}' with '{1}' in '{2}'",
                  this.Find, this.Replacement, this.File);
              var text = System.IO.File.ReadAllText(this.File).Replace(this.Find, this.Replacement);
              System.IO.File.WriteAllText(this.File, text);
              return true;
            }
            catch (Exception e) {
                this.Log.LogErrorFromException(e);
                return false;
            }
        }
    }
}
