# dino2-unpacker

Utility for extracting data from Dino Crisis 2, for both the PC and PlayStation versions.

Drag files or folders onto `dino2-unpacker.exe`, or pass them on the command line. Folders are
searched recursively. Each file's version is detected automatically, and the output is written
next to it (or into `--output-dir`).

## SUPPORTED FILES

| FILE PATTERN | SUPPORTED | NOTES                                                                                            |
| ------------ | --------- | ------------------------------------------------------------------------------------------------ |
| \*.PXL       | ✅        | Saves images as PNGs                                                                             |
| \*.TEX       | ✅        | Saves images as PNGs                                                                             |
| ARMOR\*.DAT  | 🔨        | Implemented dechunking                                                                           |
| CAPLOGO.DAT  | ❌        |                                                                                                  |
| COMING.DAT   | ✅        | RGB555 image with pallette                                                                       |
| CONTINUE.DAT | ❌        |                                                                                                  |
| CORE\*.DAT   | 🔨         | WAVE files (DONE), character models (DONE), RGB555 textures                                      |
| DEMO\*.TRG   | ❌        |                                                                                                  |
| DOOR\*.DAT   | 🔨        | Extracts textures and soudns and most of 3d models (excpetion is DOOR1900.DAT)                   |
| E\*.DAT      | ✅        | Textured model as OBJ and rigged glTF (DONE), texture page PNG (DONE), animations (DONE)         |
| ENDING.DAT   | ✅        | WAVE files                                                                                       |
| ENTRY.DAT    | 🔨        | Dino colliseum portraits, some data after                                                        |
| FILE.DAT     | ✅        | Dino File images stored as RGB555 with pallette                                                  |
| GAMEOVER.DAT | ❌        |                                                                                                  |
| ITEM.DAT     | ✅        | Saves images as PNGs                                                                             |
| KOF\_\*.DAT  | ✅         | WAVE files (DONE), Dino Duel characters as rigged glTF + OBJ with animations and textures (DONE) |
| LOAD.DAT     | ❌        |                                                                                                  |
| MAP.BIN      | ❌        |                                                                                                  |
| MAP.DAT      | ❌        | RGB555 images with pallette                                                                      |
| MAP_ST\*.DAT | ❌        | RGB555 images with pallette                                                                      |
| ME\_\*.DAT   | ❌        | LAME MP3 Files (Enemy Music)                                                                     |
| MF\_\*.DAT   | ❌        | LAME MP3 Files (Background Music)                                                                |
| M_RESULT.DAT | ❌        | Possibly LAME MP3 Files                                                                          |
| MS\_\*.DAT   | ❌        | LAME MP3 Files (Background Sounds)                                                               |
| M_TITLE.DAT  | 🔨        | MP3 file at top, WAVE files at bottom (DONE)                                                     |
| OPENING.DAT  | ❌        |                                                                                                  |
| OPTION.DAT   | ❌        |                                                                                                  |
| RES\*.DAT    | ❌        |                                                                                                  |
| RESULT.DAT   | 🔨         | WAVE files (DONE), Colosseum results mesh as OBJ and texture pages (DONE)                        |
| SAVE.DAT     | ❌        |                                                                                                  |
| SC\*.DAT     | 🔨        | WAVE files (DONE), RGB555 with pallette                                                          |
| ST\*.DAT     | 🔨        | Mix of files: RGB555 with palletee, WAVE files (DONE) and others                                 |
| ST\*.DBS     | ✅         | Room backgrounds: JPGs (PC); PNGs decoded from MDEC frames (PlayStation)                         |
| SUBSCR3.DAT  | ❌        |                                                                                                  |
| SUBSCR6.DAT  | ✅        | RGB555 image with pallette, contains data for boat ride selection screeen                        |
| TITLE.DAT    | ✅        | WAVE files, maybe something else hides here                                                      |
| TITLE2.DAT   | ✅        | WAVE files, maybe something else hides here                                                      |
| WARNING.DAT  | ❌        |                                                                                                  |
| WEP\_\*.DAT  | ✅         | WAVE files (DONE), textured model as OBJ and rigged glTF (DONE), texture page PNG (DONE), animations (DONE) |
| WP\*.DAT     | 🔨        | Saves images as PNGs, additional data after images for 75A, 79A, 83A, 84A                        |


## ENEMY ANIMATION COVERAGE

Compatible animation export was checked for:

`E00.DAT`, `E10.DAT`, `E20.DAT`, `E30.DAT`, `E31.DAT`, `E32.DAT`, `E40.DAT`, `E50.DAT`, `E60.DAT`, `E70.DAT`, `E80.DAT`, `E90.DAT` and `EA0.DAT`.

`E32.DAT` contains an additional embedded rig, which exports separately with 17 compatible clips. Each exported model uses its own hierarchy and joint count.

The decoder handles pointer tables, repeated motion pointers, empty slots, multiple record sizes, aligned or packed joint rotations, and validated sequential records. A different-rig record no longer causes all compatible clips to be discarded.

Numeric names such as `Clip_00` preserve source indices. Behavior names such as idle, walk, attack and death are not guessed.


## PC AND PLAYSTATION VERSIONS

Both versions use the same file names and the same DAT container (a 0x800-byte table of
0x20-byte entries, each payload padded to 0x800 bytes), but some contents are stored
differently. The extractor reads the container header to tell them apart and prints the
version it detected:

| CONTENT                    | PC                            | PLAYSTATION                                          |
| -------------------------- | ----------------------------- | ---------------------------------------------------- |
| Model + animation block    | type 5, loaded at 0x6xxxxx    | type 7, loaded at 0x80xxxxxx                          |
| Character program code     | -                             | further type-7 blocks (skipped)                      |
| Compressed texture         | type 6                        | type 8                                               |
| Sounds                     | type 3: bank of RIFF WAVs     | type 3 "Gian" header + type 4 SPU ADPCM samples      |
| Room backgrounds (.DBS)    | JPEG                          | MDEC "BS" v2 frames                                  |

Texture-only files (.TEX, .PXL, ENTRY, COMING, SC\*, ITEM ...) are identical on both.

PlayStation sounds are decoded to 16-bit 22050 Hz WAVs. Where both versions have a sound it is
the same recording, sample for sample; some PlayStation banks hold a few extra samples, so
the numbering can differ. PlayStation backgrounds are decoded to
320x240 PNGs, the same images as the PC JPEGs. Models, animations and textures export
identically from both versions. The one difference found is real: two bytes of the colour
palette in WP00A.DAT differ between the releases.

The PlayStation files can be copied off a disc image with any ISO tool; they are under
`/PSX/DATA`. Tested with the USA release (SLUS-01279).


## CHARACTER MODEL TEXTURES

Each character model (E\*.DAT, WEP\_\*.DAT) names the texture page it samples through the
tpage field in its header, which is a position in the PlayStation's video memory. The game
fills that position from whichever files are loaded, so a model's texture is not always in
its own file:

- Most files carry the page themselves: 256x256 at 8 bits per pixel, or 128x128 for small
  characters such as the Compsognathus (EA0.DAT, WEP_PC12.DAT).
- The default-outfit player models WEP_P000.DAT (Regina) and WEP_P100.DAT (Dylan) have no
  body texture. It comes from WP00A.DAT / WP10A.DAT. When the matching WP\*A.DAT is in the same
  folder, the extractor uses it (WEP_Pabc.DAT -> WPacA.DAT).
- E00.DAT (Velociraptor) and E90.DAT (Oviraptor) have no texture at all. They use the shared
  enemy page, which the room files (SC\*.DAT, ST502.DAT) load. It differs from room to room,
  so there is no single right file, and these two models are exported untextured.


## KNOWN LIMITATIONS

- Some secondary motion records remain unbound because their target rig is unverified.
- Multi-actor scenes can contain a shared motion library for actors with the same joint count. Exact original actor/sequence assignments remain unverified.
- Exact engine playback timing, final-key hold, interpolation and event metadata remain unverified.
- Some archive payloads are retained raw rather than converted to a complete asset format.
- Dedicated MP3 detection/decoding and many menu, screen and map formats remain unresolved.
- Some texture pages come from other archives and are not selected automatically.
- Tested with the Dino Crisis 2 PC (Steam) release and the PlayStation USA release (SLUS-01279). Dino Crisis 1 is not supported.
- PlayStation music and ambience (ME\_, MF\_, MS\_\*.DAT) are extracted as their individual SPU samples, not as finished tracks.
- This build does not claim universal decoding of every possible DAT or animation format.


## COMMAND-LINE USAGE

```bat
dino2-unpacker.exe E40.DAT
dino2-unpacker.exe --output-dir=Extracted "C:\Game\Data"
dino2-unpacker.exe --output-dir=Extracted --save-chunks "C:\Game\Data"
dino2-unpacker.exe --output-dir=Extracted E30.DAT
dino2-unpacker.exe --in-place --output-dir=Extracted E80.DAT
dino2-unpacker.exe --animation-tps=30 --output-dir=Extracted E30.DAT
dino2-unpacker.exe --no-animations --output-dir=Extracted E30.DAT
dino2-unpacker.exe --help
```

| OPTION               | EFFECT                                                                        |
| -------------------- | ----------------------------------------------------------------------------- |
| `--output-dir=PATH`  | Write extracted files into PATH instead of next to each input file           |
| `--no-animations`    | Export character models without their animations                              |
| `--in-place`         | Export animations without the movement through the world                      |
| `--animation-tps=60` | Animation ticks per second (default 60; 30 plays at half speed)               |
| `--save-chunks`      | Also save raw sectors and each model block, compressed and decompressed       |

Animations are embedded in each `.model.N.gltf` as clips named `Clip_NN` after their index in
the game's animation table. The first root part follows each frame's stored body position,
and moves through the world horizontally unless `--in-place` is given. A
`.model.N.gltf.animations.json` report lists every animation record and whether it was exported.


## BUILDING

Use a C++17 compiler, CMake and Qt 6 (Core and Gui), with a compiler that matches your Qt build.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

`build-windows.cmd` wraps these two commands. Run `windeployqt` on the executable to collect
the Qt DLLs when distributing a build.

Two tests use your own E30 data (no game files are included):

- `animation_decoder_test` checks the animation decoder: export `E30.DAT` with `--save-chunks`,
  then pass `E30.DAT.block.0.decompressed` to it.
- `integration_e30` runs the unpacker end to end:
  `integration_e30 dino2-unpacker.exe E30.DAT [REFERENCE.gltf]`. It checks the clip, joint and
  key counts and every option, and, given a reference export, that the geometry, skin and every
  rotation key match it.

Set `E30_TEST_BLOCK`, `E30_TEST_DAT` and optionally `E30_TEST_REFERENCE` when configuring CMake
to run both with `ctest`.


## AUTHOR AND CREDITS

Original author:

Krzysztof Michalski  
https://github.com/krzysztof6563

Underlying fork/build:

DexeTech/dino2-unpacker  
https://github.com/DexeTech/dino2-unpacker

Animation/collection extension:

Ziggertron (https://github.com/Ziggertron), October 1, 2026. Development used AI-assisted coding and binary analysis; the project user verified the original E30 animation recovery in Blender. Original authors retain credit for their work. Existing copyright notices are retained.


## LICENSE

GPL-3.0 License
https://www.gnu.org/licenses/gpl-3.0.html


## TODO

- Resolve target rigs for remaining secondary motion records.
- Verify original actor/sequence assignment in multi-actor scenes.
- Compare recovered playback timing, interpolation and final-frame hold with the game.
- Decode animation event metadata and establish behavior labels from evidence.
- Add dedicated MP3 detection/extraction and decoding where appropriate.
- Expand menu, screen, map and other non-character asset conversion.
- Verify or restore legacy non-DAT workflows before claiming support for them.
- Test additional game versions and formats, recording failures and partial recovery explicitly.
