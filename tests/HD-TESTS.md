# HD presentation and asset probes

All test PNGs and manifests are synthetic. Keep save state and screenshot
outputs private; do not put them in the repository.

Configure one of these entries in a Windows Release build:

```powershell
cmake -S . -B build-hd-test -DCMAKE_BUILD_TYPE=Release `
  -DTMNT_RETAIL_WINDOWS=OFF -DTMNT_HD_TEST=gpu
cmake --build build-hd-test --config Release
.\build-hd-test\TmntIvTurtlesInTimeSNESRecomp.exe

cmake -S . -B build-hd-pack-test -DCMAKE_BUILD_TYPE=Release `
  -DTMNT_RETAIL_WINDOWS=OFF -DTMNT_HD_TEST=packs
cmake --build build-hd-pack-test --config Release
.\build-hd-pack-test\TmntIvTurtlesInTimeSNESRecomp.exe `
  .\hd-packs\author-example .\tests\invalid-hd-pack

cmake -S . -B build-hd-runtime -DCMAKE_BUILD_TYPE=Release `
  -DTMNT_RETAIL_WINDOWS=OFF -DTMNT_HD_TEST=runtime
cmake --build build-hd-runtime --config Release
```

The GPU probe creates a hidden OpenGL 3.3 context and checks Original nearest
against generated pixels, HD filters, the split view, atlas replacement, alpha,
brightness and the original-pixel mask at 512×448, 1080p and 4K. The pack probe
requires two directories: the included author example and a writable private
directory for malformed-manifest fixtures. The runtime probe supports the same
`ROM write|read state expected players [frames|nmi]` invocation as the save
probe and additionally checks that opening the graphics panel changes no
guest state or visible native frame.

For a runtime trace while loading a private save, set
`TMNT_HD_DIAGNOSTICS_ROOT` to a writable user data directory. Set
`TMNT_HD_PACK_ROOT` to a valid PNG pack to exercise cache lookup; the native
guest snapshots and PPU output must still match. `TMNT_HD_BENCH=1`
prints CPU frame time and process working set/commit for a 600-frame sample.
