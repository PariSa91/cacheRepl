all:		efectiu

efectiu:	cache.cc efectiu.cc replacement_state.cpp replacement_state.h trace.h
		g++  -DCACHE -O3 -Wall -g -o efectiu cache.cc efectiu.cc replacement_state.cpp -lz

clean:
	 	rm -f efectiu
