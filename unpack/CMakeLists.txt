add_compile_definitions(APP_VERSION="1.1.0")
add_executable (ug2-unpack
	unpack.hpp
	main.cpp
	unpack.cpp
	read_pre.cpp
	buffers.cpp
	write_files.cpp
	inflate_file.cpp
	write_spec.cpp
	path_get_file_name.cpp
	path_to_string.cpp
	../libug2/read_word.hpp
)
set_property (TARGET ug2-unpack PROPERTY CXX_STANDARD 17)
target_link_libraries(ug2-unpack libug2)
install(TARGETS ug2-unpack DESTINATION bin)
