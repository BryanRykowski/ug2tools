add_executable (ug2-tex2dds tex2dds.cpp ../common/read_word.hpp ../common/read_word.cpp ../common/write_word.hpp ../common/write_word.cpp ../common/dds_header.hpp ../common/tex_header.hpp)
set_property (TARGET ug2-tex2dds PROPERTY CXX_STANDARD 17)
install(TARGETS ug2-tex2dds DESTINATION bin)