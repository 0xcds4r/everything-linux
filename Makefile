TARGET = builds/everything

CXX = g++
CXXFLAGS = -Wall -w -O3 -std=c++17 -lstdc++fs -g -I.

LIBS = -lGL -lGLEW -lglfw -ldl -lpthread

SRC_DIRS += $(shell find . -name "*.cpp")

OBJS = $(patsubst %.cpp,objs/%.o,$(SRC_DIRS))

$(TARGET): $(OBJS)
	$(CXX) -o $@ $(OBJS) $(LIBS)

objs/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: clean run
