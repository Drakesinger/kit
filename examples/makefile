DEBUG        ?= 0
CXX          := g++
CXXFLAGS     := -std=c++14 -Wall -Wextra -Wpedantic -Wno-unused-parameter
LIBS         := $(shell pkg-config --libs kit)
DEPFLAGS     := $(shell pkg-config --cflags kit)


ifeq ($(DEBUG), 1)
	CXXFLAGS += -DKIT_DEBUG -g
else
	CXXFLAGS += -O2 -g
endif


all: window renderer dynamic-materials

window: 
	@echo 'Building Window example ...'
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) $(LDFLAGS)  src/window.cpp -o window-example $(LIBS)

renderer:
	@echo 'Building Renderer example ...'
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) $(LDFLAGS)  src/renderer.cpp -o renderer-example $(LIBS)
	
dynamic-materials:
	@echo 'Building Dynamic Materials example ...'
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) $(LDFLAGS)  src/dynamic-materials.cpp -o dynamic-materials-example $(LIBS)
	
clean:
	$(shell rm window-example renderer-example dynamic-materials-example)
