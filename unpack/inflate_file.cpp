// Copyright (c) 2024 Bryan Rykowski
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
#include <vector>

static char ring_buffer[4096];

struct InflateVars
{
	unsigned int in_buffer_pos = 0;
	unsigned int out_buffer_pos = 0;
	unsigned int ring_buffer_pos = 4078; // In THUG2/THUGPRO pre files, the ring buffer starts at 4078 for some reason.
};

static void CopyByte(InflateVars& vars)
{
	// Just copy a literal byte from input to output. Every byte written to the output must also
	// be written to the ring buffer, so it is available for future dict entries.

	ring_buffer[vars.ring_buffer_pos] = Unpack::in_buffer[vars.in_buffer_pos];
	Unpack::out_buffer[vars.out_buffer_pos] = Unpack::in_buffer[vars.in_buffer_pos];
	++vars.in_buffer_pos;
	++vars.out_buffer_pos;
	vars.ring_buffer_pos = (vars.ring_buffer_pos + 1) % 4096;
}

static void ProcessDict(InflateVars& vars)
{
	// A dict is 2 bytes containing an offset and a count. The offset is the position to start 
	// copying from the ring buffer. The count is the number of bytes to copy. As with simple
	// byte copies, the bytes must also be written to the ring buffer.

	// The offset is an unsigned 12 bit integer with the first byte (c0) as the lower 8 bits and
	// the upper 4 bits of the second byte (c1) as the upper 4 bits.

	// The count is an unsigned 4 bit integer made from the lower 4 bits of the second byte (c1).
	// The shortest useful dict count would be 3 so the range of count is 3-18 instead of 0-15.

	// c0:             zzzzzzzz
	// c1:             xxxxyyyy
	// offset:     xxxxzzzzzzzz
	// count:              yyyy

	unsigned int c0 = (unsigned char)Unpack::in_buffer[vars.in_buffer_pos];
	unsigned int c1 = (unsigned char)Unpack::in_buffer[vars.in_buffer_pos + 1];
	vars.in_buffer_pos += 2;
	unsigned int offset = c0 | ((c1 & 0xf0) << 4);
	unsigned int count = (c1 & 0xf) + 3;

	for (unsigned int i = 0; i < count; ++i)
	{
		ring_buffer[vars.ring_buffer_pos] = ring_buffer[(offset + i) % 4096];
		Unpack::out_buffer[vars.out_buffer_pos] = ring_buffer[(offset + i) % 4096];
		++vars.out_buffer_pos;
		vars.ring_buffer_pos = (vars.ring_buffer_pos + 1) % 4096;
	}
}

bool Unpack::InflateFile(std::ifstream& in_stream, std::ofstream& out_stream, const Unpack::EmbeddedFile& file)
{
	in_stream.seekg(file.offset);
	in_stream.read(&Unpack::in_buffer[0], file.lzss_size);

	if (in_stream.fail())
	{
		std::fprintf(stderr, "ERROR: Inflate read error\n");
		return true;
	}

	// The bits in a type byte indicate what the next 8 operations are going be. 1 means just
	// copy a byte from input to output and ring buffer. 0 means read a 2 byte dict and copy 3-18
	// bytes from the ring buffer to the output and ring buffer.

	unsigned char type_byte;
	InflateVars vars;

	while (vars.in_buffer_pos < file.lzss_size)
	{
		type_byte = in_buffer[vars.in_buffer_pos];
		++vars.in_buffer_pos;
		if (vars.in_buffer_pos >= file.lzss_size) {break;}

		for (int i = 0; i < 8; ++i)
		{
			if ((type_byte >> i) & 0x1)
			{
				CopyByte(vars);
				if (vars.in_buffer_pos >= file.lzss_size) {break;}
			}
			else
			{
				ProcessDict(vars);
				if (vars.in_buffer_pos >= file.lzss_size) {break;}
			}
		}
	}

	out_stream.write(&Unpack::out_buffer[0], file.raw_size);

	if (out_stream.fail())
	{
		std::fprintf(stderr, "ERROR: Inflate write error\n");
		return true;
	}

	return false;
}
