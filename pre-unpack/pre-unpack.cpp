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

#include "../common/pre_header.hpp"
#include "../common/subfile_header.hpp"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <filesystem>

struct
{
    bool printhelp = false;
    bool unpack = true;
    bool quiet = false;
    bool overwrite = false;
    bool prespec = true;
    bool prespecfullpath = true;
    std::filesystem::path inpath;
    std::filesystem::path outDir;
} globalValues;

void PrintHelp();
bool ReadArgs(int argc, char **argv);
bool ReadHeader(std::ifstream &infile, PreHeader &outheader);
bool ReadSubFileHeader(std::ifstream &infile, SubFileHeader &outsubheader);
bool SkipSubFile(std::ifstream &infile, const SubFileHeader &subheader);
bool ExtractSubFile(std::ifstream &infile, const SubFileHeader &subheader);

int main(int argc, char **argv)
{
    PreHeader header;
    std::ifstream instream;
    std::ofstream prespecstream;

    if (!(argc > 1))
    {
        std::cerr << "Error: No arguments" << std::endl;
        std::cerr << "Unpacking failed." << std::endl;
        PrintHelp();
        return -1;
    }

    if (ReadArgs(argc, argv))
    {
        std::cerr << "Unpacking failed." << std::endl;
        return -1;
    }

    if (globalValues.printhelp)
    {
        PrintHelp();
        return 0;
    }

    if (globalValues.inpath.empty())
    {
        std::cerr << "Error: No input file" << std::endl;
        std::cerr << "Unpacking failed." << std::endl;
        return -1;
    }

    instream.open(globalValues.inpath, instream.binary);

    if (!instream.good())
    {
        std::cerr << "Error: Failed to open input file" << std::endl;
        std::cerr << "Unpacking failed." << std::endl;
        return -1;
    }

    if (globalValues.prespec)
    {
        std::filesystem::path prespecpath = globalValues.outDir / globalValues.inpath.filename();
        prespecpath.replace_extension("prespec");

        if (!globalValues.overwrite && std::filesystem::exists(prespecpath))
        {
            std::cerr << "Error: file \"" << prespecpath << "\" already exists and overwrite not enabled" << std::endl;
            std::cerr << "Unpacking failed." << std::endl;
            return -1;
        }

        prespecstream.open(prespecpath);

        if (prespecstream.fail()) // Check fail() instead of good(). Seems like eof causes good() to return false when opening a new file on mingw-g++.
        {
            std::cerr << "Error: Failed to create prespec file" << std::endl;
            std::cerr << "Unpacking failed." << std::endl;
            return -1;
        }
    }

    if (ReadHeader(instream, header))
    {
        std::cerr << "Error: Failed to read pre/prx header" << std::endl;
        std::cerr << "Unpacking failed." << std::endl;
        return -1;
    }

    if (!globalValues.quiet)
    {
        std::cout << std::endl;
        std::cout << "Size: " << header.size << std::endl;
        std::cout << "Version: " << header.version << std::endl;
        std::cout << "Files: " << header.numFiles << std::endl;
        std::cout << std::endl; 
        std::cout << "Index | Inflated Size | Deflated Size | Path" << std::endl;
        std::cout << std::endl;
    }

    // Loop though the subfiles.
    for (unsigned int i = 0; i < header.numFiles; ++i)
    {
        SubFileHeader subheader;
        std::string path;

        if (ReadSubFileHeader(instream, subheader))
        {
            std::cerr << "Error: Failed to read sub file header " << i << std::endl;
            std::cerr << "Unpacking failed." << std::endl;
            return -1;
        }

        path = std::string(subheader.path.begin(), subheader.path.end());

        if (!globalValues.quiet)
        {
            std::cout << std::setw(2) << i << std::setw(10) << subheader.inflatedSize << " " << std::setw(10) << subheader.deflatedSize << std::setw(0) << " " << path << std::endl;
        }

        if (globalValues.unpack)
        {
            if (ExtractSubFile(instream, subheader)) // Inflate the file.
            {
                std::cerr << "Unpacking failed." << std::endl;
                return -1;
            }
        }
        else
        {
            if (SkipSubFile(instream, subheader)) // Or just jump to the next header if -n flag is set.
            {
                std::cerr << "Unpacking failed." << std::endl;
                return -1;
            }
        }

        if (globalValues.prespec)
        {
            std::vector<char>::difference_type slash_loc = 0;
            std::vector<char>::difference_type null_loc = 0;

            for (unsigned int i = 0; i < subheader.path.size(); ++i)
            {
                if (subheader.path[i] == '\\') {slash_loc = i + 1;}
            }

            for (int i = subheader.path.size() - 1; i >=0; --i)
            {
                if (subheader.path[i] == 0) {null_loc = i;}
            }

            std::filesystem::path filepath;
            
            if (globalValues.prespecfullpath)
            {
                filepath = std::filesystem::current_path();
                filepath /= globalValues.outDir;
            }

            filepath /= std::string(subheader.path.begin() + slash_loc, subheader.path.begin() + null_loc);

            prespecstream << filepath.string() << std::endl;
            prespecstream << std::string(subheader.path.begin(), subheader.path.begin() + null_loc) << std::endl << std::endl;
        }
    }

    if (!globalValues.quiet)
    {
        std::cout << "Unpacking successful." << std::endl;
    }
    
    return 0;
}

void PrintHelp()
{
    std::cout << "Usage: pre-unpack [FILE] [OPTION]..." << std::endl << std::endl;
    std::cout << "Extract files embedded in pre/prx files." << std::endl << std::endl;
    std::cout << "Example:" << std::endl << std::endl;
    std::cout << "        pre-unpack infile.prx -wo data/pre" << std::endl << std::endl;
    std::cout << "        Lists the contents of \"infile.prx\" and extracts them to" << std::endl;
    std::cout << "        ./data/pre, overwriting any existing versions of the files." << std::endl << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "    -h              Print this help text" << std::endl;
    std::cout << "    -o DIRECTORY    Place files in DIRECTORY instead of current directory" << std::endl;
    std::cout << "    -q              Suppress some output. Does not include errors" << std::endl;
    std::cout << "    -w              Overwrite existing files" << std::endl;
    std::cout << "    -p              Disable prespec file generation." << std::endl;
    std::cout << "    -P              Disable absolute paths in prespec file." << std::endl;
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
                    globalValues.quiet = true;
                }
                else if (c == 'n')
                {
                    globalValues.unpack = false;
                }
                else if (c == 'w')
                {
                    globalValues.overwrite = true;
                }
                else if (c == 'h')
                {
                    globalValues.printhelp = true;
                }
                else if (c == 'p')
                {
                    globalValues.prespec = false;
                }
                else if (c == 'P')
                {
                    globalValues.prespecfullpath = false;
                }
                else if (c == 'o')
                {
                    if ((i + 1) >= argc)
                    {
                        std::cerr << "Error: No output directory provided after -o argument" << std::endl;
                        return true;
                    }
                    
                    i++;
                    globalValues.outDir = argv[i];
                }
            }
        }
        else
        {
            globalValues.inpath = arg;
        }
    }

    return false;
}

bool ReadHeader(std::ifstream &infile, PreHeader &outheader)
{
    char bytes[12];

    infile.read(bytes, 12);

    if (!infile.good())
    {
        return true;
    }

    // This might seem like overkill but I want this to work on big endian platforms just in case.
    outheader.size = static_cast<unsigned char>(bytes[0]);
    outheader.size |= static_cast<unsigned char>(bytes[1]) << 8;
    outheader.size |= static_cast<unsigned char>(bytes[2]) << 16;
    outheader.size |= static_cast<unsigned char>(bytes[3]) << 24;

    outheader.version = static_cast<unsigned char>(bytes[4]);
    outheader.version |= static_cast<unsigned char>(bytes[5]) << 8;
    
    outheader.unknown = static_cast<unsigned char>(bytes[6]);
    outheader.unknown |= static_cast<unsigned char>(bytes[7]) << 8;

    outheader.numFiles = static_cast<unsigned char>(bytes[8]);
    outheader.numFiles |= static_cast<unsigned char>(bytes[9]) << 8;
    outheader.numFiles |= static_cast<unsigned char>(bytes[10]) << 16;
    outheader.numFiles |= static_cast<unsigned char>(bytes[11]) << 24;
    
    return false;
}

bool ReadSubFileHeader(std::ifstream &infile, SubFileHeader &outsubheader)
{
    char bytes[16];

    infile.read(bytes, 16);

    if (!infile.good())
    {
        return true;
    }

    outsubheader.inflatedSize = static_cast<unsigned char>(bytes[0]);
    outsubheader.inflatedSize |= static_cast<unsigned char>(bytes[1]) << 8;
    outsubheader.inflatedSize |= static_cast<unsigned char>(bytes[2]) << 16;
    outsubheader.inflatedSize |= static_cast<unsigned char>(bytes[3]) << 24;

    outsubheader.deflatedSize = static_cast<unsigned char>(bytes[4]);
    outsubheader.deflatedSize |= static_cast<unsigned char>(bytes[5]) << 8;
    outsubheader.deflatedSize |= static_cast<unsigned char>(bytes[6]) << 16;
    outsubheader.deflatedSize |= static_cast<unsigned char>(bytes[7]) << 24;

    outsubheader.pathSize = static_cast<unsigned char>(bytes[8]);
    outsubheader.pathSize |= static_cast<unsigned char>(bytes[9]) << 8;
    outsubheader.pathSize |= static_cast<unsigned char>(bytes[10]) << 16;
    outsubheader.pathSize |= static_cast<unsigned char>(bytes[11]) << 24;

    outsubheader.pathCRC = static_cast<unsigned char>(bytes[12]);
    outsubheader.pathCRC |= static_cast<unsigned char>(bytes[13]) << 8;
    outsubheader.pathCRC |= static_cast<unsigned char>(bytes[14]) << 16;
    outsubheader.pathCRC |= static_cast<unsigned char>(bytes[15]) << 24;

    outsubheader.path.resize(outsubheader.pathSize);
    infile.read(reinterpret_cast<std::ifstream::char_type*>(&outsubheader.path.front()), outsubheader.pathSize);

    // Just like every other section of a pre/prx file, the subfile headers are 4 byte aligned.
    // However, the subfile path length includes the padding at the end, so we don't have to manually skip any
    // bytes.

    if (infile.gcount() != outsubheader.pathSize)
    {
        return true;
    }

    return false;
}

bool SkipSubFile(std::ifstream &infile, const SubFileHeader &subheader)
{
    unsigned int skipCount;
    unsigned int padding;

    if (subheader.deflatedSize == 0)
    {
        skipCount = subheader.inflatedSize;
    }
    else
    {
        skipCount = subheader.deflatedSize;
    }

    padding = skipCount % 4;
    
    if (padding)
    {
        padding = (4 - padding);
        skipCount += padding;
    }

    // Every segment of a pre/prx file is aligned to a 4 byte boundary.
    // This means we need to skip between 1 and 3 bytes to get to the next subfile's header.

    for (unsigned int i = 0; i < skipCount; ++i)
    {
        if (!infile.good())
        {
            std::cout << "Error: Failed to skip subfile" << std::endl;
            return true;
        }

        infile.get();
    }

    return false;
}

bool ExtractSubFile(std::ifstream &infile, const SubFileHeader &subheader)
{
    std::ofstream outfile;
    std::filesystem::path outpath;
    std::string filename;
    std::vector<char> buffer;
    unsigned int buffer_pos = 0xfee;
    unsigned int subfile_pos = 0;
    unsigned int slash_loc = 0;
    unsigned int null_loc = 0;
    unsigned int readCount;
    unsigned int padding;
    
    for (unsigned int i = 0; i < subheader.path.size(); ++i)
    {
        if (subheader.path[i] == '\\') {slash_loc = i;}
    }

    for (int i = subheader.path.size() - 1; i >=0; --i)
    {
        if (subheader.path[i] == 0) {null_loc = i;}
    }

    for (unsigned int i = slash_loc + 1; i < null_loc; ++i)
    {
        filename.push_back(subheader.path[i]);
    }

    outpath = globalValues.outDir / filename; 

    // Check if the file already exists and fail if necessary.
    if (!globalValues.overwrite && std::filesystem::exists(outpath))
    {
        std::cout << "Error: file \"" << outpath << "\" already exists and overwrite not enabled" << std::endl;
        return true;
    }

    outfile.open(outpath, outfile.binary);

    // Check if the subfile is compressed. Uncompressed files have a deflated
    // size of 0.
    if (subheader.deflatedSize == 0)
    {
        char c;

        readCount = subheader.inflatedSize;

        while (subfile_pos < readCount)
        {
            if (!infile.get(c))
            {
                std::cout << "Error: Failed to inflate subfile" << std::endl;
                return true;
            }
            
            outfile.put(c);
            ++subfile_pos;
        }
    }
    else
    {
        // pre/prx files use LZSS compression. Data is stored in groups starting with a type byte.
        // Each 1 bit indicates a regular byte, while each 0 indicates a 2 byte offset/length pair.
        // This means that each segment will be between 9 and 17 bytes.
        //
        //     Example:
        //     
        //     D - regular byte
        //     L - offset/length low byte
        //     H - offset/length high byte
        //     
        //     [01110111][D][D][D][L][H][D][D][D][L][H]
        //
        // The last segment will likely be shorter than 8 pieces. The deflatedSize in the header
        // should be used to decide when to stop.
        //
        // The offset/length pairs contain a 12 bit offset and 4 bit length indicating a start point
        // and run length to be read from the ring buffer. The offset is made from combining the low
        // byte with the 4 high bits of the high byte:
        //
        //      o/l high  o/l low       offset
        //     [hhhhxxxx][llllllll] -> [hhhhllllllll]
        //
        // The 4 low bits of the high byte are the number of bytes to read from the buffer. The actual
        // length to read is the value of those bits + 3, meaning anywhere from 3 to 18.
        //
        // The buffer is a 4KiB ring buffer that starts being written to at offset 0xFEE (4078). Every
        // byte written to the output file is also written to the buffer.

        readCount = subheader.deflatedSize;
        buffer.resize(4096);

        while (subfile_pos < readCount)
        {
            char c;
            unsigned char type_byte;

            // Read the type byte of the next segment.
            if (!infile.get(c))
            {
                std::cout << "Error: Failed to inflate subfile" << std::endl;
                return true;
            }
            ++subfile_pos;

            type_byte = static_cast<unsigned char>(c);

            // Loop through the 8 pieces of the segment. Each of these is either a regular byte or an
            // offset/length pair.
            for (int i = 0; i < 8; ++i)
            {
                if ((type_byte >> i) & 0x1) // If this piece is a regular byte, just write it to the buffer and output file.
                {
                    
                    // Check if we've hit the end of the compressed file data.
                    if (subfile_pos >= readCount)
                    {
                        break;
                    }

                    if (!infile.get(c))
                    {
                        std::cout << "Error: Failed to inflate subfile" << std::endl;
                        return true;
                    }
                    ++subfile_pos;

                    buffer[buffer_pos] = c;
                    buffer_pos = (buffer_pos + 1) % 4096; // Use mod 4096 to make sure we wrap around.

                    outfile.put(c);
                }
                else // Otherwise, read the offset and length pair.
                {
                    unsigned char b0, b1;
                    unsigned int offset;
                    unsigned int count;
                    
                    // Check if we've hit the end of the compressed file data. 
                    if (subfile_pos >= readCount)
                    {
                        break;
                    }
                    
                    if (!infile.get(c))
                    {
                        std::cout << "Error: Failed to inflate subfile" << std::endl;
                        return true;
                    }
                    ++subfile_pos;

                    b0 = static_cast<unsigned char>(c);

                    if (!infile.get(c))
                    {
                        std::cout << "Error: Failed to inflate subfile" << std::endl;
                        return true;
                    }
                    ++subfile_pos;

                    b1 = static_cast<unsigned char>(c);

                    // Unpack the offset and length from the pair.
                    offset = b0;
                    offset |= static_cast<unsigned int>(b1 & 0xf0) << 4;

                    count = (b1 & 0x0f) + 3;

                    // Read [count] bytes from the buffer starting at [offset], writing them to the
                    // end of the buffer and the output file.
                    for (unsigned int j = 0; j < count; ++j)
                    {
                        buffer[buffer_pos] = buffer[offset];
                        buffer_pos = (buffer_pos + 1) % 4096; // Use mod 4096 to make sure we wrap around.

                        outfile.put(buffer[offset]);
                        offset = (offset + 1) % 4096;
                    }
                }
            }
        }
    }

    // Every section of a pre/prx file is aligned to 4 byte boundaries. If the subfile is not a multiple of 4
    // bytes long we need to skip between 1 and 3 bytes to get to the next subfile's header.
    
    padding = readCount % 4;

    if (padding)
    {
        padding = (4 - padding);
    }

    for (unsigned int i = 0; i < padding; ++i)
    {
        if (!infile.good())
        {
            std::cout << "Error: Failed to pad subfile" << std::endl;
            return true;
        }

        infile.get();
    }

    return false;
}