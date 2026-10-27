# Save integration tests

Use a legally owned, verified USA ROM. States and pictures are private test
outputs: do not commit or distribute them.

With the generated game sources available, configure a separate Release tree:

```powershell
cmake -S . -B build-save-test -DCMAKE_BUILD_TYPE=Release -DTMNT_RETAIL_WINDOWS=OFF -DTMNT_SAVE_RUNTIME_PROBE=ON
cmake --build build-save-test --config Release
mkdir save-test-output
cd save-test-output
$probe = '../build-save-test/TmntIvTurtlesInTimeSNESRecomp.exe'
& $probe keys
& $probe 'C:/private/game.sfc' write checkpoint.state expected.txt 1 180
& $probe 'C:/private/game.sfc' read checkpoint.state expected.txt 1
```

The `read` command starts another process with a newly initialized SNES. It
loads the file without booting the game first. Every one of the next 180
complete machine snapshots and rendered frames must match the reference.
The host PCM delivery ring is excluded; all guest DSP/SPC state remains in the
comparison. Then 120 successive boundaries are each captured, advanced,
restored and advanced again. A divergence returns a nonzero exit code.
The probe also opens, navigates, renders and cancels the save overlay, checking
that the complete snapshot and visible frame remain unchanged. This direct API
test does not exercise the desktop modal loop, audio device or SDL focus events.

Repeat in separate output directories with player count `2`, action frames
`0` (first-stage checkpoint), and action selector `nmi` (find a suspended NMI
checkpoint). The two-player script joins and confirms player two using Start,
without WRAM pokes. Inspect `checkpoint.ppm`, `baseline-after.ppm` and
`restored-after.ppm` to verify the actual game phase and active players.

The probe also tests rejected corruption, truncation, ROM hash, schema and game
identity while asserting that the live machine is unchanged. `keys` checks
F5/F9, remapping, legacy conflicts and all six file paths.

Windows fault-injection test, in a VS x64 developer prompt:

```powershell
cl /std:c11 /O2 /MD /D_CRT_SECURE_NO_WARNINGS /I../snesrecomp/runner/src ../tests/atomic_state_test.c /Fe:atomic-test.exe
./atomic-test.exe
```

Run that command from `save-test-output`. It checks short writes, failed
flush/close/replace, a locked destination, process death during writing, a
subsequent successful save and Unicode paths. The production writer is compiled
directly into the test, with only its Windows I/O calls intercepted.

Framework regressions: `tests/snapshot_guard/test_transaction.py`, the host
save browser tests (also with `SNESRECOMP_PERSISTENT_SAVE_PROFILE=1` and
`SNES_SAVESTATE_SLOTS=5`), and `tests/interp816/run.sh`. The S21 bridge case
specifically covers an NMI waiting for a later NMI, both RTIs and the main PC.

These tests do not certify Windows reboot persistence, installed-executable
integration, physical controllers, audio listening or complete campaigns.
