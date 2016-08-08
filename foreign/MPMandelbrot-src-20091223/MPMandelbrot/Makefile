# OpenCL multiprecision Mandelbrot Set
# EB Dec 2009

# archive data
CURRENT_DATE := $(shell date +%Y%m%d_%Hh%M)
CURRENT_DIR := $(notdir $(shell /bin/pwd))
CURRENT_MACHINE := $(shell uname -m)

# output directories
OBJDIR := objs
DEPDIR := deps
MOCDIR := moc
BINDIR := bin
ifeq ($(CURRENT_MACHINE),x86_64)
LIBDIR := lib-64
else
LIBDIR := lib-32
endif

# targets
MOCFILES = ViewWidget AppWindow EngineThread MandelbrotNativeThread HelpDialog
MOCSRC = $(patsubst %,$(MOCDIR)/moc_%.cpp, $(MOCFILES))
OPENCL_CODE = $(patsubst src/%,$(BINDIR)/%,$(wildcard src/*.cl) )
OBJS = $(patsubst %,$(OBJDIR)/%.o, $(MOCFILES) Mandelbrot MandelbrotNative MandelbrotOpenCL \
        UnitTests appMain SpecialSites ) \
	$(patsubst %,$(OBJDIR)/moc_%.o, $(MOCFILES) )
DEPS = $(patsubst $(OBJDIR)/%.o, $(DEPDIR)/%.d, $(OBJS))

# OpenCL SDK directory
# OPENCL_DIR = /opt/cuda/include
OPENCL_DIR = /opt/ati-stream-sdk-v2.0-beta4-lnx64
# Bealto OpenCL library path
BEALTO_OPENCL_DIR = ../BealtoOpenCL
BEALTO_OPENCL_LIB = $(BEALTO_OPENCL_DIR)/$(LIBDIR)/libBealtoOpenCL.a

# flags
CXXFLAGS = -DLinux -O2 -mtune=nocona -msse3 -Wall -I/usr/include/qt4 -I$(OPENCL_DIR)/include -I$(BEALTO_OPENCL_DIR)/include
LDFLAGS = -L/usr/lib/qt4 -L$(OPENCL_DIR)/lib/x86_64

VPATH = src:$(MOCDIR)

all: mpmt

# Command line
mpmt: $(BINDIR)/mpmt $(OPENCL_CODE)
# Qt interface
mpm: $(BINDIR)/mpm $(OPENCL_CODE)

lib: $(BEALTO_OPENCL_LIB)

$(BINDIR)/mpm: $(MOCSRC) $(OBJS) $(BEALTO_OPENCL_LIB)
	@[ -d $(BINDIR) ] || mkdir -p $(BINDIR)
	g++ $(LDFLAGS) -o $@ $(OBJS) $(BEALTO_OPENCL_LIB) -lpthread -lOpenCL -lQtCore -lQtGui 

$(BINDIR)/mpmt: $(patsubst %,$(OBJDIR)/%.o, Mandelbrot MandelbrotOpenCL SpecialSites textMain )
	@[ -d $(BINDIR) ] || mkdir -p $(BINDIR)
	g++ $(LDFLAGS) -o $@ $^ $(BEALTO_OPENCL_LIB) -lOpenCL -lpng

$(BINDIR)/%.cl: src/%.cl
	@[ -d $(BINDIR) ] || mkdir -p $(BINDIR)
	@install -pv -t $(BINDIR) $^

$(BEALTO_OPENCL_LIB):
	make -C $(BEALTO_OPENCL_DIR) lib

dos2unix:
	dos2unix src/*.cpp src/*.h src/*.cl Makefile

clean:
	/bin/rm -f src/*~ *~ *float.png *double.png *fp128.png
	/bin/rm -rf vs2008/Release-* vs2008/Debug-* $(OBJDIR) $(DEPDIR) $(BINDIR) $(MOCDIR)

archive: clean
	@echo "ARCHIVE $(CURRENT_DATE)"
	tar czf "../MPMandelbrot-$(CURRENT_DATE).tar.gz" -C.. --exclude=".svn" $(CURRENT_DIR)

##### Dependencies
$(DEPDIR)/%.d: %.cpp
	@[ -d $(DEPDIR) ] || mkdir -p $(DEPDIR)
	@/bin/echo -e "DEPS \033[32m$*\033[0m"
	@$(CXX) $(CXXFLAGS) -o $@ -MM -MT '$(OBJDIR)/$*.o $@' $<

##### Compilation
$(OBJDIR)/%.o: %.cpp
	@[ -d $(OBJDIR) ] || mkdir -p $(OBJDIR)
	@/bin/echo -e "C++  \033[34m$*\033[0m"
	@$(CXX) $(CXXFLAGS) -c -o $@ $<

##### Qt
$(MOCDIR)/moc_%.cpp: %.h
	@[ -d $(MOCDIR) ] || mkdir -p $(MOCDIR)
	@/bin/echo -e "MOC  \033[33m$*\033[0m"
	@moc -o $@ $<

-include $(DEPS)
