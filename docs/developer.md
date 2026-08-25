Developer notes
===============

This document records developer-focused build knobs and platform-targeting details that are intentionally hidden or not shown to end users in the main README.

Windows SDK target (_WIN32_WINNT)
--------------------------------
- The project defines the Windows SDK macro via a Meson option called `windows_target` (internal). This maps to a compile-time define: `-D_WIN32_WINNT=<value>`.
- Typical values:
  - 0x0601 = Windows 7 / Windows Server 2008 R2
  - 0x0A00 = Windows 10
  - Newer values correspond to newer OS feature sets; consult Microsoft docs for exact mapping.

Notes:
- This setting only affects which APIs are exposed at compile-time. It does not change which OS versions a built binary can run on at runtime — runtime compatibility depends on which APIs you actually call and whether you guard newer APIs with runtime checks.
- Keep this value conservative unless you intentionally require newer APIs.

Meson options (developer)
-------------------------
- windows_target (string, default: 0x0A00)
  - Internal: adjusts the `_WIN32_WINNT` value passed to the compiler. End users typically do not need to change this.
  - To change for local testing: `meson setup build --reconfigure -Dwindows_target=0x0A00`.
  - See: meson_options.txt

- vs_toolset (string, default: latest)
  - When generating Visual Studio solutions with Meson's VS backend, you may request a toolset with `-Dvs_toolset=vXYZ` or `-Dvs_toolset=latest`.
  - Example: `meson setup vs_build --backend vs -Dvs_toolset=latest`.

Visual Studio 2026 notes
------------------------
- The project has been prepared to upstream support for Visual Studio 2026 by adding the `vs_toolset` option and a best-effort CI matrix entry.
- If VS 2026 uses a new toolset name (e.g., `v2026`), set `-Dvs_toolset=v2026` when generating the solution.
- Meson will still rely on the Visual Studio and Windows SDK installed on the developer machine.

CI considerations
-----------------
- CI contains a best-effort matrix entry to exercise the requested VS toolset. GitHub runners may not provide a native VS 2026 image yet; the job will attempt to use the installed Visual Studio on the runner.
- For strict enforcement of a toolset or SDK, consider adding a CI step to fail the job when the required components are not present. This is intentionally not enforced in mainline CI to avoid blocking contributors who may not have the new toolset available.

When to modify these settings
-----------------------------
- If you are implementing Windows-11-only features and the APIs are gated behind a new `_WIN32_WINNT` value, update `windows_target` (and document the change) and ensure CI and contributor environments have the proper SDK.
- For most contributions, neither `windows_target` nor `vs_toolset` need changing.

Contact
-------
If you change these values or make SDK/toolset-dependent code changes, notify the maintainers and update CI/documentation accordingly.