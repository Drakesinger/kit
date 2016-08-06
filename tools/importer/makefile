CXX          := g++
CXXFLAGS     := -std=c++14 -Wall -Wextra -Wpedantic -Wno-unused-parameter -fPIC -DKIT_DEBUG
#LDFLAGS      := -ldl -pthread
LIBS         := $(shell pkg-config --libs kit Qt5Core Qt5Widgets assimp)
DEPFLAGS     := $(shell pkg-config --cflags kit  Qt5Core Qt5Widgets assimp)
OUT_BINARY   := kit-importer
BUILDDIR     := build

SOURCEDIR    := src
INCLUDEDIR   := include

SOURCES      := $(shell find $(SOURCEDIR) -name '*.cpp')
OBJECTS      := $(addprefix $(BUILDDIR)/,$(SOURCES:%.cpp=%.o))

$(OUT_BINARY): $(OBJECTS)
	$(shell mkdir bin)
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) $(LDFLAGS) $(OBJECTS) -o bin/$(OUT_BINARY) $(LIBS)

$(BUILDDIR)/%.o: %.cpp
	@echo 'Building ${notdir $@} ...'
	$(shell mkdir -p  "${dir $@}")
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -I$(INCLUDEDIR) -c $< -o $@
	
install: $(OUT_BINARY)
	cp bin/$(OUT_BINARY) ../../dist/
	
clean:
	$(shell rm -rf ./build)
	$(shell rm -rf ./bin)
