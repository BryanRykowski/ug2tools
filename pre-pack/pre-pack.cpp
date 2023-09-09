// Copyright (c) 2023 BryanRykowski
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

#include <filesystem>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include "../common/pre_header.hpp"
#include "../common/subfile_header.hpp"

struct FilePair
{
    std::filesystem::path path;
    std::string internal_path;

    FilePair(std::filesystem::path pathin, std::string internal_pathin) : path(pathin), internal_path(internal_pathin) {} 
};

struct
{
    std::filesystem::path prespecpath;
    std::filesystem::path outpath = "out.pre";
    std::vector<FilePair> filelist;
} globalValues;

bool ReadArgs(int argc, char **argv);
bool ReadPrespec();
bool ReadLine(std::ifstream &instream, std::string &outstr);
bool WritePre();
bool WritePreHeader(std::ofstream &outstream, const PreHeader &header, unsigned int &sizeout);

int main(int argc, char **argv)
{
    if (!(argc > 1))
    {
        std::cerr << "Error: No arguments" << std::endl;
        std::cerr << "Packing failed." << std::endl;
        return -1;
    }

    if (ReadArgs(argc, argv))
    {
        std::cerr << "Packing failed." << std::endl;
        return -1;
    }

    if (!globalValues.prespecpath.empty())
    {
        if (ReadPrespec())
        {
            std::cerr << "Packing failed." << std::endl;
            return -1;
        }
    }

    if (globalValues.filelist.size() == 0)
    {
        std::cerr << "Error: No files to pack" << std::endl;
        std::cerr << "Packing failed." << std::endl;
        return -1;
    }

    if (WritePre())
    {
        std::cerr << "Packing failed." << std::endl;
        return -1;
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
            bool exclusive_sw = false;

            for (char c : switches)
            {
                if (c == 'f')
                {
                    if (exclusive_sw)
                    {
                        std::cerr << "Error: Mutually exclusive switches combined" << std::endl;
                        return true;
                    }

                    exclusive_sw = true;
                    
                    if (i + 2 >= argc)
                    {
                        std::cerr << "Error: Wrong number of arguments after -f" << std::endl;
                        return true;
                    }

                    globalValues.filelist.push_back(FilePair(std::filesystem::path(argv[i+1]),argv[i+2]));
                    i += 2;
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
                    globalValues.outpath = argv[i];
                }
            }
        }
        else
        {
            globalValues.prespecpath = arg;
        }
    }

    return false;
}

bool ReadPrespec()
{
    std::ifstream psstream(globalValues.prespecpath);

    if (!psstream.good())
    {
        std::cerr << "Error: Failed to open prespec file \"" << globalValues.prespecpath.string() << "\"" << std::endl;
        return true;
    }

    std::string line;

    while (psstream.good() && !psstream.eof())
    {
        if (ReadLine(psstream, line))
        {
            std::cerr << "Error: Failed to read prespec file" << std::endl;
            return true;
        }

        std::filesystem::path filepath = line;

        if (psstream.eof())
        {
            std::cerr << "Error: Disk path/internal path mismatch in prespec file" << std::endl;
            return true;
        }

        if (ReadLine(psstream, line))
        {
            std::cerr << "Error: Failed to read prespec file" << std::endl;
            return true;
        }

        globalValues.filelist.push_back(FilePair(filepath, line));
    }

    return false;
}

bool ReadLine(std::ifstream &instream, std::string &outstr)
{
    bool line_ended = false;
    outstr = "";

    while (true)
    {

        if (instream.fail()) {return true;}

        int p = instream.peek();

        if (p == EOF)
        {
            break;
        }
        else if (static_cast<char>(p) == '\r' || static_cast<char>(p) == '\n')
        {
            line_ended = true;
            instream.get();
        }
        else
        {
            if (line_ended) {break;}

            outstr.push_back(instream.get());
        }
    }

    return false;
}

bool WritePre()
{
    unsigned int presize = 0;
    unsigned int precount = 0;
    std::ofstream outstream;
    std::vector<char> buffer;
    PreHeader header;
    const unsigned int chunksize = 1024 * 1024;

    outstream.open(globalValues.outpath, outstream.binary);

    if (outstream.fail())
    {
        std::cerr << "Error: Failed to create pre file \"" << globalValues.outpath.string() << "\"" << std::endl;
        return true;
    }

    buffer.reserve(chunksize);

    if (WritePreHeader(outstream, PreHeader(), presize))
    {
        std::cerr << "Error: Failed to write pre file header" << std::endl;
        return true;
    }

    for (FilePair fp : globalValues.filelist)
    {
        std::ifstream instream;
        SubFileHeader subheader;
        unsigned int pad;
        
        std::cout << "file: " << fp.path.string() << std::endl;
        std::cout << "internal path: " << fp.internal_path << std::endl;

        pad = (fp.internal_path.size() % 4) ? (4 - (fp.internal_path.size() % 4)) : 0;

        for (char c : fp.internal_path)
        {
            subheader.path.push_back(c);
        }

        for (unsigned int i = 0; i < pad; ++i)
        {
            subheader.path.push_back(0);
        }

        subheader.pathSize = subheader.path.size();

        instream.open(fp.path, instream.binary);

        if (!instream.good())
        {
            std::cerr << "Error: Failed to open \"" << fp.path.string() << "\"" << std::endl;
            return true;
        }

        subheader.inflatedSize = 0;
        subheader.deflatedSize = 0;
        buffer.clear();
        
        while (!instream.eof())
        {
            if (subheader.inflatedSize == buffer.capacity())
            {
                buffer.reserve(buffer.capacity() + chunksize);
            }

            instream.read(buffer.data() + subheader.inflatedSize, chunksize);
            subheader.inflatedSize += instream.gcount();
        }

        std::cout << "size: " << subheader.inflatedSize << std::endl << std::endl;

        presize += 16 + subheader.pathSize + subheader.inflatedSize;

        ++precount;
    }

    header.size = presize;
    header.numFiles = precount;

    outstream.seekp(0);

    if (WritePreHeader(outstream, header, presize))
    {
        std::cerr << "Error: Failed to write pre file header" << std::endl;
        return true;
    }

    std::cout << globalValues.outpath.string() << std::endl << std::endl;
    std::cout << "total files: " << precount << std::endl;
    std::cout << "total size: " << presize << std::endl;
    
    return false;
}

bool WritePreHeader(std::ofstream &outstream, const PreHeader &header, unsigned int &sizeout)
{
    char bytes[12];

    bytes[0] = static_cast<char>(0xff & header.size);
    bytes[1] = static_cast<char>(0xff & (header.size >> 8));
    bytes[2] = static_cast<char>(0xff & (header.size >> 16));
    bytes[3] = static_cast<char>(0xff & (header.size >> 24));
    bytes[4] = 0x3;
    bytes[5] = 0x0;
    bytes[6] = 0xcd;
    bytes[7] = 0xab;
    bytes[8] = static_cast<char>(0xff & header.numFiles);
    bytes[9] = static_cast<char>(0xff & (header.numFiles >> 8));
    bytes[10] = static_cast<char>(0xff & (header.numFiles >> 16));
    bytes[11] = static_cast<char>(0xff & (header.numFiles >> 24));

    outstream.write(bytes, 12);

    if (outstream.fail())
    {
        return true;
    }

    sizeout += 12;
    return false;
}