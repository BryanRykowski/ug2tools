add_executable (ug2-pre-unpack pre-unpack.cpp ../common/pre_header.hpp ../common/subfile_header.hpp ../libug2/read_word.hpp)
set_property (TARGET ug2-pre-unpack PROPERTY CXX_STANDARD 17)
target_link_libraries(ug2-pre-unpack libug2)
install(TARGETS ug2-pre-unpack DESTINATION bin)