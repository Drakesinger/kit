CXX          := g++
CXXFLAGS     := -std=c++14 -Wall -Wextra -Wpedantic -Wno-unused-parameter -fPIC -DKIT_DEBUG
#LDFLAGS      := -ldl -pthread
LIBS         := $(shell pkg-config --libs kit Qt5 Qt5Widgets assimp)
DEPFLAGS     := $(shell pkg-config --cflags kit  Qt5 Qt5Widgets assimp)
OUT_BINARY   := kit-importer
BUILDDIR     := build

SOURCEDIR    := src
INCLUDEDIR   := include

SOURCES      := $(shell find $(SOURCEDIR) -name '*.cpp')
OBJECTS      := $(addprefix $(BUILDDIR)/,$(SOURCES:%.cpp=%.o))

$(OUT_BINARY): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) $(LDFLAGS) $(LIBS) $(OBJECTS) -o bin/$(OUT_BINARY)

$(BUILDDIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -I$(INCLUDEDIR) -c $< -o $@
	
install: $(OUT_BINARY)
	cp bin/$(OUT_BINARY) ../../dist/
	
clean:
	$(shell rm $(OBJECTS) bin/$(OUT_BINARY))
