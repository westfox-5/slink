main: src/*
	g++ -fopenmp --std=c++17 -O3 src/util.cpp src/matrix.cpp src/slink.cpp src/main.cpp -o slink

clean:
	rm -f *.o