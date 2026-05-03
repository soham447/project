CXX      = g++
CXXFLAGS = -std=c++17 -O2
TARGET   = typing_analyzer
SRCS     = main.cpp terminal.cpp database.cpp typing.cpp
 
$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET)
 
clean:
	rm -f $(TARGET)
 
