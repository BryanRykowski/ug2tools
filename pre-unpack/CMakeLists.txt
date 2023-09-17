add_executable (ug2-pre-unpack pre-unpack.cpp ../common/pre_header.hpp ../common/subfile_header.hpp)
set_property (TARGET ug2-pre-unpack PROPERTY CXX_STANDARD 17)
install(TARGETS ug2-pre-unpack DESTINATION bin)