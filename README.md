# ug2tools
Useful utilities for THUG 2/THUG Pro.

* Cross platform
* MIT License

## Tools

### ug2-unpack
<details>
<br>
<summary>Extract files embedded in pre/prx files.</summary>

```
Usage:

    ug2-unpack [FILE] [OPTION]...
    
Example:

    ug2-unpack infile.prx -wo data/pre

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
    -V              Print version information.
```
</details>

### ug2-pack
<details>
<br>
<summary>Embed game resources in pre/prx file.</summary>

```
Usage: 
    
    ug2-pack [FILE] [OPTION]...

Examples:
    
    ug2-pack in.prespec -o out.pre

    Create out.pre and insert the files listed in in.prespec.

    ug2-pack -o somewhere/name.pre \
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
    -V                          Print version info
```

**Note: ug2-pack does not compress input files.**

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
    -f FILENAME                 Override output filename.
    -q                          Suppress some output. Does not include errors
    -w                          Overwrite existing files.
    -n                          Don't create dds files, just list the contents of the tex file.
    -l                          Disable generation of filelist.
    -L                          Use relative paths in filelist.
```
</details>

### ug2-dds2tex
<details>
<br>
<summary>Pack dds files into a tex.xbx file.</summary>

```
Usage: ug2-dds2tex [OPTION] [OUT FILE]...

Examples:

        ug2-dds2tex outfile.tex.xbx -l infile.filelist -c infile.tex.xbx

        Place files listed in infile.filelist into outfile.tex.xbx and copy over checksums
        from infile.tex.xbx.

Options:
    -h                          Print this help text
    -f FILENAME                 Manually specify an input file.
    -q                          Suppress some output. Does not include errors
    -n                          Don't create tex.xbx file, just list the input files.
    -l FILELIST                 Provide list of input files.
    -c TEXFILE                  Provide tex.xbx file to copy checksums from.
    -w                          Overwrite existing output file.
```
</details>

## Status
Tool|Status
---|---
ug2-pre-unpack|v1.1.0
ug2-pre-pack|Ready
ug2-tex2dds|Ready
ug2-dds2tex|Ready
ug2-img2png|
ug2-png2img|
ug2-mdl2obj|
ug2-obj2mdl|
ug2-col2obj|
ug2-obj2col|
---
**Copyright (c) 2023-2024 Bryan Rykowski**
