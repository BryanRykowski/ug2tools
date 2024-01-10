// Copyright (c) 2023-2024 Bryan Rykowski
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

#include "unpack.hpp"
#include "../libug2/read_word.hpp"
#include <cstdint>
#include <cstdio>
#include <vector>
#include <fstream>

static bool ReadHeader(std::ifstream& in_stream, uint32_t& size, uint32_t& num_files)
{
	char buffer[12];
	in_stream.read(buffer, 12);

	if (!in_stream.good())
	{
		std::fprintf(stderr, "ERROR: Failed to read header");
		return true;
	}

	size = read_u32le(&buffer[0]);
	num_files = read_u32le(&buffer[8]);

	return false;
}

static bool ReadSubFile(std::ifstream& in_stream, Unpack::EmbeddedFile& embedded_file)
{
	char buffer[16];
	in_stream.read(buffer, 16);

	if (!in_stream.good())
	{
		std::fprintf(stderr, "ERROR: Failed to read embedded file header");
		return true;
	}

	embedded_file.raw_size = read_u32le(&buffer[0]);
	embedded_file.lzss_size = read_u32le(&buffer[4]);
	unsigned int skip_length = embedded_file.lzss_size ? embedded_file.lzss_size : embedded_file.raw_size; // Uncompressed files have lzss = 0
	unsigned int remain = skip_length % 4; // Everything in a pre file is 4 byte aligned
	skip_length += remain ? (4 - remain) : 0;
	unsigned int path_length = read_u32le(&buffer[8]);
	embedded_file.crc = read_u32le(&buffer[12]);
	embedded_file.path.resize(path_length);
	in_stream.read(&embedded_file.path[0], path_length);

	if (!in_stream.good())
	{
		std::fprintf(stderr, "ERROR: Failed to read embedded file path");
		return true;
	}

	embedded_file.offset = in_stream.tellg();
	in_stream.ignore(skip_length);

	return false;
}

bool Unpack::ReadPre(const std::filesystem::path in_file, Unpack::PreFile& pre)
{
	if (std::filesystem::is_directory(in_file))
	{
		std::fprintf(stderr, "ERROR: \"%s\" is a directory\n", in_file.string().c_str());
		return true;
	}

	pre.in_stream.open(in_file, std::ios::binary);
	
	if (!pre.in_stream.good())
	{
		std::fprintf(stderr, "ERROR: Failed to open file \"%s\"\n", in_file.string().c_str());
		return true;
	}

	pre.in_path = in_file;

	uint32_t num_files;
	if (ReadHeader(pre.in_stream, pre.size, num_files)) {return true;}

	for (unsigned int i = 0; i < num_files; ++i)
	{
		EmbeddedFile embedded_file;
		if (ReadSubFile(pre.in_stream, embedded_file)) {return true;}
		pre.files.push_back(embedded_file);
	}
	
	return false;
}
