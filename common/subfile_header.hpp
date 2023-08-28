#pragma once
#include <vector>
#include <stdint.h>

struct SubFileHeader
{
    unsigned int inflatedSize;
    unsigned int deflatedSize;
    unsigned int pathSize;
    unsigned int pathCRC;
    std::vector<char> path;
};