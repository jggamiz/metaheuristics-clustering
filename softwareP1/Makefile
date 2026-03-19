# --- Compiler settings ---
CXX = g++
CXXFLAGS = -Wall -Wextra -O3 -std=c++17 -Iinc -Icommon

# --- Directories ---
SRC_DIR = src
INC_DIR = inc
COM_DIR = common
BIN_DIR = bin

# --- Files ---
SRCS = $(wildcard $(SRC_DIR)/*.cpp) main.cpp
# Genera los nombres de los .o correspondientes
OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(BIN_DIR)/%.o, $(wildcard $(SRC_DIR)/*.cpp)) $(BIN_DIR)/main.o

TARGET = $(BIN_DIR)/par_solver

# --- Rules ---
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^
	@echo "Build complete! Executable at $(TARGET)"

# Compilar los archivos de src/
$(BIN_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compilar el main.cpp de la raíz
$(BIN_DIR)/main.o: main.cpp
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BIN_DIR)/*.o $(TARGET)
	@echo "Cleaned up."