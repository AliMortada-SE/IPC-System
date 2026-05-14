all:  g++

g++:
	g++ -std=c++17 -Iinclude src/*.cpp -o build/pipe -lpthread