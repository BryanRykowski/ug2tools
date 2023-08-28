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
    std::string inpath = "";
    std::string outDir = "";
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

    if (globalValues.inpath == "")
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
            if (ExtractSubFile(instream, subheader))
            {
                std::cerr << "Unpacking failed." << std::endl;
                return -1;
            }
        }
        else
        {
            if (SkipSubFile(instream, subheader))
            {
                std::cerr << "Unpacking failed." << std::endl;
                return -1;
            }
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
                else if (c == 'o')
                {
                    if ((i + 1) >= argc)
                    {
                        std::cerr << "Error: No output directory provided after -o argument" << std::endl;
                        return true;
                    }
                    
                    i++;
                    globalValues.outDir = argv[i];

                    if (globalValues.outDir.back() != '/')
                    {
                        globalValues.outDir.push_back('/');
                    }
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

    outsubheader.path.reserve(outsubheader.pathSize);
    outsubheader.path.resize(outsubheader.pathSize);
    infile.read(reinterpret_cast<std::ifstream::char_type*>(&outsubheader.path.front()), outsubheader.pathSize);

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
    std::string outpath;
    std::vector<char> buffer;
    unsigned int buffer_pos = 0xfee;
    unsigned int subfile_pos = 0;
    unsigned int path_pos = 0;
    unsigned int slash_loc;
    unsigned int readCount;
    unsigned int padding;

    for (char c : subheader.path)
    {
        if (c == '\\') {slash_loc = path_pos;}
        ++path_pos;
    }

    outpath = std::string(subheader.path.begin(), subheader.path.end());
    outpath = globalValues.outDir + outpath.substr(slash_loc + 1, outpath.size() - 1);

    if (!globalValues.overwrite && std::filesystem::exists(outpath))
    {
        std::cout << "Error: file \"" << outpath << "\" already exists and overwrite not enabled" << std::endl;
        return true;
    }

    outfile.open(outpath, outfile.binary);

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
        readCount = subheader.deflatedSize;
        buffer.reserve(4096);

        while (subfile_pos < readCount)
        {
            char c;
            unsigned char type_byte;

            if (!infile.get(c))
            {
                std::cout << "Error: Failed to inflate subfile" << std::endl;
                return true;
            }
            ++subfile_pos;

            type_byte = static_cast<unsigned char>(c);

            for (int i = 0; i < 8; ++i)
            {
                if ((type_byte >> i) & 0x1)
                {
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
                    buffer_pos = (buffer_pos + 1) % 4096;

                    outfile.put(c);
                }
                else
                {
                    unsigned char b0, b1;
                    unsigned int offset;
                    unsigned int count;
                    
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

                    offset = b0;
                    offset |= static_cast<unsigned int>(b1 & 0xf0) << 4;

                    count = (b1 & 0x0f) + 3;

                    for (unsigned int j = 0; j < count; ++j)
                    {
                        buffer[buffer_pos] = buffer[offset];
                        buffer_pos = (buffer_pos + 1) % 4096;

                        outfile.put(buffer[offset]);
                        offset = (offset + 1) % 4096;
                    }
                }
            }
        }
    }

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