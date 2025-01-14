CXX = g++
CXXFLAGS = -std=c++17 -Wall

# Directories
MONTADORDIR = MONTADOR
LIGADORDIR = LIGADOR

# Source files
MONTADOR_SRC = $(MONTADORDIR)/montador.cpp
LIGADOR_SRC = $(LIGADORDIR)/ligador.cpp

# Executables
MONTADOR_EXEC = $(MONTADORDIR)/Montador
LIGADOR_EXEC = $(LIGADORDIR)/Ligador

run: $(MONTADOR_EXEC) $(LIGADOR_EXEC)
	@echo "Built Montador and Ligador executables."

$(MONTADOR_EXEC): $(MONTADOR_SRC)
	$(CXX) $(CXXFLAGS) $< -o $@

$(LIGADOR_EXEC): $(LIGADOR_SRC)
	$(CXX) $(CXXFLAGS) $< -o $@


clean:
	rm -f $(MONTADOR_EXEC) $(LIGADOR_EXEC)
	@echo "Cleaned up executables."
