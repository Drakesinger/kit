CXX          := g++
CXXFLAGS     := -std=c++14 -Wall -Wextra -Wpedantic -Wno-unused-parameter -DKIT_DEBUG -g
#LDFLAGS      := -ldl -pthread
LIBS         := $(shell pkg-config --libs kit)
DEPFLAGS     := $(shell pkg-config --cflags kit)

all: window renderer dynamic-materials

window: 
	@echo 'Building Window example ...'
	$(shell mkdir bin)
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) $(LDFLAGS)  src/window.cpp -o bin/window-example $(LIBS)
	cp bin/window-example ../dist/

renderer:
	@echo 'Building Renderer example ...'
	$(shell mkdir bin)
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) $(LDFLAGS)  src/renderer.cpp -o bin/renderer-example $(LIBS)
	cp bin/renderer-example ../dist/
	
dynamic-materials:
	@echo 'Building Dynamic Materials example ...'
	$(shell mkdir bin)
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) $(LDFLAGS)  src/dynamic-materials.cpp -o bin/dynamic-materials-example $(LIBS)
	cp bin/dynamic-materials-example ../dist/

clean:
	$(shell rm -rf ./bin)
