main:
	g++ -fopenmp --std=c++17 -O3 src/*.cpp src/slink_executors/*.cpp -o slink

clean:
	rm -f *.o