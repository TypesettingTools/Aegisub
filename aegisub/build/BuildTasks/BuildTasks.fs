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

module BuildTasks

open System
open System.Diagnostics
open Microsoft.Build.Evaluation
open Microsoft.Build.Framework
open Microsoft.Build.Utilities

exception ShellException of string

let searchPath file =
  Environment.GetEnvironmentVariable("path").Split ';'
  |> Seq.map (fun p -> IO.Path.Combine(p, file))
  |> Seq.filter IO.File.Exists
  |> Seq.append [""]
  |> Seq.nth 1

let propertyMap (be : IBuildEngine) =
  use reader = Xml.XmlReader.Create(be.ProjectFileOfTaskNode)
  let project = new Project(reader)
  project.AllEvaluatedProperties
  |> Seq.filter (fun x -> not x.IsEnvironmentProperty)
  |> Seq.filter (fun x -> not x.IsGlobalProperty)
  |> Seq.filter (fun x -> not x.IsReservedProperty)
  |> Seq.map (fun x -> (x.Name, x.EvaluatedValue))
  |> Map.ofSeq

type ShellWrapper(props : Map<String, String>) =
  let sh =
    match props.TryFind "MsysBasePath" with
    | None | Some "" -> searchPath "sh.exe"
    | Some path -> sprintf "%s\\bin\\sh.exe" path

  let cwd = function
  | null | "" -> props.["AegisubSourceBase"]
  | x -> x

  member this.call scriptName workingDir =
    if not <| IO.File.Exists sh then
      raise <| ShellException "sh.exe not found. Make sure the MSYS root is set to a correct location."

    let info = new ProcessStartInfo(FileName = sh
                                  , Arguments = scriptName
                                  , WorkingDirectory = cwd workingDir
                                  , RedirectStandardOutput = true
                                  , UseShellExecute = false)

    use p = new Process(StartInfo = info)
    ignore(p.Start())
    p.WaitForExit()
    if p.ExitCode <> 0 then
      raise <| ShellException(p.StandardOutput.ReadToEnd())

type ExecShellScript() =
  inherit Task()

  member val WorkingDirectory = "" with get, set
  member val Command = "" with get, set

  override this.Execute() =
    try
      let sw = ShellWrapper (propertyMap this.BuildEngine)
      this.Log.LogMessage("Calling '{0}'", this.Command);
      sw.call this.Command this.WorkingDirectory
      true
    with ShellException(e) ->
      this.Log.LogError(e)
      false
