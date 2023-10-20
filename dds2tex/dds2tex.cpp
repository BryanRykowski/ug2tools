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

#include <vector>
#include <filesystem>
#include <string>
#include <iostream>
#include <fstream>
#include "../common/dds_header.hpp"
#include "../common/write_word.hpp"
#include "../common/read_word.hpp"
#include "../common/tex_header.hpp"

typedef std::vector<std::filesystem::path> FileList;
typedef std::vector<unsigned int> ChecksumList;

struct OptionStruct
{
	bool write = true;
	bool quiet = false;
};

struct PathStruct
{
	std::filesystem::path out_path = "out.tex.xbx";
	std::filesystem::path checksum_path = "";
	std::filesystem::path list_path = "";
};

bool ReadArgs(int argc, char **argv, PathStruct &paths, FileList &file_list, OptionStruct &options);
bool ReadList(std::filesystem::path &list_path, FileList &file_list);
bool ReadChecksums(std::filesystem::path &checksum_path, ChecksumList &checksum_list);
bool ReadFiles(std::filesystem::path &out_path, FileList &file_list, ChecksumList &checksum_list, OptionStruct &options);

int main(int argc, char **argv)
{
	OptionStruct options;
	PathStruct paths;
	FileList file_list;
	ChecksumList checksum_list;
		
	if (argc < 2)
	{
		std::cerr << "Error: No arguments" << std::endl;
		return -1;
	}
		
	if (ReadArgs(argc, argv, paths, file_list, options)) return -1;

	if (!paths.list_path.empty())
	{
		if (ReadList(paths.list_path, file_list)) return -1;
	}
	
	if (!paths.checksum_path.empty())
	{
		if (ReadChecksums(paths.checksum_path, checksum_list)) return -1;

		if (!options.quiet)
		{
			std::cout << "Read " << checksum_list.size() << " checksums from \"" << paths.checksum_path.string() << "\"" << std::endl << std::endl;
		}
	}

	if (ReadFiles(paths.out_path, file_list, checksum_list, options)) return -1;
		
	return 0;
}

bool ReadArgs(int argc, char **argv, PathStruct &paths, FileList &file_list, OptionStruct &options)
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
				else if (c == 'c')
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
					paths.checksum_path = argv[i];
				}
				else if (c == 'l')
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
					paths.list_path = argv[i];
				}
				else if (c == 'n')
				{
					options.write = false;
				}
				else if (c == 'q')
				{
					options.quiet = true;
				}
			}
		}
		else
		{
			paths.out_path = arg;
		}
	}
		
	return false;
}

bool ReadList(std::filesystem::path &list_path, FileList &file_list)
{
	std::ifstream in_stream(list_path);
	std::string line;

	if (in_stream.fail())
	{
		std::cerr << "Error: Failed to read file list \"" << list_path.string() << "\"" << std::endl;
		return true;
	}

	while (!in_stream.eof())
	{
		std::getline(in_stream, line);

		if (in_stream.fail())
		{
			std::cerr << "Error: Failed to read file list \"" << list_path.string() << "\"" << std::endl;
			return true;
		}

		file_list.push_back(line);
	}

	return false;
}

bool ReadChecksums(std::filesystem::path &checksum_path, ChecksumList &checksum_list)
{
	unsigned int num_images = 0;
	char word[4];
	std::ifstream in_stream(checksum_path, std::ios::binary);

	if (in_stream.fail())
	{
		std::cerr << "Error: Failed to read tex file \"" << checksum_path.string() << "\"" << std::endl;
		return true;
	}

	in_stream.ignore(4);
	in_stream.read(word, 4);

	if (in_stream.fail())
	{
		std::cerr << "Error: Failed to read tex file header" << std::endl;
		return true;
	}

	num_images = read_u32le(word);

	for (unsigned int i = 0; i < num_images; ++i)
	{
		unsigned int num_levels;
		char header[32];

		in_stream.read(header, 32);

		if (in_stream.fail())
		{
			std::cerr << "Error: Failed to read tex image header" << std::endl;
			return true;
		}

		checksum_list.push_back(read_u32le(header));
		num_levels = read_u32le(header + 12);

		for (unsigned int j = 0; j < num_levels; ++j)
		{
			char level_size[4];

			in_stream.read(level_size, 4);

			if (in_stream.fail())
			{
				std::cerr << "Error: Failed to read tex image level size " << j << std::endl;
				return true;
			}

			in_stream.ignore(read_u32le(level_size));

			if (in_stream.fail())
			{
				std::cerr << "Error: Failed to skip tex image level " << j << std::endl;
				return true;
			}
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

bool GetDdsData(std::ifstream &in_stream, std::vector<char> &dds_data, const DdsFileHeader &dds_header)
{
	unsigned int buffer_pos = 0;
	unsigned int total_size = 0;
	unsigned int level_size = dds_header.pitch;

	for (unsigned int i = 0; i < dds_header.levels; ++i)
	{
		total_size += level_size;
		level_size /= 4;
	}

	dds_data.resize(total_size + (dds_header.levels * 4));

	level_size = dds_header.pitch;
	
	for (unsigned int i = 0; i < dds_header.levels; ++i)
	{
		write_u32le(dds_data.data() + buffer_pos, level_size);
		buffer_pos += 4;

		in_stream.read(dds_data.data() + buffer_pos, level_size);
		buffer_pos += level_size;
		
		if (in_stream.fail())
		{
			std::cerr << "Error: Failed to read dds pixel data" << std::endl;
			return true;
		}
		
		level_size /= 4;
	}

	return false;
}

bool WriteImageHeader(std::ofstream &out_stream, TexImageHeader &image_header)
{
	char buffer[32];

	write_u32le(buffer, image_header.checksum);
	write_u32le(buffer + 4, image_header.width);
	write_u32le(buffer + 8, image_header.height);
	write_u32le(buffer + 12, image_header.levels);
	write_u32le(buffer + 16, 32); // Don't know what these are. Often 32.
	write_u32le(buffer + 20, 32);
	write_u32le(buffer + 24, image_header.dxt);
	write_u32le(buffer + 28, 0);

	out_stream.write(buffer, 32);

	if (out_stream.fail())
	{
		std::cerr << "Error: Failed to write image file header" << std::endl;
		return true;
	}

	return false;
}

bool ReadFiles(std::filesystem::path &out_path, FileList &file_list, ChecksumList &checksum_list, OptionStruct &options)
{
	std::ofstream out_stream;
	std::vector<char> dds_data;

	if ((checksum_list.size() != 0) && (checksum_list.size() != file_list.size()))
	{
		std::cerr << "Error: Mismatch between number of checksums and images, " << checksum_list.size() << " checksums and " << file_list.size() << " images" << std::endl;
		return true;
	}

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
		TexImageHeader image_header;

		if (in_stream.fail())
		{
			std::cerr << "Error: Failed to open dds file \"" << file_list[i].string() << "\"" << std::endl;
			return true;
		}

		if (ReadDdsHeader(in_stream, dds_header)) return true;

		if (!options.quiet)
		{
			std::cout << file_list[i].string() << std::endl;
			std::cout << "width: " << dds_header.width << std::endl;
			std::cout << "height: " << dds_header.height << std::endl;
			std::cout << "dxt: " << dds_header.pix_fmt.fourcc[3] << std::endl;
			std::cout << "mipmap levels: " << dds_header.levels << std::endl << std::endl;
		}

		if (options.write)
		{
			if (GetDdsData(in_stream, dds_data, dds_header)) return true;

			image_header.checksum = checksum_list.size() ? checksum_list[i] : 0; 
			image_header.width = dds_header.width;
			image_header.height = dds_header.height;
			image_header.levels = dds_header.levels;

			switch (dds_header.pix_fmt.fourcc[3])
			{
				case '1':
					image_header.dxt = 1;
					break;

				case '2':
					image_header.dxt = 2;
					break;
					
				case '3':
					image_header.dxt = 3;
					break;

				case '4':
					image_header.dxt = 4;
					break;

				case '5':
					image_header.dxt = 5;
					break;
			
				default:
					std::cerr << "Error: DDS file unsupported fourcc \"" << dds_header.pix_fmt.fourcc << "\"" << std::endl;
					return true;
			}

			if (WriteImageHeader(out_stream, image_header)) return true;

			out_stream.write(dds_data.data(), dds_data.size());
			
			if (out_stream.fail())
			{
				std::cerr << "Error: Failed to write image file data" << std::endl;
				return true;
			}
		}
	}

	return false;
}