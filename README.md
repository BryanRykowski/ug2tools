# ug2tools
Useful utilities for THUG 2/THUG Pro.

* Cross platform
* MIT License

## Tools
### ug2-pre-unpack
Extracts files embedded in pre/prx files. Generates a prespec file with internal paths used by game.

Usage:
```
ug2-pre-unpack file.prx -o outdir
```

Will place embedded files and file.prespec in outdir/ .

### ug2-pre-pack
Embed files in pre/prx file.

Usage:
```
ug2-pre-pack infile.prespec -o path/to/outfile.pre
```

Will embed files listed in infile.prespec in outfile.pre.

**Note: ug2-pre-pack does not compress input files.**

## Status
Tool|Status
---|---
ug2-pre-unpack|Ready
ug2-pre-pack|Ready
ug2-tex2png|
ug2-png2tex|
ug2-img2png|
ug2-png2img|
ug2-mdl2obj|
ug2-obj2mdl|
ug2-col2obj|
ug2-obj2col|

**Copyright (c) 2023 Bryan Rykowski**