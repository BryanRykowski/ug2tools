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

#include <filesystem>
#include <string>
#include <vector>
#include <iostream>
#include <cstdio>
#include <fstream>
#include "../common/pre_header.hpp"
#include "../common/subfile_header.hpp"
#include "../libug2/crc.hpp"
#include "../libug2/write_word.hpp"

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
    bool overwrite = false;
    bool pack = true;
    bool quiet = false;
	bool printversion = false;
    bool printhelp = false;
} globalValues;

void PrintHelp();
void PrintVersion();
bool ReadArgs(int argc, char **argv);
bool ReadPrespec();
bool ReadLine(std::ifstream &instream, std::string &outstr);
bool WritePre();
bool WritePreHeader(std::ofstream &outstream, const PreHeader &header, unsigned int &sizeout);
bool WriteSubFileHeader(std::ofstream &outstream, const SubFileHeader &subheader, unsigned int &sizeout);

int main(int argc, char **argv)
{
    if (!(argc > 1))
    {
        std::cerr << "Error: No arguments" << std::endl;
        std::cerr << "Packing failed." << std::endl;
        PrintHelp();
        return -1;
    }

    if (ReadArgs(argc, argv))
    {
        std::cerr << "Packing failed." << std::endl;
        return -1;
    }

    if (globalValues.printhelp || globalValues.printversion)
    {
		if (globalValues.printversion) PrintVersion();
		if (globalValues.printhelp) PrintHelp();
		if (globalValues.prespecpath.empty()) return 0;
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

    if (!globalValues.quiet)
    {
        std::cout << "Packing successful." << std::endl;
    }

    return 0;
}

void PrintHelp()
{
    std::cout << "Usage: ug2-pack [FILE] [OPTION]..." << std::endl << std::endl;
    std::cout << "Embed game resources in pre/prx file." << std::endl << std::endl;
    std::cout << "Examples:" << std::endl << std::endl;
    std::cout << "        ug2-pack in.prespec -o out.pre" << std::endl << std::endl;
    std::cout << "        Create out.pre and insert the files listed in in.prespec." << std::endl << std::endl << std::endl;
    std::cout << "        ug2-pack -o somewhere/name.pre -f file1.qb internal\\\\path\\\\file1.qb -f file2.col.xbx other\\\\internal\\\\path\\\\file2.col.xbx" << std::endl << std::endl;
    std::cout << "        Manually specify files and their internal paths using the -f switch and write pre file in specific location." << std::endl << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "    -h                          Print this help text" << std::endl;
    std::cout << "    -o PATH                     Output file at PATH instead of out.pre in current directory" << std::endl;
    std::cout << "    -f FILE INTERNAL_PATH       Embed FILE with internal path INTERNAL_PATH" << std::endl;
    std::cout << "    -q                          Suppress some output. Does not include errors" << std::endl;
    std::cout << "    -w                          Overwrite existing file" << std::endl;
    std::cout << "    -n                          Don't create pre file, just list files" << std::endl;
    std::cout << "    -V                          Print version info" << std::endl;
}

void PrintVersion()
{
#ifndef UG2TOOLS_VERSION
#define UG2TOOLS_VERSION "unknown"
#endif

#ifndef APP_VERSION
#define APP_VERSION "unknown"
#endif

	std::printf("ug2tools %s\npack %s\n", UG2TOOLS_VERSION, APP_VERSION);
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
                else if (c == 'w')
                {
                    globalValues.overwrite = true;
                }
                else if (c == 'n')
                {
                    globalValues.pack = false;
                }
                else if (c == 'q')
                {
                    globalValues.quiet = true;
                }
                else if (c == 'h')
                {
                    globalValues.printhelp = true;
                }
                else if (c == 'V')
                {
                    globalValues.printversion = true;
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

    buffer.reserve(chunksize);
    
    if (globalValues.pack)
    {
        if (std::filesystem::exists(globalValues.outpath) && !globalValues.overwrite)
        {
            std::cerr << "Error: file \"" << globalValues.outpath.string() << "\" already exists and overwrite not enabled" << std::endl;
            return true;
        }
        
        outstream.open(globalValues.outpath, outstream.binary);

        if (outstream.fail())
        {
            std::cerr << "Error: Failed to create pre file \"" << globalValues.outpath.string() << "\"" << std::endl;
            return true;
        }

        if (WritePreHeader(outstream, PreHeader(), presize))
        {
            std::cerr << "Error: Failed to write pre file header" << std::endl;
            return true;
        }
    }

    for (FilePair fp : globalValues.filelist)
    {
        std::ifstream instream;
        SubFileHeader subheader;
        unsigned int pad;
        
        if (!globalValues.quiet)
        {
            std::cout << "file: " << fp.path.string() << std::endl;
            std::cout << "internal path: " << fp.internal_path << std::endl;
        }

        subheader.pathCRC = StringCRC(fp.internal_path);
        
        // Even if the path string ends up being a multiple of 4 we need to pad it because there
        // needs to be a null at the end.
        pad = 4 - (fp.internal_path.size() % 4);

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
        buffer.resize(chunksize);
        
        while (!instream.eof())
        {
            if (subheader.inflatedSize == buffer.size())
            {
                buffer.resize(buffer.size() + chunksize);
            }

            instream.read(buffer.data() + subheader.inflatedSize, chunksize);
            subheader.inflatedSize += instream.gcount();
        }

        buffer.resize(subheader.inflatedSize);

        if (globalValues.pack)
        {
            if (WriteSubFileHeader(outstream, subheader, presize))
            {
                std::cerr << "Error: Failed to write sub file header" << std::endl;
                return true;
            }

            outstream.write(buffer.data(), buffer.size());

            if (outstream.fail())
            {
                std::cerr << "Error: Failed to write sub file" << std::endl;
                return true;
            }
        }

        if (!globalValues.quiet)
        {
            std::cout << "size: " << subheader.inflatedSize << std::endl << std::endl;
        }

        presize += subheader.inflatedSize;

        pad = (presize % 4) ? (4 - (presize % 4)) : 0;

        for (unsigned int i = 0; i < pad; ++i)
        {
            if (globalValues.pack)
            {
                outstream.put(0);

                if (outstream.fail())
                {
                    std::cerr << "Error: Failed to pad sub file" << std::endl;
                    return true;
                }
            }

            ++presize;
        }

        ++precount;
    }

    header.size = presize;
    header.numFiles = precount;

    if (globalValues.pack)
    {
        outstream.seekp(0);

        if (WritePreHeader(outstream, header, presize))
        {
            std::cerr << "Error: Failed to write pre file header" << std::endl;
            return true;
        }
    }

    if (!globalValues.quiet)
    {
        std::cout << globalValues.outpath.string() << std::endl;
        std::cout << "total files: " << header.numFiles << std::endl;
        std::cout << "total size: " << header.size << std::endl;
    }
    
    return false;
}

bool WritePreHeader(std::ofstream &outstream, const PreHeader &header, unsigned int &sizeout)
{
    char bytes[12];

    write_u32le(bytes, header.size);
    write_u16le(&bytes[4], 3);
    write_u16le(&bytes[6], 0xabcd);
    write_u32le(&bytes[8], header.numFiles);

    outstream.write(bytes, 12);

    if (outstream.fail())
    {
        return true;
    }

    sizeout += 12;
    return false;
}

bool WriteSubFileHeader(std::ofstream &outstream, const SubFileHeader &subheader, unsigned int &sizeout)
{
    std::vector<char> bytes(16);

    write_u32le(&bytes[0], subheader.inflatedSize);
    write_u32le(&bytes[4], subheader.deflatedSize);
    write_u32le(&bytes[8], subheader.pathSize);
    write_u32le(&bytes[12], subheader.pathCRC);

    bytes.insert(bytes.end(), subheader.path.begin(), subheader.path.end());

    outstream.write(bytes.data(), bytes.size());

    if (outstream.fail())
    {
        return true;
    }

    sizeout += bytes.size();
    return false;
}
