// Copyright (c) 2023 Bryan Rykowski
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <stdint.h>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <iomanip>
#include <memory>
#include <algorithm>
#include "../common/read_word.hpp"
#include "../common/write_word.hpp"
#include "../common/dds_header.hpp"
#include "../common/tex_header.hpp"

struct
{
    std::filesystem::path in_path;
    std::filesystem::path out_dir;
    std::filesystem::path filename;
    bool quiet = false;
    bool write = true;
    bool overwrite = false;
    bool printhelp = false;
    bool filelist = true;
    bool filelist_fullpath = true;
} options;

void PrintHelp();
bool ReadArgs(int argc, char **argv);
bool ReadFileHeader(std::ifstream &in_stream, TexFileHeader &out_header);
bool ReadImageHeader(std::ifstream &in_stream, TexImageHeader &out_header);
bool ReadImageLevelSize(std::ifstream &in_stream, uint32_t &out_size);
bool SkipImageLevel(std::ifstream &in_stream, unsigned int size);
bool ReadImageLevel(std::ifstream &in_stream, std::ofstream &out_stream, unsigned int size);
bool WriteDdsHeader(std::ofstream &out_stream, const DdsFileHeader &dds_header);
void BuildDdsHeader(const TexImageHeader &i_header, DdsFileHeader &dds_header);
bool ReadImage(std::ifstream &in_stream, unsigned int index, std::ofstream &filelist_stream);

int main(int argc, char **argv)
{
    std::ifstream in_stream;
    std::ofstream filelist_stream;
    TexFileHeader header;

    if (argc < 2)
    {
        std::cerr << "Error: No arguments" << std::endl;
        std::cerr << "Unpack failed." << std::endl;
        PrintHelp();
        return -1;
    }
    
    if (ReadArgs(argc, argv))
    {
        std::cerr << "Unpack failed." << std::endl;
        return -1;
    }

    if (options.printhelp)
    {
        PrintHelp();
        return 0;
    }

    if (options.in_path.empty())
    {
        std::cerr << "Error: No input file" << std::endl;
        std::cerr << "Unpack failed." << std::endl;
        return -1;
    }
    
    in_stream.open(options.in_path);

    if (in_stream.fail())
    {
        std::cerr << "Couldn't open file \"" << options.in_path.string() << "\"" << std::endl;
        std::cerr << "Unpack failed." << std::endl;
        return -1;
    }

    if (!options.quiet)
    {
        std::cout << "file: " << options.in_path.string() << std::endl;
    }

    if (ReadFileHeader(in_stream, header))
    {
        std::cerr << "Unpack failed." << std::endl;
        return -1;
    }

    if (header.version != 1)
    {
        std::cerr << "Error: byte 0 is not 0x1, this isn't a tex.xbx file" << std::endl;
        std::cerr << "Unpack failed." << std::endl;
        return -1;
    }

    if (!options.quiet)
    {
        std::cout << "images: " << header.num_files << std::endl << std::endl;
        std::cout << "index | checksum | mipmap levels | dxt version | dimensions" << std::endl << std::endl;
    }

    // A tex file has the layout:
    //
    //      header:
    //          version?            4 bytes         Always 1
    //          number of images    4 bytes
    //
    //      image 0:
    //          header              32 bytes        Contains a checksum, dimensions, number of levels, and compression type
    //          level 0             4 + x bytes     First 4 bytes are the size of the mipmap level
    //          .
    //          .
    //          level n             4 + x bytes
    //      .
    //      .
    //      image n:
    //          header              32 bytes
    //          level 0             4 + x bytes
    //          .
    //          .
    //          level n             4 + x bytes

    if (options.filelist)
    {
        std::filesystem::path filelist_path = options.out_dir;
        filelist_path /= options.in_path.filename();
        filelist_path += ".filelist";
        
        if (std::filesystem::exists(filelist_path) && !options.overwrite)
        {
            std::cerr << "Error: Filelist \"" << filelist_path << "\" already exists and overwrite not enabled" << std::endl;
            std::cerr << "Unpack failed." << std::endl;
            return -1;
        }
        
        filelist_stream.open(filelist_path);

        if (filelist_stream.fail())
        {
            std::cerr << "Error: Failed to create filelist" << std::endl;
            std::cerr << "Unpack failed." << std::endl;
            return -1;
        }
    }

    for (unsigned int i = 0; i < header.num_files; ++i)
    {
        int w = (header.num_files > 9) ? 2 : 1;

        if (!options.quiet) std::cout << std::setw(w) << std::left <<  i << std::setw(0) << " ";
        
        if (ReadImage(in_stream, i, filelist_stream))
        {
            std::cerr << "Unpack failed." << std::endl;
            return -1;
        }
    }
    
    return 0;
}

void PrintHelp()
{
    std::cout << "Usage: ug2-tex2dds [FILE] [OPTION]..." << std::endl << std::endl;
    std::cout << "Extract dds files from tex.xbx files." << std::endl << std::endl;
    std::cout << "Examples:" << std::endl << std::endl;
    std::cout << "        ug2-tex2dds infile.tex.xbx -o outdir" << std::endl << std::endl;
    std::cout << "        Extract files to outdir/ in the format infile.[image number].dds ." << std::endl << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "    -h                          Print this help text" << std::endl;
    std::cout << "    -o DIRECTORY                Output files in DIRECTORY instead of current directory." << std::endl;
    std::cout << "    -f FILENAME                 Override output filename." << std::endl;
    std::cout << "    -q                          Suppress some output. Does not include errors" << std::endl;
    std::cout << "    -w                          Overwrite existing files." << std::endl;
    std::cout << "    -n                          Don't create dds files, just list the contents of the tex file." << std::endl;
    std::cout << "    -l                          Disable generation of filelist." << std::endl;
    std::cout << "    -L                          Use relative paths in filelist." << std::endl;
}

bool ReadArgs(int argc, char **argv)
{
    std::string arg;

    for (int i = 1; i < argc; ++i)
    {
        arg = argv[i];

        if ((arg[0] == '-') && (arg.size() > 1))
        {
            std::string switches = arg.substr(1, arg.size() - 1);
            bool exclusive_sw = false;
            
            for (char c : switches)
            {
                if (c == 'q')
                {
                    options.quiet = true;
                }
                else if (c == 'n')
                {
                    options.write = false;
                }
                else if (c == 'w')
                {
                    options.overwrite = true;
                }
                else if (c == 'h')
                {
                    options.printhelp = true;
                }
                else if (c == 'l')
                {
                    options.filelist = false;
                }
                else if (c =='L')
                {
                    options.filelist_fullpath = false;
                }
                else if (c == 'o')
                {
                    if (exclusive_sw)
                    {
                        std::cerr << "Error: Mutually exclusive switches combined" << std::endl;
                        return true;
                    }

                    exclusive_sw = true;

                    if (i + 1 >= argc)
                    {
                        std::cerr << "Error: Wrong number of arguments after -o" << std::endl;
                        return true;
                    }

                    ++i;
                    options.out_dir = argv[i];
                }
                else if (c == 'f')
                {
                    if (exclusive_sw)
                    {
                        std::cerr << "Error: Mutually exclusive switches combined" << std::endl;
                        return true;
                    }

                    exclusive_sw = true;

                    if (i + 1 >= argc)
                    {
                        std::cerr << "Error: Wrong number of arguments after -f" << std::endl;
                        return true;
                    }

                    ++i;
                    options.filename = argv[i];
                }
            }
        }
        else
        {
            options.in_path = arg;
        }
    }
    
    return false;
}

bool ReadFileHeader(std::ifstream &in_stream, TexFileHeader &out_header)
{
    char buffer[8];

    in_stream.read(buffer, 8);

    if (in_stream.fail() || in_stream.gcount() != 8)
    {
        std::cerr << "Error: Failed to read file header" << std::endl;
        return true;
    }

    out_header.version = read_u32le(&buffer[0]);
    out_header.num_files = read_u32le(&buffer[4]);

    return false;
}

bool ReadImageHeader(std::ifstream &in_stream, TexImageHeader &out_header)
{
    char buffer[36];

    in_stream.read(buffer, 36);

    if (in_stream.fail() || in_stream.gcount() != 36)
    {
        std::cerr << "Error: Failed to read image header" << std::endl;
        return true;
    }

    out_header.checksum = read_u32le(buffer + 0);
    out_header.width = read_u32le(buffer + 4);
    out_header.height = read_u32le(buffer + 8);
    out_header.levels = read_u32le(buffer + 12);
    out_header.dxt = read_u32le(buffer + 24);
    out_header.size = read_u32le(buffer + 32);

    return false;
}

bool ReadImageLevelSize(std::ifstream &in_stream, uint32_t &out_size)
{
    char size_buffer[4];
    
    in_stream.read(size_buffer, 4);

    if (in_stream.fail() || in_stream.gcount() != 4)
    {
        std::cerr << "Error: Failed to read mipmap level size" << std::endl;
        return true;
    }

    out_size = read_u32le(size_buffer);

    return false;
}

bool SkipImageLevel(std::ifstream &in_stream, unsigned int size)
{
    in_stream.ignore(size);

    if (in_stream.fail() || in_stream.gcount() != size)
    {
        std::cerr << "Error: Failed to skip image data" << std::endl;
        return true;
    }

    return false;
}

bool ReadImageLevel(std::ifstream &in_stream, std::ofstream &out_stream, unsigned int size)
{
    auto data_buffer = std::make_unique<char[]>(1024);
    unsigned int pos = 0;

    while (pos < size)
    {
        unsigned int read_count = (size - pos > 1024) ? 1024 : (size - pos);
        
        in_stream.read(data_buffer.get(), read_count);

        if (in_stream.fail() || in_stream.gcount() != read_count)
        {
            std::cerr << "Error: Failed to read image data" << std::endl;
            return true;
        }

        out_stream.write(data_buffer.get(), read_count);

        if (out_stream.fail())
        {
            std::cerr << "Error: Failed to write image data" << std::endl;
            return true;
        }

        pos += read_count;
    }

    if (pos != size)
    {
        std::cerr << "Error: Failed to read/write image data" << std::endl;
        return true;
    }

    return false;
}

bool WriteDdsHeader(std::ofstream &out_stream, const DdsFileHeader &dds_header)
{
    char buffer[128];

    // Magic number
    buffer[0] = 'D';
    buffer[1] = 'D';
    buffer[2] = 'S';
    buffer[3] = ' ';

    write_u32le(buffer + 4, 124); // Size, always 124
    write_u32le(buffer + 8, dds_header.flags);
    write_u32le(buffer + 12, dds_header.height);
    write_u32le(buffer + 16, dds_header.width);
    write_u32le(buffer + 20, dds_header.pitch);
    write_u32le(buffer + 24, dds_header.depth);
    write_u32le(buffer + 28, dds_header.levels);
    std::fill(buffer + 32, buffer + 76, 0); // Zero out 44 unused reserved1 bytes.
    
    // DDS Pixel Format
    write_u32le(buffer + 76, 32); // Size, always 32
    write_u32le(buffer + 80, dds_header.pix_fmt.flags);
    std::copy(dds_header.pix_fmt.fourcc, dds_header.pix_fmt.fourcc + 4, buffer + 84);
    write_u32le(buffer + 88, dds_header.pix_fmt.rgb_bits);
    write_u32le(buffer + 92, dds_header.pix_fmt.r_bitmask);
    write_u32le(buffer + 96, dds_header.pix_fmt.g_bitmask);
    write_u32le(buffer + 100, dds_header.pix_fmt.b_bitmask);
    write_u32le(buffer + 104, dds_header.pix_fmt.a_bitmask);
    
    write_u32le(buffer + 108, dds_header.caps);
    write_u32le(buffer + 112, dds_header.caps2);
    std::fill(buffer + 116, buffer + 128, 0); // Zero out 12 unused cap3, cap4, and reserved2 bytes.

    out_stream.write(buffer, 128);

    if (out_stream.fail())
    {
        std::cerr << "Error: failed to write file header" << std::endl;
        return true;
    }

    return false;
}

void BuildDdsHeader(const TexImageHeader &i_header, DdsFileHeader &dds_header)
{
    const char char_table[5] = {'1', '2', '3', '4', '5'};

    dds_header.flags = 0xa1007; // DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT | DDSD_MIPMAPCOUNT | DDSD_LINEARSIZE
    dds_header.height = i_header.height;
    dds_header.width = i_header.width;
    dds_header.pitch = i_header.size; // First mipmap level size
    dds_header.depth = 0;
    dds_header.levels = i_header.levels;
    
    dds_header.pix_fmt.flags = 0x4; // Indicate that the fourcc field is present
    dds_header.pix_fmt.fourcc[0] = 'D';
    dds_header.pix_fmt.fourcc[1] = 'X';
    dds_header.pix_fmt.fourcc[2] = 'T';
    dds_header.pix_fmt.fourcc[3] = char_table[i_header.dxt - 1];
    dds_header.pix_fmt.rgb_bits = 0; // Leaving the rest of the pixel format as 0 seems to work fine
    dds_header.pix_fmt.r_bitmask = 0;
    dds_header.pix_fmt.g_bitmask = 0;
    dds_header.pix_fmt.b_bitmask = 0;
    dds_header.pix_fmt.a_bitmask = 0;

    dds_header.caps = 0x401008; // DDSCAPS_COMPLEX | DDS_CAPS_MIPMAP | DDSCAPS_TEXTURE
    dds_header.caps2 = 0; // Cubemap capabilities, not used.
}

bool ReadImage(std::ifstream &in_stream, unsigned int index, std::ofstream &filelist_stream)
{
    // Each image has the layout:
    //
    //      header:
    //          checksum        4 bytes
    //          width           4 bytes
    //          height          4 bytes
    //          levels          4 bytes
    //          unknown         4 bytes
    //          unknown         4 bytes
    //          dxt version     4 bytes
    //          unknown         4 bytes
    //
    //      level 0:
    //          size            4 bytes
    //          data            [size] bytes
    //      .
    //      .
    //      .
    //
    //      level n:
    //          size            4 bytes
    //          data            [size] bytes
    
    TexImageHeader i_header;
    DdsFileHeader dds_header;
    std::ofstream out_stream;
    unsigned int level_size;
    bool dxt2 = false;
    
    if (ReadImageHeader(in_stream, i_header)) return true;

    if (i_header.dxt > 5 || i_header.dxt == 0)
    {
        std::cerr << "Error: Invalid dxt version (" << i_header.dxt << ") in image " << index << std::endl;
        return true;
    }

    // Some THUG Pro tex.xbx files say they are dxt2 format, but are dxt1.
    // If that's detected, just change the dxt value to 1.
    if (i_header.dxt == 2)
    {
        unsigned int expected_size = i_header.width * i_header.height;
        
        if (i_header.size == expected_size / 2)
        {
            dxt2 = true;
            i_header.dxt = 1;
        }
        else if (i_header.size != expected_size)
        {
            std::cerr << "Error: " << i_header.width << "x" << i_header.height << " dxt" << i_header.dxt << " image should be " << expected_size << " bytes, but was " << i_header.size << std::endl;
            return true;
        }
    } 

    if (!options.quiet)
    {
        std::cout << "0x" << std::hex <<  i_header.checksum << std::dec << " ";
        std::cout << i_header.levels << " ";
        std::cout << ( dxt2 ? "2->" : "" ) << i_header.dxt << " ";
        std::cout << i_header.width << "x" << i_header.height << std::endl;
    }

    if (options.write)
    {
        std::filesystem::path out_path;

        out_path = options.out_dir;
        
        if (options.filename.empty())
        {
            out_path /= options.in_path.stem().stem(); // Remove the .tex.xbx extensions.
        }
        else
        {
            out_path /= options.filename;
        }

        out_path += "." + std::to_string(index) + ".dds";

        if (std::filesystem::exists(out_path) && !options.overwrite)
        {
            std::cerr << "Error: file \"" << out_path.string()  << "\" already exists and overwrite not enabled" << std::endl;
            return true;
        }
        
        out_stream.open(out_path, out_stream.binary);

        if (out_stream.fail())
        {
            std::cerr << "Error: Failed to open output file \"" << out_path.string() << "\"" << std::endl;
            return true;
        }

        BuildDdsHeader(i_header, dds_header);
        
        if (WriteDdsHeader(out_stream, dds_header)) return true;
        if (ReadImageLevel(in_stream, out_stream, i_header.size)) return true;

        for (unsigned int i = 1; i < i_header.levels; ++i)
        {
            if (ReadImageLevelSize(in_stream, level_size)) return true;
            if (ReadImageLevel(in_stream, out_stream, level_size)) return true;
        }

        if (options.filelist)
        {
            std::filesystem::path file_path = (options.filelist_fullpath ? std::filesystem::absolute(out_path) : out_path);
            filelist_stream << file_path.string() << std::endl;

            if (filelist_stream.fail())
            {
                std::cerr << "Error: Failed to write to filelist" << std::endl;
                return true;
            }
        }
    }
    else
    {
        if (SkipImageLevel(in_stream, i_header.size)) return true;

        for (unsigned int i = 1; i < i_header.levels; ++i)
        {
            if (ReadImageLevelSize(in_stream, level_size)) return true;
            if (SkipImageLevel(in_stream, level_size)) return true;
        }
    }

    return false;
}
