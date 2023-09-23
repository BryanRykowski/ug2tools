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
#include "../common/read_word.hpp"

struct TexFileHeader
{
    uint32_t version;
    uint32_t num_files;
};

struct TexImageHeader
{
    uint32_t checksum;
    uint32_t width;
    uint32_t height;
    uint32_t levels;
    uint32_t dxt;
};

struct
{
    std::filesystem::path in_path;
    bool quiet = false;
    bool write = true;
} options;

bool ReadArgs(int argc, char **argv);
bool ReadFileHeader(std::ifstream &in_stream, TexFileHeader &out_header);
bool ReadImageHeader(std::ifstream &in_stream, TexImageHeader &out_header);
bool SkipImageLevel(std::ifstream &in_stream);
bool ReadImageLevel(std::ifstream &in_stream, std::ofstream &out_stream);
bool ReadImage(std::ifstream &in_stream);

int main(int argc, char **argv)
{
    std::ifstream in_stream;
    TexFileHeader header;

    if (argc < 2)
    {
        std::cerr << "Error: No arguments" << std::endl;
        std::cerr << "Unpack failed." << std::endl;
        return -1;
    }
    
    if (ReadArgs(argc, argv))
    {
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

    if (!options.quiet)
    {
        std::cout << "images: " << header.num_files << std::endl << std::endl;
        std::cout << "index | checksum | mipmap levels | dxt version | dimensions" << std::endl << std::endl;
    }

    for (unsigned int i = 0; i < header.num_files; ++i)
    {
        int w = (header.num_files > 9) ? 2 : 1;

        if (!options.quiet) std::cout << std::setw(w) << std::left <<  i << std::setw(0) << " ";
        
        if (ReadImage(in_stream))
        {
            std::cerr << "Unpack failed." << std::endl;
            return -1;
        }
    }
    
    return 0;
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
    char buffer[32];

    in_stream.read(buffer, 32);

    if (in_stream.fail() || in_stream.gcount() != 32)
    {
        std::cerr << "Error: Failed to read image header" << std::endl;
        return true;
    }

    out_header.checksum = read_u32le(&buffer[0]);
    out_header.width = read_u32le(&buffer[4]);
    out_header.height = read_u32le(&buffer[8]);
    out_header.levels = read_u32le(&buffer[12]);
    out_header.dxt = read_u32le(&buffer[24]);

    return false;
}

bool SkipImageLevel(std::ifstream &in_stream)
{
    char size_buffer[4];
    unsigned int data_size;
    
    in_stream.read(size_buffer, 4);
    data_size = read_u32le(size_buffer);

    in_stream.ignore(data_size);

    if (in_stream.fail() || in_stream.gcount() != data_size)
    {
        std::cerr << "Error: Failed to skip image data" << std::endl;
        return true;
    }

    return false;
}

bool ReadImageLevel(std::ifstream &in_stream, std::ofstream &out_stream)
{
    char size_buffer[4];
    unsigned int data_size;
    auto data_buffer = std::make_unique<char[]>(1024);
    unsigned int pos = 0;
    
    in_stream.read(size_buffer, 4);
    data_size = read_u32le(size_buffer);

    while (pos < data_size)
    {
        unsigned int read_count = (data_size - pos > 1024) ? 1024 : (data_size - pos);
        
        in_stream.read(data_buffer.get(), read_count);

        if (in_stream.fail() || in_stream.gcount() != read_count)
        {
            std::cerr << "Error: Failed to read image data" << std::endl;
            return true;
        }

        /*
        out_stream.write(data_buffer.get(), read_count);

        if (out_stream.fail())
        {
            std::cerr << "Error: Failed to write image data" << std::endl;
            return true;
        }
        */

        pos += read_count;
    }

    if (pos != data_size)
    {
        std::cerr << "Error: Failed to read/write image data" << std::endl;
        return true;
    }

    return false;
}

bool ReadImage(std::ifstream &in_stream)
{
    TexImageHeader i_header;
    std::ofstream out_stream;
    
    if (ReadImageHeader(in_stream, i_header))
    {
        return true;
    }

    if (!options.quiet)
    {
        std::cout << "0x" << std::hex <<  i_header.checksum << std::dec << " ";
        std::cout << i_header.levels << " ";
        std::cout << i_header.dxt << " ";
        std::cout << i_header.width << "x" << i_header.height << std::endl;
    }

    for (unsigned int i = 0; i < i_header.levels; ++i)
    {
        if (options.write)
        {
            if (ReadImageLevel(in_stream, out_stream))
            {
                return true;
            }
        }
        else
        {
            if (SkipImageLevel(in_stream))
            {
                return true;
            }
        }
    }

    return false;
}
