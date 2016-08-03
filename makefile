PREFIX       := /usr
CXX          := g++
CXXFLAGS     := -std=c++14 -Wall -Wextra -Wpedantic -Wno-unused-parameter -fPIC
LDFLAGS      := -ldl -pthread -shared 
REQLIBS      := glfw3 freetype2 chaiscript gl
LIBS         := $(shell pkg-config --libs $(REQLIBS))
DEPFLAGS     := $(shell pkg-config --cflags $(REQLIBS))
OUT_LIBRARY  := libkit.so
BUILDDIR     := build

SOURCEDIR    := src
INCLUDEDIR   := include

SOURCES      := $(shell find $(SOURCEDIR) -name '*.cpp')
OBJECTS      := $(addprefix $(BUILDDIR)/,$(SOURCES:%.cpp=%.o))

PCFILE       := pkgconfig/kit.pc

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
	echo 'libdir=$${exec_prefix}/lib64' >> $(PCFILE)
	echo 'includedir=$${prefix}/include' >> $(PCFILE)
	echo '' >> $(PCFILE)
	echo 'Name: kit' >> $(PCFILE)
	echo 'Description: The Kit framework' >> $(PCFILE)
	echo 'Version: 0.3.7'  >> $(PCFILE)
	echo 'Requires: $(REQLIBS)' >> $(PCFILE)
	echo 'Libs: -L$${libdir} -lkit' >> $(PCFILE)
	echo 'Cflags: -I$${includedir}' >> $(PCFILE)
	
install: $(OUT_LIBRARY) $(PCFILE)
	cp -r $(INCLUDEDIR)/* $(PREFIX)/include
	cp lib/$(OUT_LIBRARY) $(PREFIX)/lib64/
	cp $(PCFILE) $(PREFIX)/lib64/pkgconfig/kit.pc
	
clean:
	$(shell rm -rf ./build)
	$(shell rm -rf ./lib)
	$(shell rm -rf ./pkgconfig)
	$(shell rm -f $(OBJECTS) lib/$(OUT_LIBRARY) $(PCFILE))
