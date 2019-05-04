CXX  = g++ -std=c++11
DIR  = /usr/local
LIBS = -lgecodedriver    -lgecodesearch  \
       -lgecodeminimodel -lgecodeint     \
       -lgecodekernel    -lgecodesupport

p: Nor.cpp
	$(CXX) -I$(DIR)/include -c Nor.cpp
	$(CXX) -L$(DIR)/lib -o Nor Nor.o $(LIBS)
