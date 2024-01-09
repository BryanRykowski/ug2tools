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
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <vector>

static bool CopyFile(std::ifstream& in_stream, std::ofstream& out_stream, const Unpack::EmbeddedFile& file)
{
	in_stream.seekg(file.offset);
	in_stream.read(&Unpack::out_buffer[0], file.raw_size);

	if (in_stream.fail())
	{
		std::fprintf(stderr, "ERROR: Copy error (read)\n");
		return true;
	}

	out_stream.write(&Unpack::out_buffer[0], file.raw_size);

	if (out_stream.fail())
	{
		std::fprintf(stderr, "ERROR: Copy error (write)\n");
		return true;
	}

	return false;
}

static bool PathGetFileName(const std::vector<char>& path, std::string& file_name)
{
	if (path.size() < 1)
	{
		std::fprintf(stderr, "ERROR: Malformed path in embedded file\n");
		return true;
	}

	int i = path.size() - 1;
	int start = i;
	int end = path.size();

	while (i >= 0)
	{
		if (path[i] == '\0') {end = i;}
		if (path[i] == '\\') {break;}
		start = i;
		--i;
	}

	file_name.resize(end - start);
	std::memcpy(&file_name[0], &path[start], file_name.size());

	return false;
}

bool Unpack::WriteFiles(const Unpack::Config& config, Unpack::PreFile& pre)
{
	unsigned int largest_lzss = 0;
	unsigned int largest_raw = 0;

	// Make the buffers big enough for the largest file.

	for (const EmbeddedFile& file : pre.files)
	{
		if (file.lzss_size > largest_lzss) {largest_lzss = file.lzss_size;}
		if (file.raw_size > largest_raw) {largest_raw = file.raw_size;}
	}

	in_buffer.resize(largest_lzss);
	out_buffer.resize(largest_raw);

	for (const EmbeddedFile& file : pre.files)
	{
		std::string file_name;
		if (PathGetFileName(file.path, file_name)) {return true;}
		std::filesystem::path out_path = config.out_dir;
		out_path /= file_name;

		if (std::filesystem::exists(out_path) && !config.overwrite)
		{
			std::fprintf(stderr, "ERROR: File \"%s\" already exists [Hint: pass -w (overwrite) flag]\n", out_path.c_str());
			return true;
		}

		std::ofstream out_stream(out_path, std::ios::binary);

		// A lzss of 0 indicates an uncompressed file that can just be copied.

		if (file.lzss_size == 0)
		{
			if (CopyFile(pre.in_stream, out_stream, file)) { return true;}
		}
		else
		{
			if (InflateFile(pre.in_stream, out_stream, file)) { return true;}
		}  
	}

	return false;
}
