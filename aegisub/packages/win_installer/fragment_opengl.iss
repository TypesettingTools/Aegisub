; Copyright (c) 2007-2009, Niels Martin Hansen
;
; Redistribution and use in source and binary forms, with or without
; modification, are permitted provided that the following conditions are met:
;
;   * Redistributions of source code must retain the above copyright notice,
;     this list of conditions and the following disclaimer.
;   * Redistributions in binary form must reproduce the above copyright notice,
;     this list of conditions and the following disclaimer in the documentation
;     and/or other materials provided with the distribution.
;   * Neither the name of the Aegisub Group nor the names of its contributors
;     may be used to endorse or promote products derived from this software
;     without specific prior written permission.
;
; THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
; AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
; IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
; ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
; LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
; CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
; SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
; INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
; CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
; ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
; POSSIBILITY OF SUCH DAMAGE.
;
; -----------------------------------------------------------------------------
;
; AEGISUB
;
; Website: http://www.aegisub.org/
; Contact: mailto:nielsm@indvikleren.dk
;


; Check whether the user has a possibly redundant OPENGL32.DLL file in his program folder and offer to remove it

[InstallDelete]
Type: files; Name: {app}\opengl32.dll; Tasks: RemoveRedundantOpenGL
Type: files; Name: {app}\opengl32.txt; Tasks: RemoveRedundantOpenGL
Type: files; Name: {app}\opengl32.lib; Tasks: RemoveRedundantOpenGL

[Tasks]
Name: RemoveRedundantOpenGL; Description: Remove OPENGL32.DLL from Aegisub folder; Check: OpenGLdllPresent

[Messages]
WizardSelectTasks=Select Additional Tasks
SelectTasksDesc=Which additional tasks should be performed?
SelectTasksLabel2=You appear to have a Mesa3D OPENGL32.DLL file installed in your Aegisub directory. This file has previously helped make video display more stable, but is no longer needed because Aegisub's code has been made more robust.%nThis file will be removed by default, but if you want to keep it you can unselect the option below.

[Code]
function OpenGLdllPresent: Boolean;
begin
  try
    Result :=
      FileExists(ExpandConstant('{app}\opengl32.dll'))
      // MD5 hash of the DLL distributed on Aegisub's forum
      and (GetMD5OfFile(ExpandConstant('{app}\opengl32.dll')) = 'f928a03f4b265658589be951cbd09a27')
      ;
  except
    Result := False;
  end;
end;
[/Code]




