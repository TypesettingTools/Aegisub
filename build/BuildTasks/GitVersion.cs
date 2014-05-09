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

using LibGit2Sharp;
using System;
using System.IO;
using System.Linq;
using System.Text.RegularExpressions;

namespace BuildTasks {
    public class GitVersion : Microsoft.Build.Utilities.Task {
        public string Root { get; set; }

        private static ObjectId LastSVNCommit = new ObjectId("16cd907fe7482cb54a7374cd28b8501f138116be");
        private const string versionHTemplate =
@"#define BUILD_GIT_VERSION_NUMBER {0}
#define BUILD_GIT_VERSION_STRING ""{1}""
#define TAGGED_RELEASE {2}
#define INSTALLER_VERSION ""{3}""
";
        private const string versionXmlTemplate =
@"<?xml version=""1.0"" encoding=""utf-8""?>
<Project ToolsVersion=""4.0"" xmlns=""http://schemas.microsoft.com/developer/msbuild/2003"">
  <PropertyGroup>
    <GitVersionNumber>{0}</GitVersionNumber>
    <GitVersionString>{1}</GitVersionString>
  </PropertyGroup>
</Project>
";

        private string UniqueAbbreviation(Repository repo, string full) {
            for (int len = 7; len < 40; ++len) {
                try {
                    repo.Lookup(full.Substring(0, len));
                    return full.Substring(0, len);
                }
                catch (AmbiguousSpecificationException) {
                    continue;
                }
            }
            return full;
        }

        private void WriteIfChanged(string path, string template, params object[] args) {
            var body = string.Format(template, args).Replace("\r\n", "\n");
            try {
                var oldBody = File.ReadAllText(path);
                if (body != oldBody)
                    File.WriteAllText(path, body);
            }
            catch (IOException) {
                File.WriteAllText(path, body);
            }
        }

        public override bool Execute() {
            string versionHPath = Root + "build/git_version.h";
            string versionXmlPath = Root + "build/git_version.xml";

            if (!Directory.Exists(Root + ".git")) {
                if (File.Exists(versionHPath)) {
                    Log.LogMessage("Using cached version.h");
                    return true;
                }
                Log.LogError("git repo not found and no cached git_version.h");
                return false;
            }

            int commits = 6962; // Rev ID when we switched away from SVN
            string installerVersion = "0.0.0";
            string versionStr = null;
            bool taggedRelease = false;
            using (var repo = new Repository(Root + ".git")) {
                commits += repo.Commits.TakeWhile(c => !c.Id.Equals(LastSVNCommit)).Count();

                foreach (var tag in repo.Tags) {
                    if (!tag.Target.Id.Equals(repo.Head.Tip.Id)) continue;

                    taggedRelease = true;
                    versionStr = tag.Name;
                    if (versionStr.StartsWith("v")) versionStr = versionStr.Substring(1);
                    if (Regex.Match(versionStr, @"(\d)\.(\d)\.(\d)").Success)
                        installerVersion = versionStr;
                    break;
                }

                if (versionStr == null) {
                    string branch = repo.Head.Name ?? "(unnamed branch)";
                    versionStr = string.Format("{0}-{1}-{2}", commits, branch,
                        UniqueAbbreviation(repo, repo.Head.Tip.Sha.ToString()));
                }
            }

            WriteIfChanged(versionHPath,  versionHTemplate, commits, versionStr, taggedRelease ? "1" : "0", installerVersion);
            WriteIfChanged(versionXmlPath,  versionXmlTemplate, commits, versionStr);

            return true;
        }
    }
}
