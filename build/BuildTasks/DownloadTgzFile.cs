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

namespace BuildTasks {
    public class DownloadTgzFile : Microsoft.Build.Utilities.Task {
        public string Url { get; set; }
        public string OutputFile { get; set; }
        public string Hash { get; set; }

        private void DownloadArchive(string url, string unpackDest) {
            var downloadStream = new System.Net.WebClient().OpenRead(url);
            var gzStream = new ICSharpCode.SharpZipLib.GZip.GZipInputStream(downloadStream);
            using (var file = System.IO.File.Create(unpackDest)) {
                gzStream.CopyTo(file);
            }
        }

        public override bool Execute() {
            try {
                using (var fs = System.IO.File.OpenRead(this.OutputFile)) {
                    var hash = new System.Security.Cryptography.SHA1Managed().ComputeHash(fs);
                    if (System.BitConverter.ToString(hash).Replace("-", "").ToLower() == this.Hash)
                        return true;
                }
            }
            catch (System.IO.IOException) {
                // Need to download if file not present or not readable
            }

            try {
                DownloadArchive(this.Url, this.OutputFile);
            }
            catch (System.Exception e) {
                this.Log.LogErrorFromException(e);
                return false;
            }

            return true;
        }
    }
}
