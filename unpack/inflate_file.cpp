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

#include "unpack.hpp"
#include <vector>

static std::vector<char> in_buffer;
static std::vector<char> out_buffer;
static char ring_buffer[4096];

struct InflateVars
{
	unsigned int in_buffer_pos = 0;
	unsigned int out_buffer_pos = 0;
	unsigned int ring_buffer_pos = 4078;
};

static void CopyByte(InflateVars& vars)
{
	ring_buffer[vars.ring_buffer_pos] = in_buffer[vars.in_buffer_pos];
	out_buffer[vars.out_buffer_pos] = in_buffer[vars.in_buffer_pos];
	++vars.in_buffer_pos;
	++vars.out_buffer_pos;
	vars.ring_buffer_pos = (vars.ring_buffer_pos + 1) % 4096;
}

static void ProcessDict(InflateVars& vars)
{
	unsigned int c0 = (unsigned char)in_buffer[vars.in_buffer_pos];
	unsigned int c1 = (unsigned char)in_buffer[vars.in_buffer_pos + 1];
	vars.in_buffer_pos += 2;
	unsigned int offset = c0 | ((c1 & 0xf0) << 4);
	unsigned int count = (c1 & 0xf) + 3;

	for (unsigned int i = 0; i < count; ++i)
	{
		ring_buffer[vars.ring_buffer_pos] = ring_buffer[(offset + i) % 4096];
		out_buffer[vars.out_buffer_pos] = ring_buffer[(offset + i) % 4096];
		++vars.out_buffer_pos;
		vars.ring_buffer_pos = (vars.ring_buffer_pos + 1) % 4096;
	}
}

bool Unpack::InflateFile(std::ifstream& in_stream, std::ofstream& out_stream, const Unpack::EmbeddedFile& file)
{
	in_buffer.resize(file.lzss_size);
	out_buffer.resize(file.raw_size);
	in_stream.seekg(file.offset);
	in_stream.read(&in_buffer[0], file.lzss_size);

	if (in_stream.fail())
	{
		std::fprintf(stderr, "ERROR: Inflate read error\n");
		return true;
	}

	unsigned char type_byte;
	InflateVars vars;

	while (vars.in_buffer_pos < file.lzss_size)
	{
		type_byte = in_buffer[vars.in_buffer_pos];
		++vars.in_buffer_pos;

		for (int i = 0; i < 8; ++i)
		{
			if ((type_byte >> i) & 0x1)
			{
				CopyByte(vars);
			}
			else
			{
				ProcessDict(vars);
			}
		}
	}

	out_stream.write(&out_buffer[0], file.raw_size);

	if (out_stream.fail())
	{
		std::fprintf(stderr, "ERROR: Inflate write error\n");
		return true;
	}

	return false;
}
