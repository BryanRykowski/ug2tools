add_executable (ug2-dds2tex dds2tex.cpp ../common/dds_header.hpp ../common/tex_header.hpp ../libug2/write_word.hpp ../libug2/read_word.hpp)
set_property (TARGET ug2-dds2tex PROPERTY CXX_STANDARD 17)
target_link_libraries(ug2-dds2tex libug2)
install(TARGETS ug2-dds2tex DESTINATION bin)