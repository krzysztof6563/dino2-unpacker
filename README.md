# dino2-unpacker

Utility for extracting data from Dino Crisis 2

## SUPPORTED FILES

| FILE PATTERN | SUPPORTED | NOTES                                                                                            |
| ------------ | --------- | ------------------------------------------------------------------------------------------------ |
| \*.PXL       | ✅        | Saves images as PNGs                                                                             |
| \*.TEX       | ✅        | Saves images as PNGs                                                                             |
| ARMOR\*.DAT  | 🔨        | Implemented dechunking                                                                           |
| CAPLOGO.DAT  | ❌        |                                                                                                  |
| COMING.DAT   | ✅        | RGB555 image with pallette                                                                       |
| CONTINUE.DAT | ❌        |                                                                                                  |
| CORE\*.DAT   | 🔨        | WAVE files (DONE), RGB555 textures                                                               |
| DEMO\*.TRG   | ❌        |                                                                                                  |
| DOOR\*.DAT   | 🔨        | Extracts textures and soudns and most of 3d models (excpetion is DOOR1900.DAT)                   |
| E\*.DAT      | ✅        | Textured model as OBJ and rigged glTF (DONE), texture page PNG (DONE), animations (DONE)         |
| ENDING.DAT   | ✅        | WAVE files                                                                                       |
| ENTRY.DAT    | 🔨        | Dino colliseum portraits, some data after                                                        |
| FILE.DAT     | ✅        | Dino File images stored as RGB555 with pallette                                                  |
| GAMEOVER.DAT | ❌        |                                                                                                  |
| ITEM.DAT     | ✅        | Saves images as PNGs                                                                             |
| KOF\_\*.DAT  | ✅        | Sound effects in WAVE format                                                                     |
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
| RESULT.DAT   | 🔨        | WAVE files (DONE), something else is also here maybe 3d model and textures for colloseum trophy? |
| SAVE.DAT     | ❌        |                                                                                                  |
| SC\*.DAT     | 🔨        | WAVE files (DONE), RGB555 with pallette                                                          |
| ST\*.DAT     | 🔨        | Mix of files: RGB555 with palletee, WAVE files (DONE) and others                                 |
| ST\*.DBS     | ✅        | Saves invidual images as JPGs                                                                    |
| SUBSCR3.DAT  | ❌        |                                                                                                  |
| SUBSCR6.DAT  | ✅        | RGB555 image with pallette, contains data for boat ride selection screeen                        |
| TITLE.DAT    | ✅        | WAVE files, maybe something else hides here                                                      |
| TITLE2.DAT   | ✅        | WAVE files, maybe something else hides here                                                      |
| WARNING.DAT  | ❌        |                                                                                                  |
| WEP\_\*.DAT  | ✅        | WAVE files (DONE), textured model as OBJ and rigged glTF (DONE), texture page PNG (DONE), animations (DONE)         |
| WP\*.DAT     | 🔨        | Saves images as PNGs, additional data after images for 75A, 79A, 83A, 84A                        |


## ENEMY ANIMATION COVERAGE

Compatible animation export was checked for:

`E00.DAT`, `E10.DAT`, `E20.DAT`, `E30.DAT`, `E31.DAT`, `E32.DAT`, `E40.DAT`, `E50.DAT`, `E60.DAT`, `E70.DAT`, `E80.DAT`, `E90.DAT` and `EA0.DAT`.

`E32.DAT` contains an additional embedded rig, which exports separately with 17 compatible clips. Each exported model uses its own hierarchy and joint count.

The decoder handles pointer tables, repeated motion pointers, empty slots, multiple record sizes, aligned or packed joint rotations, and validated sequential records. A different-rig record no longer causes all compatible clips to be discarded.

Numeric names such as `Clip_00` preserve source indices. Behavior names such as idle, walk, attack and death are not guessed.


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
- The tested scope is Dino Crisis 2 PC. Dino Crisis 1 and console formats are not established as supported.
- This build does not claim universal decoding of every possible DAT or animation format.


## COMMAND-LINE USAGE

```bat
dino2-unpacker.exe --output-dir=Extracted --save-chunks "C:\Game\Data"
dino2-unpacker.exe --output-dir=Extracted E30.DAT
dino2-unpacker.exe --in-place --output-dir=Extracted E80.DAT
dino2-unpacker.exe --animation-tps=30 --output-dir=Extracted E30.DAT
dino2-unpacker.exe --no-animations --output-dir=Extracted E30.DAT
dino2-unpacker.exe --help
```


## AUTHOR AND CREDITS

Original author:

Krzysztof Michalski  
https://github.com/krzysztof6563

Underlying fork/build:

DexeTech/dino2-unpacker  
https://github.com/DexeTech/dino2-unpacker

Animation/collection extension:

Unofficial modified build, October 1, 2026. Development used AI-assisted coding and binary analysis; the project user verified the original E30 animation recovery in Blender. Original authors retain credit for their work. Existing copyright notices are retained.


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
