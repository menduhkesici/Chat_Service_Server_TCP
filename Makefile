###########################################################

## C/C++ PROJECT FILE STRUCTURE ##

#|--> Project/
#	|--> Makefile
#	|--> main.cpp
#	|--> src/		(source files)
#		|--> file1.cpp
#		|--> file2.cpp
#		|--> file1.c
#		|--> file2.c
#	|--> include/	(header files)
#		|--> file1.h
#		|--> file2.h
#		|--> file1.hpp
#		|--> file2.hpp
#	|--> bin/		(object files and dependency files)
#	|--> Project	(executable output)

# Source file directory:
SRC_DIR = src

# Include header file diretory:
INC_DIR = include

# Object file directory:
OBJ_DIR = bin

##########################################################

## COMPILE OPTIONS ##

# Linker options:
LD= g++
LD_FLAGS= -std=c++17 -O3
LD_LIBS= 

# C++ compiler options:
CXX= g++
CXX_FLAGS= -MMD -std=c++17 -O3
CXX_LIBS=

# C compiler options:
CC= gcc
CC_FLAGS= -MMD -O3
CC_LIBS=

##########################################################

## MAKE VARIABLES ##

# Target executable name:
EXE := $(shell basename $(CURDIR))

# Object files:
OBJS := $(OBJ_DIR)/main.cpp.o\
 $(patsubst $(SRC_DIR)/%, $(OBJ_DIR)/%.o, $(shell find $(SRC_DIR) -name '*.cpp' -or -name '*.c'))

# Dependencies:
DEPS := $(OBJS:%.o=%.d)

##########################################################

## COMPILE ##

# Link C/C++ compiled object files to target executable
$(EXE) : $(OBJS)
	$(LD) $(LD_FLAGS) $(OBJS) -o $@ $(LD_LIBS)
	@echo "-- Build complete --"

# Include dependency files
-include $(DEPS)

# Compile main.cpp file to object file
$(OBJ_DIR)/main.cpp.o : main.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXX_FLAGS) -c $< -o $@ $(CXX_LIBS)

# Compile C++ source files to object files
$(OBJ_DIR)/%.cpp.o : $(SRC_DIR)/%.cpp 
	@mkdir -p $(@D)
	$(CXX) $(CXX_FLAGS) -c $< -o $@ $(CXX_LIBS)

# Compile C source files to object files
$(OBJ_DIR)/%.c.o : $(SRC_DIR)/%.c 
	@mkdir -p $(@D)
	$(CC) $(CC_FLAGS) -c $< -o $@ $(CC_LIBS)

# Run executable after build
.PHONY: run
run: $(EXE)
	@echo "-- Program is started --"
	@./$(EXE)
	@echo "-- Program terminated successfully --"

# Clean objects in object directory
.PHONY: clean
clean:
	rm -f $(OBJ_DIR)/*.o $(OBJ_DIR)/*.d $(EXE)

###########################################################
