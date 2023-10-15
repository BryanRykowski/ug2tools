#include <vector>
#include <filesystem>
#include <string>
#include <iostream>
#include <fstream>
#include "../common/dds_header.hpp"
#include "../common/write_word.hpp"
#include "../common/read_word.hpp"

typedef std::vector<std::filesystem::path> FileList;

struct OptionStruct
{
	bool write = true;
};

bool ReadArgs(int argc, char **argv, std::filesystem::path &out_path, FileList &file_list, OptionStruct &options);
bool ReadFiles(std::filesystem::path &out_path, FileList &file_list, OptionStruct &options);

int main(int argc, char **argv)
{
	OptionStruct options;
	std::filesystem::path out_path = "out.tex.xbx";
	FileList file_list;
		
	if (argc < 2)
	{
		std::cerr << "Error: No arguments" << std::endl;
		return -1;
	}
		
	if (ReadArgs(argc, argv, out_path, file_list, options)) return -1;

	if (ReadFiles(out_path, file_list, options)) return -1;
		
	return 0;
}

bool ReadArgs(int argc, char **argv, std::filesystem::path &out_path, FileList &file_list, OptionStruct &options)
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

					if (i + 1 >= argc)
					{
						std::cerr << "Error: Wrong number of arguments after -f" << std::endl;
						return true;
					}

					++i;
					file_list.push_back(argv[i]);
				}
				else if (c == 'n')
				{
					options.write = false;
				}
			}
		}
		else
		{
			out_path = arg;
		}
	}
		
	return false;
}

bool WriteTexHeader(std::ofstream &out_stream, unsigned int num_files)
{
	char buffer[8];

	write_u32le(buffer, 1);
	write_u32le(buffer + 4, num_files);

	out_stream.write(buffer, 8);

	if (out_stream.fail())
	{
		std::cerr << "Error: Failed to write tex file header" << std::endl;
		return true;
	}
		
	return false;
}

bool ReadDdsHeader(std::ifstream &in_stream, DdsFileHeader &dds_header)
{
	char buffer[128];

	in_stream.read(buffer, 128);

	if (!(buffer[0] == 'D') || !(buffer[1] == 'D') || !(buffer[2] == 'S') || !(buffer[3] = ' '))
	{
		std::cerr << "Error: DDS file doesn't begin with \"DDS \"" << std::endl;
		return true;
	}

	if (read_u32le(buffer + 4) != 124)
	{
		std::cerr << "Error: DDS file reports header size other than 124" << std::endl;
		return true;
	}

	if (read_u32le(buffer + 76) != 32)
	{
		std::cerr << "Error: DDS file reports pixel format header size other than 32" << std::endl;
		return true;
	}

	dds_header.flags = read_u32le(buffer + 8);
	dds_header.height = read_u32le(buffer + 12);
	dds_header.width = read_u32le(buffer + 16);
	dds_header.pitch = read_u32le(buffer + 20);
	dds_header.depth = read_u32le(buffer + 24);
	dds_header.levels = read_u32le(buffer + 28);
	dds_header.pix_fmt.flags = read_u32le(buffer + 80);
	std::copy(buffer + 84, buffer + 88, dds_header.pix_fmt.fourcc);
	dds_header.pix_fmt.rgb_bits = read_u32le(buffer + 88);
	dds_header.pix_fmt.r_bitmask = read_u32le(buffer + 92);
	dds_header.pix_fmt.g_bitmask = read_u32le(buffer + 96);
	dds_header.pix_fmt.b_bitmask = read_u32le(buffer + 100);
	dds_header.pix_fmt.a_bitmask = read_u32le(buffer + 104);
	dds_header.caps = read_u32le(buffer + 108);
	dds_header.caps2 = read_u32le(buffer + 112);
	
	return false;
}

bool SkipDdsLevels(std::ifstream &in_stream, const DdsFileHeader &dds_header)
{
	for (unsigned int i = 0; i < dds_header.levels; ++i)
	{

	}
	
	return false;
}

bool ReadFiles(std::filesystem::path &out_path, FileList &file_list, OptionStruct &options)
{
	std::ofstream out_stream;

	if (options.write)
	{
		out_stream.open(out_path, std::ios::binary);

		if (out_stream.fail())
		{
			std::cerr << "Error: Failed to open output file \"" << out_path.string() << "\"" << std::endl;
			return true;
		}

		if (WriteTexHeader(out_stream, file_list.size())) return true;
	}

	for (unsigned int i = 0; i < file_list.size(); ++i)
	{
		std::ifstream in_stream(file_list[i], std::ios::binary);
		DdsFileHeader dds_header;

		if (in_stream.fail())
		{
			std::cerr << "Error: Failed to open dds file \"" << file_list[i].string() << "\"" << std::endl;
			return true;
		}

		if (ReadDdsHeader(in_stream, dds_header)) return true;

		if (SkipDdsLevels(in_stream, dds_header)) return true;
	}

	return false;
}