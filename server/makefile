# Configurações do compilador
CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2 -Iinclude/
LDFLAGS := -pthread
DEPFLAGS = -MMD -MP -MF $(BUILD_DIR)/$*.d  # Flags para geração de dependências

# Diretórios
SRC_DIR := src
BUILD_DIR := build
BIN_DIR := bin

# Arquivos fonte
SOURCES := $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SOURCES))
TARGET := $(BIN_DIR)/server

# Regra padrão
all: $(TARGET)

# Regra de ligação
$(TARGET): $(OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(LDFLAGS) $^ -o $@

# Regra de compilação com tratamento para primeira execução
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

# Criação do diretório build
$(BUILD_DIR):
	@mkdir -p $@

# Inclui os arquivos de dependência (ignora erro se não existirem)
-include $(wildcard $(BUILD_DIR)/*.d)

# Limpeza (agora remove arquivos .d também)
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

# Recompilar tudo
rebuild: clean all

# Executar o servidor
run: $(TARGET)
	./$(TARGET)

.PHONY: all clean rebuild run