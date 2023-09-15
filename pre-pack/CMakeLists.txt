add_executable (pre-pack pre-pack.cpp ../common/pre_header.hpp ../common/subfile_header.hpp ../common/crc.hpp ../common/crc.cpp)
set_property (TARGET pre-pack PROPERTY CXX_STANDARD 17)