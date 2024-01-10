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
#include <filesystem>
#include <fstream>

bool Unpack::WriteSpec(const Unpack::Config& config, const Unpack::PreFile& pre)
{
	std::filesystem::path spec_path = config.out_dir;
	std::filesystem::path filename = pre.in_path.filename();
	spec_path /= filename;
	spec_path.replace_extension("prespec");

	if (std::filesystem::exists(spec_path) && !config.overwrite)
	{
		std::fprintf(stderr, "ERROR: File \"%s\" already exists [Hint: pass -w (overwrite) flag]\n", spec_path.c_str());
		return true;
	}

	std::ofstream out_stream(spec_path);

	if (out_stream.fail())
	{
		std::fprintf(stderr, "ERROR: Failed to create file \"%s\"\n", spec_path.c_str());
		return true;
	}

	std::filesystem::path working_dir = std::filesystem::current_path();

	for (const EmbeddedFile& file : pre.files)
	{
		std::filesystem::path path;

		if (config.spec_absolute)
		{
			path = working_dir;
			path /= config.out_dir;
		}

		std::string filename;
		if (PathGetFileName(file.path, filename)) {return true;}
		path /= filename; 

		out_stream << path.string() << "\n" << &file.path[0] << "\n\n";
	}
	return false;
}

