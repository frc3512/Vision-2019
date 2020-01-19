# This makefile recursively finds, compiles, and links .c and .cpp files.
#
# The following targets are supported by this makefile. Indentations represent
# dependencies.
#
# all
# clean
#
# Defining VERBOSE before running make (i.e., VERBOSE=1 make) will print all
# commands ran.

EXEC := cvTest

ifeq ($(OS), Windows_NT)
CC := build/Raspbian9-Windows-Toolchain-6.3.0/raspbian9/bin/arm-raspbian9-linux-gnueabihf-gcc
CPP := build/Raspbian9-Windows-Toolchain-6.3.0/raspbian9/bin/arm-raspbian9-linux-gnueabihf-g++
LD := build/Raspbian9-Windows-Toolchain-6.3.0/raspbian9/bin/arm-raspbian9-linux-gnueabihf-g++
else ifeq ($(OS), Linux)
CC := build/Raspbian9-Linux-Toolchain-6.3.0/raspbian9/bin/arm-raspbian9-linux-gnueabihf-gcc
CPP := build/Raspbian9-Linux-Toolchain-6.3.0/raspbian9/bin/arm-raspbian9-linux-gnueabihf-g++
LD := build/Raspbian9-Linux-Toolchain-6.3.0/raspbian9/bin/arm-raspbian9-linux-gnueabihf-g++
else ifeq ($(OS), Darwin)
CC := build/Raspbian9-Mac-Toolchain-6.3.0/raspbian9/bin/arm-raspbian9-linux-gnueabihf-gcc
CPP := build/Raspbian9-Mac-Toolchain-6.3.0/raspbian9/bin/arm-raspbian9-linux-gnueabihf-g++
LD := build/Raspbian9-Mac-Toolchain-6.3.0/raspbian9/bin/arm-raspbian9-linux-gnueabihf-g++
endif

CFLAGS := -O3 -Wall -s -std=c11 -flto
CPPFLAGS := -O3 -Wall -s -std=c++14 -flto

# Specify defines with -D directives here
DEFINES :=


# Specify include paths with -I directives here
IFLAGS := -Isrc -Isrc/thirdparty

# Specify libs with -l directives here
LDFLAGS := -Lbuild/example-cpp-2019.3.1/cpp-multiCameraServer/lib -lwpilibc \
	-lwpiHal -lcameraserver -lntcore -lcscore -lopencv_calib3d -lopencv_imgproc \
	-lopencv_core -lwpiutil -Wl,--unresolved-symbols=ignore-in-shared-libs

BUILDSYS := make.py Makefile

SRCDIR := src
OBJDIR := build

# Make does not offer a recursive wildcard function, so here's one:
rwildcard=$(wildcard $1$2) $(foreach dir,$(wildcard $1*),$(call rwildcard,$(dir)/,$2))

# Recursively find all C source files
SRC_C := $(call rwildcard,$(SRCDIR)/,*.c)

# Recursively find all C++ source files
SRC_CPP := $(call rwildcard,$(SRCDIR)/,*.cpp)

# Create raw list of object files
OBJ_C := $(SRC_C:.c=.o)
OBJ_CPP := $(SRC_CPP:.cpp=.o)

# Create list of object files for build
OBJ_C := $(addprefix $(OBJDIR)/,$(OBJ_C))
OBJ_CPP := $(addprefix $(OBJDIR)/,$(OBJ_CPP))

.PHONY: build
build: $(OBJDIR)/$(EXEC)

-include $(OBJ_C:.o=.d) $(OBJ_CPP:.o=.d)

$(OBJDIR)/$(EXEC): $(OBJ_C) $(OBJ_CPP)
	@mkdir -p $(@D)
	@echo [LD] $@
ifdef VERBOSE
	$(LD) -o $@ $+ $(LDFLAGS)
else
	@$(LD) -o $@ $+ $(LDFLAGS)
endif

# Pattern rule for building object file from C source
# The -MMD flag generates .d files to track changes in header files included in
# the source.
$(OBJDIR)/%.o: %.c $(BUILDSYS)
	@mkdir -p $(@D)
	@echo [C] $@
ifdef VERBOSE
	$(CC) $(CFLAGS) $(DEFINES) $(IFLAGS) -MMD -c -o $@ $<
else
	@$(CC) $(CFLAGS) $(DEFINES) $(IFLAGS) -MMD -c -o $@ $<
endif

# Pattern rule for building object file from C++ source
# The -MMD flag generates .d files to track changes in header files included in
# the source.
$(OBJDIR)/%.o: %.cpp $(BUILDSYS)
	@mkdir -p $(@D)
	@echo [CPP] $@
ifdef VERBOSE
	$(CPP) $(CPPFLAGS) $(DEFINES) $(IFLAGS) -MMD -c -o $@ $<
else
	@$(CPP) $(CPPFLAGS) $(DEFINES) $(IFLAGS) -MMD -c -o $@ $<
endif

# Cleans the build directory
.PHONY: clean
clean:
	@echo Removing object files
ifdef VERBOSE
	-$(RM) -r $(OBJDIR)/$(SRCDIR)
	-$(RM) $(OBJDIR)/$(EXEC)
else
	-@$(RM) -r $(OBJDIR)/$(SRCDIR)
	-@$(RM) $(OBJDIR)/$(EXEC)
endif
