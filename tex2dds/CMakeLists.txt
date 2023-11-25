add_executable (ug2-tex2dds tex2dds.cpp ../libug2/read_word.hpp ../libug2/write_word.hpp ../common/dds_header.hpp ../common/tex_header.hpp)
set_property (TARGET ug2-tex2dds PROPERTY CXX_STANDARD 17)
target_link_libraries(ug2-tex2dds libug2)
install(TARGETS ug2-tex2dds DESTINATION bin)