// Copyright (c) 2012, Thomas Goyne <plorkyeran@aegisub.org>
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

module DependencyFetchers

open System
open System.Diagnostics
open System.Linq
open Microsoft.Build.Evaluation
open Microsoft.Build.Framework
open Microsoft.Build.Utilities

type GitWrapper(path : String) =
  inherit ToolTask()

  member val Arguments = "" with get, set
  member val WorkingDirectory = "" with get, set

  // ToolTask overrides
  override val ToolName = "git.exe" with get
  override this.GenerateFullPathToTool() = path
  override this.GenerateCommandLineCommands() = this.Arguments
  override this.GetWorkingDirectory() = this.WorkingDirectory

  override this.Execute() =
    if this.GenerateFullPathToTool() |> IO.File.Exists |> not then
      failwith "git.exe not found. Make sure the MSYS root is set to a correct location."

    this.UseCommandProcessor <- false
    this.StandardOutputImportance <- "High"
    base.Execute()

type GitProject() =
  inherit Task()

  member val Projects : ITaskItem[] = null with get, set
  member val Root = "" with get, set
  member val GitPath = "" with get, set

  override this.Execute() =
    let callGit dir args =
      let gw = GitWrapper(this.GitPath,
                          BuildEngine = this.BuildEngine,
                          HostObject = this.HostObject,
                          Arguments = args,
                          WorkingDirectory = dir)
      if not <| gw.Execute() then failwith "git failed"

    let update (projectName : String) directory url =
      this.Log.LogMessage ("Updating {0}", projectName)
      callGit directory "pull --rebase"
      callGit directory "clean -xfd"

    let fetch (projectName : String) root (url : ITaskItem) =
      this.Log.LogMessage ("Fetching {0}", projectName)
      ignore <| IO.Directory.CreateDirectory root

      callGit root (sprintf "clone %s --no-checkout" url.ItemSpec)

      let dir = (sprintf "%s\\%s" root projectName)

      let autocrlf = url.GetMetadata "AutoCrlf"
      if autocrlf.Length > 0
      then callGit dir (sprintf "config --local core.autocrlf %s" autocrlf)

      let branch = match url.GetMetadata("Branch") with "" -> "master" | x -> x
      callGit dir (sprintf "checkout %s" branch)

    let updateGit (url : ITaskItem) =
      let projectName = Uri(url.ItemSpec).Segments.Last().Replace(".git", "")
      let directory = sprintf "%s\\%s" this.Root projectName

      if IO.Directory.Exists directory
      then update projectName directory url
      else fetch projectName this.Root url

    this.Log.LogMessage ("Using git at {0}", this.GitPath)
    try
      this.Projects |> Array.map updateGit |> ignore
      true
    with e ->
      this.Log.LogErrorFromException e
      false

let downloadArchive (url : String) unpackDest =
  use wc = new Net.WebClient()
  use downloadStream = wc.OpenRead url
  use gzStream = new ICSharpCode.SharpZipLib.GZip.GZipInputStream(downloadStream)
  use tarStream = new ICSharpCode.SharpZipLib.Tar.TarInputStream(gzStream)
  use tarArchive = ICSharpCode.SharpZipLib.Tar.TarArchive.CreateInputTarArchive tarStream
  tarArchive.ExtractContents unpackDest

type TarballProject() =
  inherit Task()

  member val Projects : ITaskItem[] = null with get, set
  member val Root = "" with get, set

  override this.Execute() =
    let needsUpdate directory version =
      try
        not <| String.Equals(sprintf "%s\\version.aegisub" directory |> IO.File.ReadAllText, version)
      with | :? IO.IOException -> true

    let update directory (project : ITaskItem) version =
      try IO.Directory.Delete(directory, true) with | :? IO.IOException -> ()

      this.Log.LogMessage ("Downloading {0} {1} from {2}", project.ItemSpec, version, project.GetMetadata "Url")
      downloadArchive (project.GetMetadata "Url") (sprintf @"%s\.." directory)

      let dirname = project.GetMetadata "DirName"
      if not <| String.IsNullOrWhiteSpace dirname
      then IO.Directory.Move(dirname |> sprintf @"%s\..\%s" directory, directory)

      IO.File.WriteAllText(sprintf @"%s\version.aegisub" directory, version)

    let check (project : ITaskItem) =
      let directory = sprintf "%s\\%s" this.Root project.ItemSpec
      let version = project.GetMetadata "Version"

      if needsUpdate directory <| version
      then update directory project version
      else this.Log.LogMessage <| sprintf "%s is up to date" project.ItemSpec

    try
      this.Projects |> Array.map check |> ignore
      true
    with e ->
      this.Log.LogErrorFromException e
      false

type DownloadTgzFile() =
  inherit Task()

  member val Url = "" with get, set
  member val Destination = "" with get, set
  member val OutputFile = "" with get, set
  member val Hash = "" with get, set

  override this.Execute() =
    let needsDownload =
      try
        use fs = IO.File.OpenRead this.OutputFile
        let sha = new Security.Cryptography.SHA1Managed ()
        let hash = sha.ComputeHash fs
        BitConverter.ToString(hash).Replace("-", "") <> this.Hash
      with | :? IO.IOException -> true

    try
      if needsDownload
      then downloadArchive this.Url this.Destination
      true
    with e ->
      this.Log.LogErrorFromException e
      false
