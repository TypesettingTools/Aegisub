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

using ICSharpCode.SharpZipLib.Tar;
using System;
using System.IO;
using System.Security.Cryptography;

namespace BuildTasks {
    public class TarballProject : Microsoft.Build.Utilities.Task {
        public string Url { get; set; }
        public string Root { get; set; }
        public string Hash { get; set; }

        private bool NeedsUpdate() {
            try {
                return Hash != File.ReadAllText(Path.Combine(Root, "aegisub.hash"));
            }
            catch (IOException) {
                return true;
            }
        }

        private void ExtractEntry(string destDir, TarEntry entry, ICSharpCode.SharpZipLib.Tar.TarInputStream stream) {
            string name = entry.Name;
            if (Path.IsPathRooted(name))
                name = name.Substring(Path.GetPathRoot(name).Length);
            name = name.Replace('/', Path.DirectorySeparatorChar);
            name = name.Substring(name.IndexOf(Path.DirectorySeparatorChar) + 1);

            string dest = Path.Combine(destDir, name);
            if (entry.IsDirectory)
                Directory.CreateDirectory(dest);
            else {
                Directory.CreateDirectory(Path.GetDirectoryName(dest));
                using (Stream outputStream = File.Create(dest)) {
                    stream.CopyEntryContents(outputStream);
                }
            }
        }

        public override bool Execute() {
            if (!NeedsUpdate()) return true;

            try {
                var ms = new MemoryStream();
                var downloadStream = new System.Net.WebClient().OpenRead(Url);
                downloadStream.CopyTo(ms);
                ms.Seek(0, SeekOrigin.Begin);

                var hash = new SHA256Managed().ComputeHash(ms);
                if (BitConverter.ToString(hash).Replace("-", "").ToLower() != this.Hash) {
                    Log.LogError("Got wrong hash for {0}", Url);
                    return false;
                }

                try {
                    Directory.Delete(Root, true);
                }
                catch (DirectoryNotFoundException) {
                    // Obviously not an issue
                }

                ms.Seek(0, SeekOrigin.Begin);
                var bzStream = new ICSharpCode.SharpZipLib.BZip2.BZip2InputStream(ms);
                var tarStream = new ICSharpCode.SharpZipLib.Tar.TarInputStream(bzStream);
                while (true) {
                    TarEntry entry = tarStream.GetNextEntry();
                    if (entry == null) break;
                    ExtractEntry(Root, entry, tarStream);
                }

                File.WriteAllText(Path.Combine(Root, "aegisub.hash"), Hash);

                return true;
            }
            catch (Exception e) {
                Log.LogErrorFromException(e);
                return false;
            }
        }
    }
}
