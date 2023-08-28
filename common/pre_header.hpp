#pragma once

struct PreHeader
{
    unsigned int size;
    unsigned short version;
    unsigned short unknown;
    unsigned int numFiles;
};