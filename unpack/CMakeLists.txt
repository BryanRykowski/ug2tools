add_executable (ug2-unpack unpack.hpp main.cpp unpack.cpp read_pre.cpp write_files.cpp inflate_file.cpp ../common/pre_header.hpp ../common/subfile_header.hpp ../libug2/read_word.hpp)
set_property (TARGET ug2-unpack PROPERTY CXX_STANDARD 17)
target_link_libraries(ug2-unpack libug2)
install(TARGETS ug2-unpack DESTINATION bin)
