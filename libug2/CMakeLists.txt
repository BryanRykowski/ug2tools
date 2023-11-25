add_library(libug2 STATIC
	crc.hpp
	crc.cpp
	read_word.hpp
	read_word.cpp
	write_word.hpp
	write_word.cpp
)

set_property (TARGET libug2 PROPERTY CXX_STANDARD 17)