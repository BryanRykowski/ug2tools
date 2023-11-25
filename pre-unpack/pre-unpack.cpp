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

#include "../common/pre_header.hpp"
#include "../common/subfile_header.hpp"
#include "../libug2/read_word.hpp"
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
    std::filesystem::path workingdir;

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

    if (globalValues.prespec && globalValues.unpack)
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

        workingdir = std::filesystem::current_path();
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

        for (char c : subheader.path)
        {
            if (c < 32) break; 
            path.push_back(c);
        }

        if (!globalValues.quiet)
        {
            std::cout << std::setw(3) << i << std::setw(10) << subheader.inflatedSize << " " << std::setw(10) << subheader.deflatedSize << std::setw(0) << " " << path << std::endl;
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

        if (globalValues.prespec && globalValues.unpack)
        {
            unsigned int slash_loc = 0;
            std::string internal_path;
            std::string filename;
            
            for (unsigned int j = 0; j < subheader.path.size(); ++j)
            {
                char c = subheader.path[j];
                if (c == '\\') slash_loc = j;
                if (c < 32) break;
                internal_path.push_back(c);
            }
            
            for (unsigned int j = slash_loc + 1; j < internal_path.size(); ++j)
            {
                filename.push_back(internal_path[j]);
            }

            std::filesystem::path filepath;
            
            if (globalValues.prespecfullpath)
            {
                filepath = workingdir;
                filepath /= globalValues.outDir;
            }

            filepath /= filename;

            prespecstream << filepath.string() << std::endl;
            prespecstream << internal_path << std::endl << std::endl;
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
    std::cout << "Usage: ug2-pre-unpack [FILE] [OPTION]..." << std::endl << std::endl;
    std::cout << "Extract files embedded in pre/prx files." << std::endl << std::endl;
    std::cout << "Example:" << std::endl << std::endl;
    std::cout << "        ug2-pre-unpack infile.prx -wo data/pre" << std::endl << std::endl;
    std::cout << "        Lists the contents of \"infile.prx\" and extracts them to" << std::endl;
    std::cout << "        ./data/pre, overwriting any existing versions of the files." << std::endl << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "    -h              Print this help text" << std::endl;
    std::cout << "    -o DIRECTORY    Place files in DIRECTORY instead of current directory" << std::endl;
    std::cout << "    -q              Suppress some output. Does not include errors" << std::endl;
    std::cout << "    -w              Overwrite existing files" << std::endl;
    std::cout << "    -p              Disable prespec file generation." << std::endl;
    std::cout << "    -P              Disable absolute paths in prespec file." << std::endl;
    std::cout << "    -n              Don't extract files or generate prespec." << std::endl;
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

    outheader.size = read_u32le(bytes);
    outheader.version = read_u16le(&bytes[4]);
    outheader.unknown = read_u16le(&bytes[6]);
    outheader.numFiles = read_u32le(&bytes[8]);
    
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

    outsubheader.inflatedSize = read_u32le(bytes);
    outsubheader.deflatedSize = read_u32le(&bytes[4]);
    outsubheader.pathSize = read_u32le(&bytes[8]);
    outsubheader.pathCRC = read_u32le(&bytes[12]);

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

    if (subheader.deflatedSize == 0)
    {
        skipCount = subheader.inflatedSize;
    }
    else
    {
        skipCount = subheader.deflatedSize;
    }

    // Align the skip to a multiple of 4.
    skipCount += (skipCount % 4) ? (4 - (skipCount % 4)) : 0;

    infile.ignore(skipCount);

    if (infile.fail() || infile.gcount() != skipCount)
    {
        std::cerr << "Error: Failed to skip sub file" << std::endl;
        return true;
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

    null_loc = subheader.path.size();

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
        std::cerr << "Error: file \"" << outpath << "\" already exists and overwrite not enabled" << std::endl;
        return true;
    }

    outfile.open(outpath, outfile.binary);

    if (outfile.fail())
    {
        std::cerr << "Error: Unable to create file \"" << outpath << "\"" << std::endl;
        return true;
    }

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
                std::cerr << "Error: Failed to inflate subfile" << std::endl;
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
                std::cerr << "Error: Failed to inflate subfile" << std::endl;
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
                        std::cerr << "Error: Failed to inflate subfile" << std::endl;
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
                        std::cerr << "Error: Failed to inflate subfile" << std::endl;
                        return true;
                    }
                    ++subfile_pos;

                    b0 = static_cast<unsigned char>(c);

                    if (!infile.get(c))
                    {
                        std::cerr << "Error: Failed to inflate subfile" << std::endl;
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
    
    padding = (readCount % 4) ? (4 - (readCount % 4)) : 0;

    for (unsigned int i = 0; i < padding; ++i)
    {
        if (!infile.good())
        {
            std::cerr << "Error: Failed to pad subfile" << std::endl;
            return true;
        }

        infile.get();
    }

    return false;
}