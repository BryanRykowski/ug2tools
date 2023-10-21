# ug2tools
Useful utilities for THUG 2/THUG Pro.

* Cross platform
* MIT License

## Tools

### ug2-pre-unpack
<details>
<summary>Extract files embedded in pre/prx files.</summary>

```
Usage:

    ug2-pre-unpack [FILE] [OPTION]...
    
Example:

    ug2-pre-unpack infile.prx -wo data/pre

    Lists the contents of "infile.prx" and extracts them to ./data/pre, overwriting any existing
    versions of the files.

Options:

    -h              Print help text
    -o DIRECTORY    Place files in DIRECTORY instead of current directory
    -q              Suppress some output. Does not include errors
    -w              Overwrite existing files
    -p              Disable prespec file generation.
    -P              Disable absolute paths in prespec file.
    -n              Don't extract files or generate prespec.
```
</details>

### ug2-pre-pack
<details>
<br>
<summary>Embed game resources in pre/prx file.</summary>

```
Usage: 
    
    ug2-pre-pack [FILE] [OPTION]...

Examples:
    
    ug2-pre-pack in.prespec -o out.pre

    Create out.pre and insert the files listed in in.prespec.

    ug2-pre-pack -o somewhere/name.pre \
    -f file1.qb internal\\path\\file1.qb \
    -f file2.col.xbx other\\internal\\path\\file2.col.xbx

    Manually specify files and their internal paths using the -f switch and write pre file in
    specific location.

Options:

    -h                          Print help text
    -o PATH                     Output file at PATH instead of out.pre in current directory
    -f FILE INTERNAL_PATH       Embed FILE with internal path INTERNAL_PATH
    -q                          Suppress some output. Does not include errors
    -w                          Overwrite existing file
    -n                          Don't create pre file, just list files
```

**Note: ug2-pre-pack does not compress input files.**

</details>

### ug2-tex2dds
<details>
<br>
<summary>Extract dds files from tex.xbx files.</summary>

```
Usage:

    ug2-tex2dds [FILE] [OPTION]...

Examples:

        ug2-tex2dds infile.tex.xbx -o outdir

        Extract files to outdir/ in the format infile.[image number].dds .

Options:
    -h                          Print help text
    -o DIRECTORY                Output files in DIRECTORY instead of current directory.
    -f FILENAME                 Overwride output filename.
    -q                          Suppress some output. Does not include errors
    -w                          Overwrite existing files.
    -n                          Don't create dds files, just list the contents of the tex file.
    -l                          Disable generation of filelist.
    -L                          Use relative paths in filelist.
```
</details>

## Status
Tool|Status
---|---
ug2-pre-unpack|Ready
ug2-pre-pack|Ready
ug2-tex2dds|Ready
ug2-dds2tex|
ug2-img2png|
ug2-png2img|
ug2-mdl2obj|
ug2-obj2mdl|
ug2-col2obj|
ug2-obj2col|
---
**Copyright (c) 2023 Bryan Rykowski**
