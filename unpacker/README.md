# Dino Crisis 2 collection unpacker with animations

This modified DexeTech build parses DAT archives by their structure, finds embedded
rigged models, and embeds validated matching animation records in glTF outputs.
It accepts files or folders; folders are searched recursively for DATs.

```bat
dino2-unpacker.exe --output-dir=Extracted --save-chunks "C:\Game\Data"
dino2-unpacker.exe --in-place --output-dir=Extracted E80.DAT
dino2-unpacker.exe --animation-tps=30 --output-dir=Extracted E30.DAT
```

Animations default to an assumed 60 ticks/second. `--in-place` omits animated root
translation; `--no-animations` exports static rigs. `--save-chunks` retains all raw
archive entries and decompressed payloads. Unsupported containers are retained raw
and reported explicitly. FILE/ITEM image galleries and MAP raw sectors are handled.

The supplied collection of 375 DATs passed extraction checks with no failures.
All 13 enemy DATs export animations. The Windows package includes a README with
the complete test results, limitations, DLLs and licensing notices.

Motion tables and sequential records are validated independently. Records for a
different rig never get truncated or assigned guessed bone indices. Embedded models
are discovered by absolute pointers and validated geometry/hierarchy, rather than
by a creature filename list. Per-file extraction and per-model animation JSON reports
make partial recovery visible. Unknown bytes remain available for investigation.

This does not establish universal decoding of every possible motion format.
Unmapped secondary motions remain unbound. Multi-actor scenes can share a motion
library when actors have the same joint count; exact actor assignment is unverified.
New model playback, engine timing, event metadata and final-frame hold need visual
or engine validation. E30's motions were user-tested in Blender and remain unchanged.
Dino Crisis 1 and console formats are outside the tested scope.

## Build

Use a C++17 compiler, CMake and Qt 6 Core/Gui. On Windows, use a compiler matching Qt.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

Use `windeployqt` to collect Qt runtime dependencies when distributing a new build.
`build-windows.cmd` provides a CMake wrapper. No game assets are bundled.

## Tests

```sh
g++ -std=c++17 tests/animation_decoder.cpp unpacker/DC2Animations.cpp -o decoder-test
./decoder-test /path/to/your/E30.DAT.model.0.decompressed
python tests/integration_e30.py UNPACKER E30.DAT VALIDATED_E30.glb
```

Tests require your own input files. They check the E30 motion baseline, options,
invalid/truncated records and recovery of unaffected clips. The integration test
compares geometry, the rig and all 543 keys with the previously validated export.

## Provenance

Source: https://github.com/DexeTech/dino2-unpacker
Baseline commit: `1fb591f03480499cb9a783e3807f4157e5540cd0`.
Program and complete modified source are GPL-3.0; see LICENSE.
