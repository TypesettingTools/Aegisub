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

let downloadArchive (url : String) unpackDest =
  use wc = new Net.WebClient()
  use downloadStream = wc.OpenRead url
  use gzStream = new ICSharpCode.SharpZipLib.GZip.GZipInputStream(downloadStream)
  use tarStream = new ICSharpCode.SharpZipLib.Tar.TarInputStream(gzStream)
  use tarArchive = ICSharpCode.SharpZipLib.Tar.TarArchive.CreateInputTarArchive tarStream
  tarArchive.ExtractContents unpackDest

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
