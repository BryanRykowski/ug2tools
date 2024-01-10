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
#include <string>

constexpr const char* help_str =
	"Usage: ug2-pre-unpack [FILE] [OPTION]...\n\n"
	"Extract files embedded in pre/prx files.\n\n"
	"Example:\n\n"
	"        ug2-pre-unpack infile.prx -wo data/pre\n\n"
	"        Lists the contents of \"infile.prx\" and extracts them to\n\n"
	"        ./data/pre, overwriting any existing versions of the files.\n\n"
	"Options:\n"
	"    -h              Print this help text\n"
	"    -o DIRECTORY    Place files in DIRECTORY instead of current directory\n"
	"    -q              Suppress some output. Does not include errors\n"
	"    -w              Overwrite existing files\n"
	"    -p              Disable prespec file generation.\n"
	"    -P              Disable absolute paths in prespec file.\n"
	"    -n              Don't extract files or generate prespec.\n"
	"    -V              Print version information.\n";

bool Unpack::GetConfig(int argc, char** argv, Unpack::Config& config)
{
	std::string arg;
	
	if (argc < 2) {return false;}

	for (int i = 1; i < argc; ++i)
	{
		arg = argv[i];

		if ((arg.size() > 1) && (arg[0] == '-') && (arg[1] != '-'))
		{
			for (unsigned int j = 1; j < arg.size(); ++j)
			{
				switch (arg[j])
				{
				case 'h':
					config.print_help = true;
					break;
				case 'o':
					if (i >= (argc - 1))
					{
						std::fprintf(stderr, "ERROR: Missing argument after -o\n");
						return true;
					}

					++i;
					config.out_dir = argv[i];
					break;
				case 'q':
					config.quiet = true;
					break;
				case 'w':
					config.overwrite = true;
					break;
				case 'p':
					config.write_spec = false;
					break;
				case 'P':
					config.spec_absolute = false;
					break;
				case 'n':
					config.write = false;
					break;
				case 'V':
					config.print_version = true;
					break;
				default:
					std::fprintf(stderr, "ERROR: Unrecognized argument \"-%c\"\n", arg[j]);
					return true;
				}
			}
		}
		else
		{
			config.in_file = arg;
		}
	}

	return false;
}

void Unpack::PrintHelp()
{
	std::printf("%s", help_str);
}

void Unpack::PrintVersion()
{
	std::printf("ug2-tools %s\nunpack %s\n", "0.3.0", Unpack::app_version);
}

static void ListInfo(const Unpack::PreFile& pre)
{
	std::printf("files: %zu\n", pre.files.size());
	std::printf("size: %u\n", pre.size);
	std::printf("lszz       raw        path\n");

  std::string path;

	for (const Unpack::EmbeddedFile& file : pre.files)
	{
		path.resize(file.path.size());
    	std::memcpy(&path[0], &file.path[0], file.path.size());
		std::printf("%-10u %-10u %s\n", file.lzss_size, file.raw_size, path.c_str());
	}
}

static bool WriteSpec(const Unpack::Config& config, const Unpack::PreFile& pre)
{
	(void)config;
	(void)pre;
	return false;
}

bool Unpack::Unpack(const Unpack::Config& config)
{
	if (!config.quiet)
	{
		std::printf("unpack | Bryan Rykowski 2023\n");
	}

	if (config.print_help)
	{
		if (config.print_version) {PrintVersion();}
		PrintHelp();
		return false;
	}

	if (config.print_version) {PrintVersion();}
	
	if (config.in_file.empty())
	{
		std::fprintf(stderr, "ERROR: No input file\n");
		PrintHelp();
		return true;
	}

	PreFile pre;

	if (Unpack::ReadPre(config.in_file, pre)) {return true;}

	if (!config.quiet)
	{
		ListInfo(pre);
	}

	if (config.write_spec)
	{
		if (WriteSpec(config, pre)) {return true;}
	}

	if (config.write)
	{
		if (Unpack::WriteFiles(config, pre)) {return true;}
	}

	return false;
}
