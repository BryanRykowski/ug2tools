add_executable (pre-unpack pre-unpack.cpp ../common/pre_header.hpp ../common/subfile_header.hpp)
set_property (TARGET pre-unpack PROPERTY CXX_STANDARD 17)