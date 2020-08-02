CXX = g++
CXXFLAGS = -Wall -g

main: main.o
	$(CXX) $(CXXFLAGS) -o main main.o

main.o: main.cpp
