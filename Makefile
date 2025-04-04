# Path to libsndfile
LIBSNDFILE_PATH = C:/users/admin/vcpkg/installed/x64-windows

#Compiler flags
CFLAGS = -Wall -Wextra -I$(LIBSNDFILE_PATH)/include
LDFLAGS = -L$(LIBSNDFILE_PATH)/lib -lsndfile

# Source and object files
SRC = $(wildcard src/*.c)           # All .c files in src directory
OBJ = $(SRC:src/%.c=build/src/%.o)  # Corresponding .o files in build/src

# Test source and object files
TEST_SRC = tests/test_main.c
TEST_OBJ = build/tests/test_main.o

# Output files
EXEC = build/ggsound
TEST_EXEC = build/run_tests

# Default target: build the program
all: $(EXEC)

# Link the program from object files
$(EXEC): $(OBJ)
	gcc $(OBJ) $(LDFLAGS) -o $(EXEC)

# Compile each .c file to a .o file
build/src/%.o: src/%.c
	mkdir -p build/src
	gcc $(CFLAGS) -c $< -o $@

# Target for running tests
test: $(TEST_EXEC)
	./$(TEST_EXEC)

# Link the test program (without src/main.o)
$(TEST_EXEC): $(TEST_OBJ) $(filter-out build/src/main.o, $(OBJ))
	gcc $(CFLAGS) $(TEST_OBJ) $(filter-out build/src/main.o, $(OBJ)) $(LDFLAGS) -o $(TEST_EXEC)

# Compile test_main.c to object file with libsndfile
build/tests/test_main.o: $(TEST_SRC)
	mkdir -p build/tests
	gcc $(CFLAGS) -c $(TEST_SRC) -o $(TEST_OBJ)

# Clean up build files
clean:
	rm -rf build audio/test.wav
