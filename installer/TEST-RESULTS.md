# Validation on 2026-10-27

Windows x64, MSVC 19.44, Release, SDL3 static, Inno Setup 6.7.3.

## Current installer revision

Release build and Inno Setup compilation passed after restoring the launcher,
adding the mandatory installer ROM page, cover, icon and fullscreen default.
The silent installer without `/ROM` was executed and rejected with exit code 1.
The staged cover SHA-256 matches the original launcher cover. Payload scanning
found no ROM or ROM cache, and the executable icon was extracted successfully.
The complete interactive installation/import/gameplay flow has not been rerun
after these changes. The integration script now supplies `/ROM` and checks the
installed payload; a game window after installation is the launcher.

## Earlier revision (before the mandatory installer ROM page)

Executed `test-rom.ps1` against the real locally supplied USA ROM:

| Case | Result |
|---|---|
| Original USA ROM | accepted |
| Same content, different Unicode filename | accepted |
| USA image with one changed byte | rejected |
| Synthetic image with another region tag | rejected |
| Invalid text file / wrong size | rejected |
| Correct image plus 512-byte copier header | rejected |
| Missing file | rejected |

Executed `test-install.ps1` against the generated installer:

| Case | Result |
|---|---|
| Fresh installation without ROM | native picker opened |
| Cancel picker | clean exit, status 0 |
| Import correct ROM | canonical copy in LocalAppData, SHA-256 matched |
| Runtime payload | required Visual C++ DLLs present |
| Uninstall | executable removed, user ROM preserved |
| Reinstall and run | cached ROM found, game window opened and closed cleanly |

The integration test ran on the development machine in a fresh application data
directory. It is a boot smoke test, not a gameplay regression test. Full clean-VM
qualification without development tools, a real other-region ROM, visual review
of bilingual error dialogs and optional desktop shortcut remain manual release
checks described in README.md. The file picker was opened/cancelled by the test;
the positive import used the same import implementation via `--import-rom`.

No test fixture ROM or extracted game data is included in the source or installer.
