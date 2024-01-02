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

#pragma once
#include <filesystem>
#include <fstream>
#include <cstdint>
#include <vector>
#include "../common/pre_header.hpp"

namespace Unpack
{
	constexpr const char* app_version = "1.1.0";

	struct Config
	{
		std::filesystem::path in_file;
		std::filesystem::path out_dir;
		bool quiet = false;
		bool write = true;
		bool overwrite = false;
		bool write_spec = true;
		bool spec_absolute = true;
		bool print_help = false;
		bool print_version = false;
	};

	struct EmbeddedFile
	{
		std::vector<char> path;
		uint32_t offset;
		uint32_t raw_size;
		uint32_t lzss_size;
		uint32_t crc;
	};

	struct PreFile
	{
		std::filesystem::path in_file;
		std::vector<Unpack::EmbeddedFile> files;
		uint32_t size;
	};

	bool GetConfig(int argc, char** argv, Unpack::Config& config);
	void PrintHelp();
	void PrintVersion();
	bool Unpack(const Unpack::Config& config);
	bool ReadPre(const std::filesystem::path in_file, Unpack::PreFile& pre);
}