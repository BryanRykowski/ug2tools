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

typedef std::pair<std::filesystem::path,std::string> FilePair;

struct
{
    std::filesystem::path prespecpath;
    std::vector<FilePair> filelist;
    PreHeader header;
} globalValues;

bool ReadArgs(int argc, char **argv);
bool ReadPrespec();
bool ReadLine(std::ifstream &instream, std::string &outstr);
bool WritePre();

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

            for (char c : switches)
            {
                if (c == 'f')
                {
                    if (i + 2 >= argc)
                    {
                        std::cerr << "Error: Wrong number of arguments after -f" << std::endl;
                        return true;
                    }

                    globalValues.filelist.push_back(FilePair(std::filesystem::path(argv[i+1]),argv[i+2]));
                    i += 2;
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

    for (FilePair fp : globalValues.filelist)
    {
        std::cout << "file: " << fp.first.string() << std::endl;
        std::cout << "internal path: " << fp.second << std::endl << std::endl;

        ++precount;
    }

    std::cout << "total files: " << precount << std::endl;
    
    return false;
}