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

/// Read all of the defined properties from the calling project file and stuff
/// them in a Map
let propertyMap (be : IBuildEngine) =
  use reader = Xml.XmlReader.Create(be.ProjectFileOfTaskNode)
  let project = new Project(reader)
  project.AllEvaluatedProperties
  |> Seq.filter (fun x -> not x.IsEnvironmentProperty)
  |> Seq.filter (fun x -> not x.IsGlobalProperty)
  |> Seq.filter (fun x -> not x.IsReservedProperty)
  |> Seq.map (fun x -> (x.Name, x.EvaluatedValue))
  |> Map.ofSeq

/// Convert an absolute windows path to an msys path
let mungePath path =
  let matchre pat str =
    let m = System.Text.RegularExpressions.Regex.Match(str, pat)
    if m.Success
    then List.tail [ for g in m.Groups -> g.Value ]
    else []
  match matchre "([A-Za-z]):\\\\(.*)" path with
  | drive :: path :: [] -> sprintf "/%s/%s" drive (path.Replace('\\', '/'))
  | _ -> path

type ShellWrapper(conf : ITaskItem) =
  inherit ToolTask()

  member val Arguments = "" with get, set
  member val WorkingDirectory = "" with get, set

  // ToolTask overrides
  override val ToolName = "sh.exe" with get
  override this.GenerateFullPathToTool() = conf.GetMetadata "Sh"
  override this.GenerateCommandLineCommands() = this.Arguments
  override this.GetWorkingDirectory() = this.WorkingDirectory

  override this.Execute() =
    if this.GenerateFullPathToTool() |> IO.File.Exists |> not then
      failwith "sh.exe not found. Make sure the MSYS root is set to a correct location."

    if not <| IO.Directory.Exists this.WorkingDirectory then ignore <| IO.Directory.CreateDirectory this.WorkingDirectory

    this.UseCommandProcessor <- false
    this.StandardOutputImportance <- "High"
    this.EnvironmentVariables <- [| for x in ["CC"; "CPP"; "CFLAGS"; "PATH"; "INCLUDE"; "LIB"]
                                    -> sprintf "%s=%s" <| x <| conf.GetMetadata(x).Replace("\n", "").Replace("        ", "") |]
    base.Execute()

type ExecShellScript() =
  inherit Task()

  // Task arguments
  [<Required>] member val WorkingDirectory = "" with get, set
  [<Required>] member val Command = "" with get, set
  [<Required>] member val Configuration : ITaskItem = null with get, set
  member val Arguments = "" with get, set

  member private this.realArgs () =
    let cleanArgs = this.Arguments.Replace("\r", "").Replace('\n', ' ')
    sprintf "-c '%s %s'" (mungePath this.Command) cleanArgs

  override this.Execute() =
    try
      let sw = ShellWrapper(this.Configuration,
                            BuildEngine = this.BuildEngine,
                            HostObject = this.HostObject,
                            Arguments = this.realArgs(),
                            WorkingDirectory = this.WorkingDirectory)

      sw.Execute()
    with
    | Failure(e) -> this.Log.LogError(e); false
    | e -> this.Log.LogErrorFromException(e); false

type MsysPath() =
  inherit Task()

  member val Path = "" with get, set
  [<Output>] member val Result = "" with get, set

  override this.Execute() =
    try
      this.Result <- mungePath this.Path
      true
    with Failure(e) ->
      this.Log.LogError(e)
      false

type UpdateFile() =
  inherit Task()

  member val File = "" with get, set
  member val Find = "" with get, set
  member val Replacement = "" with get, set

  override this.Execute() =
    try
      this.Log.LogMessage("Replacing '{0}' with '{1}' in '{2}'", this.Find, this.Replacement, this.File)
      let text = IO.File.ReadAllText(this.File).Replace(this.Find, this.Replacement)
      IO.File.WriteAllText(this.File, text)
      true
    with e ->
      this.Log.LogErrorFromException e
      false
