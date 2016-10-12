DEBUG        ?= 0
PREFIX       := /usr
CXX          := g++
CXXFLAGS     := -std=c++14 -Wall -Wextra -Wpedantic -Wno-unused-parameter -fPIC -DKIT_SHITTY_INTEL
LDFLAGS      := -ldl -pthread -shared 
REQLIBS      := glfw3 freetype2 chaiscript gl
LIBS         := $(shell pkg-config --libs $(REQLIBS))
DEPFLAGS     := $(shell pkg-config --cflags $(REQLIBS))
BUILDDIR     := build
OUT_LIBRARY  := libkit.so
SOURCEDIR    := src
INCLUDEDIR   := include

SOURCES      := $(shell find $(SOURCEDIR) -name '*.cpp')
OBJECTS      := $(addprefix $(BUILDDIR)/,$(SOURCES:%.cpp=%.o))

PCFILE       := pkgconfig/kit.pc

ifeq ($(DEBUG), 1)
	CXXFLAGS += -DKIT_DEBUG -g
else
	CXXFLAGS += -O3
endif

all: $(OUT_LIBRARY)

$(OUT_LIBRARY): $(OBJECTS) $(PCFILE)
	$(shell mkdir lib)
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) $(LDFLAGS) $(LIBS) $(OBJECTS) -o lib/$(OUT_LIBRARY)

$(BUILDDIR)/%.o: %.cpp
	@echo 'Building ${notdir $@} ...'
	$(shell mkdir -p  "${dir $@}")
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -I$(INCLUDEDIR) -c $< -o $@

$(PCFILE):
	$(shell mkdir pkgconfig)
	echo 'prefix=$(PREFIX)' > $(PCFILE)
	echo 'exec_prefix=$${prefix}' >> $(PCFILE)
	echo 'libdir=$${exec_prefix}/lib' >> $(PCFILE)
	echo 'includedir=$${prefix}/include' >> $(PCFILE)
	echo '' >> $(PCFILE)
	echo 'Name: kit' >> $(PCFILE)
	echo 'Description: The Kit framework' >> $(PCFILE)
	echo 'Version: 0.3.7'  >> $(PCFILE)
	echo 'Requires: $(REQLIBS)' >> $(PCFILE)
	echo 'Libs: -L$${libdir} -lkit' >> $(PCFILE)
	echo 'Cflags: -I$${includedir}' >> $(PCFILE)
	
install: all
	cp -r $(INCLUDEDIR)/* $(PREFIX)/include
	cp lib/$(OUT_LIBRARY) $(PREFIX)/lib/
	cp $(PCFILE) $(PREFIX)/lib/pkgconfig/kit.pc
	mkdir -p /usr/share/kit
	cp -r ./dist/static /usr/share/kit/
	
uninstall:
	rm -r $(PREFIX)/include/Kit
	rm $(PREFIX)/lib/$(OUT_LIBRARY)
	rm $(PREFIX)/lib/pkgconfig/kit.pc
	rm -r /opt/kit/
	
clean:
	$(shell rm -rf ./build)
	$(shell rm -rf ./lib)
	$(shell rm -rf ./pkgconfig)
	$(shell rm -f $(OBJECTS) lib/$(OUT_LIBRARY) $(PCFILE))
