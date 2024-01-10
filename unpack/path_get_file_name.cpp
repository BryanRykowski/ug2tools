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
#include <cstring>

bool Unpack::PathGetFileName(const std::vector<char>& path, std::string& file_name)
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
