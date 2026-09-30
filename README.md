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
| E\*.DAT      | 🔨        | Textured model as OBJ and rigged glTF (DONE), texture page PNG (DONE), animations                |
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
| WEP\_\*.DAT  | 🔨        | WAVE files (DONE), textured model as OBJ and rigged glTF (DONE), texture page PNG (DONE)         |
| WP\*.DAT     | 🔨        | Saves images as PNGs, additional data after images for 75A, 79A, 83A, 84A                        |

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

## AUTHOR

Krzysztof Michalski
https://github.com/krzysztof6563

## LICENSE

GPL-3.0 License
https://www.gnu.org/licenses/gpl-3.0.html

## TODO

- add global detection and extraction of WAVE and MP3 FILES:
  x - search for 0b11111111111 string? - start of mpeg frame
