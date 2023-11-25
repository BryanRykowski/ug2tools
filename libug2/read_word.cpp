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

#include "read_word.hpp"

uint16_t read_u16le(const char *in_data)
{
    uint16_t out_word = 0;
    
    out_word = static_cast<unsigned char>(in_data[0]);
    out_word |= static_cast<unsigned char>(in_data[1]) << 8;
    
    return out_word;
}

uint32_t read_u32le(const char *in_data)
{
    uint32_t out_word;

    out_word = static_cast<unsigned char>(in_data[0]);
    out_word |= static_cast<unsigned char>(in_data[1]) << 8;
    out_word |= static_cast<unsigned char>(in_data[2]) << 16;
    out_word |= static_cast<unsigned char>(in_data[3]) << 24;
    
    return out_word;
}
