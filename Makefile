CXX = g++

COMPILER_SRC_PATH = Compiler/Src
VM_SRC_PATH = VM/Src
BUILD_PATH = Build
BIN_NAME = TinySrcipt

COMPILER_SOURCES = $(shell find $(COMPILER_SRC_PATH) -name '*.cpp' | sort -k 1nr | cut -f2-)
COMPILER_OBJECTS = $(COMPILER_SOURCES:$(COMPILER_SRC_PATH)/%.cpp=$(BUILD_PATH)/%.o)
VM_SOURCES = $(shell find $(VM_SRC_PATH) -name '*.c' | sort -k 1nr | cut -f2-)
VM_OBJECTS = $(VM_SOURCES:$(VM_SRC_PATH)/%.c=$(BUILD_PATH)/%.o)
SOURCES = $(COMPILER_SOURCES) $(VM_SOURCES) Main.cpp
OBJECTS = $(COMPILER_OBJECTS) $(VM_OBJECTS) $(BUILD_PATH)/Main.o
DEPS = $(OBJECTS:.o=.d)

COMPILE_FLAGS = -std=c++11
INCLUDES = -I Compiler/Include -I VM/Include -I /usr/local/include
LIBS = 

.PHONY: default_target
default_target: release

.PHONY: release
release: export CXXFLAGS := $(CXXFLAGS) $(COMPILE_FLAGS)
release: dirs
	@$(MAKE) all

.PHONY: dirs
dirs:
	@mkdir -p $(dir $(OBJECTS))

.PHONY: clean
clean:
	@$(RM) $(BIN_NAME)
	@$(RM) -r $(BUILD_PATH)

.PHONY: all
all: $(BIN_NAME)

$(BIN_NAME): $(OBJECTS)
	$(CXX) $(OBJECTS) $(LIBS) -o $@

-include $(DEPS)

$(BUILD_PATH)/%.o: $(COMPILER_SRC_PATH)/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -MP -MMD -c $< -o $@

$(BUILD_PATH)/%.o: $(VM_SRC_PATH)/%.c
	$(CXX) $(CXXFLAGS) $(INCLUDES) -MP -MMD -c $< -o $@

$(BUILD_PATH)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -MP -MMD -c $< -o $@

