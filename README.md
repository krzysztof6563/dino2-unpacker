# dino2-unpacker
Utility for extracting data from Dino Crisis 2


## SUPPORTED FILES

| FILE PATTERN | SUPPORTED | NOTES                                                                                            |
| ------------ | --------- | ------------------------------------------------------------------------------------------------ |
| *.PXL        | ✅         | Saves images as PNGs                                                                             |
| *.TEX        | ✅         | Saves images as PNGs                                                                             |
| ARMOR*.DAT   | 🔨         | Implemented dechunking                                                                           |
| CAPLOGO.DAT  | ❌         |                                                                                                  |
| COMING.DAT   | ✅         | RGB555 image with pallette                                                                       |
| CONTINUE.DAT | ❌         |                                                                                                  |
| CORE*.DAT    | 🔨         | WAVE files (DONE), RGB555 textures                                                               |
| DEMO*.TRG    | ❌         |                                                                                                  |
| DOOR*.DAT    | 🔨         | Extracts textures and soudns, 3D model missing                                                   |
| E*.DAT       | ❌         | Enemy data 3D models, WAVE, RGB555 textures (optional)                                           |
| ENDING.DAT   | ✅         | WAVE files                                                                                       |
| ENTRY.DAT    | 🔨         | Dino colliseum portraits, some data after                                                        |
| FILE.DAT     | ✅         | Dino File images stored as RGB555 with pallette                                                  |
| GAMEOVER.DAT | ❌         |                                                                                                  |
| ITEM.DAT     | ✅         | Saves images as PNGs                                                                             |
| KOF_*.DAT    | ✅         | Sound effects in WAVE format                                                                     |
| LOAD.DAT     | ❌         |                                                                                                  |
| MAP.BIN      | ❌         |                                                                                                  |
| MAP.DAT      | ❌         | RGB555 images with pallette                                                                      |
| MAP_ST*.DAT  | ❌         | RGB555 images with pallette                                                                      |
| ME_*.DAT     | ❌         | LAME MP3 Files (Enemy Music)                                                                     |
| MF_*.DAT     | ❌         | LAME MP3 Files (Background Music)                                                                |
| M_RESULT.DAT | ❌         | Possibly LAME MP3 Files                                                                          |
| MS_*.DAT     | ❌         | LAME MP3 Files (Background Sounds)                                                               |
| M_TITLE.DAT  | 🔨         | MP3 file at top, WAVE files at bottom (DONE)                                                     |
| OPENING.DAT  | ❌         |                                                                                                  |
| OPTION.DAT   | ❌         |                                                                                                  |
| RES*.DAT     | ❌         |                                                                                                  |
| RESULT.DAT   | 🔨         | WAVE files (DONE), something else is also here maybe 3d model and textures for colloseum trophy? |
| SAVE.DAT     | ❌         |                                                                                                  |
| SC*.DAT      | 🔨         | WAVE files (DONE), RGB555 with pallette                                                          |
| ST*.DAT      | 🔨         | Mix of files: RGB555 with palletee, WAVE files (DONE) and others                                 |
| ST*.DBS      | ✅         | Saves invidual images as JPGs                                                                    |
| SUBSCR3.DAT  | ❌         |                                                                                                  |
| SUBSCR6.DAT  | ✅         | RGB555 image with pallette, contains data for boat ride selection screeen                        |
| TITLE.DAT    | ✅         | WAVE files, maybe something else hides here                                                      |
| TITLE2.DAT   | ✅         | WAVE files, maybe something else hides here                                                      |
| WARNING.DAT  | ❌         |                                                                                                  |
| WEP_*.DAT    | 🔨         | WAVE files (DONE), RGB555 images, model data                                                     |
| WP*.DAT      | 🔨         | Saves images as PNGs, additional data after images for 75A, 79A, 83A, 84A                        |

## AUTHOR
Krzysztof Michalski
https://github.com/krzysztof6563

## LICENSE
GPL-3.0 License
https://www.gnu.org/licenses/gpl-3.0.html

## TODO
- add global detection and extraction of WAVE and MP3 FILES:
x  - search for 0b11111111111 string? - start of mpeg frame