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
} options;

bool ReadArgs(int argc, char **argv);

int main(int argc, char **argv)
{
    std::ifstream in_stream;

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
    
    return 0;
}

bool ReadArgs(int argc, char **argv)
{
    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];

        options.in_path = arg;
    }
    
    return false;
}