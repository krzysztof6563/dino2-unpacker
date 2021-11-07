# dino2-unpacker
Utility for extracting data from Dino Crisis 2


## SUPPORTED FILES

| FILE PATTERN | SUPPORTED | NOTES                                                          |
| ------------ | --------- | -------------------------------------------------------------- |
| *.PXL        | ✅         | Saves images as PNGs                                           |
| *.TEX        | ✅         | Saves images as PNGs                                           |
| ARMOR*.DAT   | 🔨         | Implemented dechunking                                         |
| CAPLOGO.DAT  | ❌         |                                                                |
| COMING.DAT   | ❌         | RGB555 image with pallette                                     |
| CONTINUE.DAT | ❌         |                                                                |
| CORE*.DAT    | ❌         | WAVE files                                                     |
| DEMO*.TRG    | ❌         |                                                                |
| DOOR*.DAT    | 🔨         | Extracts textures and soudns, 3D model missing                 |
| E*.DAT       | ❌         | Enemy data 3D models, WAVE, RGB555 textures (optional)         |
| ENDING.DAT   | ❌         | WAVE files                                                     |
| ENTRY.DAT    | ❌         | Dino colliseum portraits                                       |
| FILE.DAT     | ✅         | Dino File images stored as RGB555 with pallette                |
| GAMEOVER.DAT | ❌         |                                                                |
| ITEM.DAT     | ✅         | Saves images as PNGs                                           |
| KOF_*.DAT    | ❌         | Sound effects in WAVE format                                   |
| LOAD.DAT     | ❌         |                                                                |
| MAP.BIN      | ❌         |                                                                |
| MAP.DAT      | ❌         |                                                                |
| MAP_ST*.DAT  | ❌         | RGB555 images with pallette                                    |
| ME_*.DAT     | ❌         | LAME MP3 Files (Enemy Music)                                   |
| MF_*.DAT     | ❌         | LAME MP3 Files (Background Music)                              |
| M_RESULT.DAT | ❌         | Possibly LAME MP3 Files                                        |
| MS_*.DAT     | ❌         | LAME MP3 Files (Background Sounds)                             |
| M_TITLE.DAT  | ❌         | WAVE files at bottom                                           |
| NO_DATA.PXL  | ❌         |                                                                |
| OPENING.DAT  | ❌         |                                                                |
| OPTION.DAT   | ❌         |                                                                |
| RES*.DAT     | ❌         |                                                                |
| RESULT.DAT   | ❌         | WAVE files                                                     |
| SAVE.DAT     | ❌         |                                                                |
| SC*.DAT      | ❌         | WAVE files, RGB555 with pallette                               |
| ST*.DAT      | ❌         | Mix of files: RGB555 with palletee, WAVE RIFF files and others |
| ST*.DBS      | ✅         | Saves invidual images as PNGs                                  |
| SUBSCR3.DAT  | ❌         |                                                                |
| SUBSCR6.DAT  | ❌         | RGB555 image with pallette                                     |
| TITLE.DAT    | ❌         | WAVE files                                                     |
| TITLE2.DAT   | ❌         | WAVE files                                                     |
| WARNING.DAT  | ❌         |                                                                |
| WEP_*.DAT    | ❌         | WAVE files                                                     |
| WP*.DAT      | ✅         | Saves images as PNGs                                           |

## AUTHOR
Krzysztof Michalski
https://github.com/krzysztof6563

## LICENSE
GPL-3.0 License
https://www.gnu.org/licenses/gpl-3.0.html