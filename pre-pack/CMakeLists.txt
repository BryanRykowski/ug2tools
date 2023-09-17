add_executable (ug2-pre-pack pre-pack.cpp ../common/pre_header.hpp ../common/subfile_header.hpp ../common/crc.hpp ../common/crc.cpp)
set_property (TARGET ug2-pre-pack PROPERTY CXX_STANDARD 17)
install(TARGETS ug2-pre-pack DESTINATION bin)