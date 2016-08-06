CXX          := g++
CXXFLAGS     := -std=c++14 -Wall -Wextra -Wpedantic -Wno-unused-parameter -DKIT_DEBUG -g
#LDFLAGS      := -ldl -pthread
LIBS         := $(shell pkg-config --libs kit)
DEPFLAGS     := $(shell pkg-config --cflags kit)

all: window renderer

window: 
	@echo 'Building Window example ...'
	$(shell mkdir bin)
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) $(LDFLAGS) $(LIBS) src/window.cpp -o bin/window-example
	cp bin/window-example ../dist/

renderer:
	@echo 'Building Renderer example ...'
	$(shell mkdir bin)
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) $(LDFLAGS) $(LIBS) src/renderer.cpp -o bin/renderer-example
	cp bin/renderer-example ../dist/

clean:
	$(shell rm -rf ./bin)
