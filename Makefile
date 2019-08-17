CXX        = c++
CXXFLAGS   = -std=c++11 -MMD -MP
OPT        = -O3 -march=native -DNDEBUG 
#OPT       := -O0 -g -ggdb -D_GLIBCXX_DEBUG
LDFLAGS    =
LIBS       =
INCLUDES   =
SRC_DIR    = ./cpp
BLD_DIR    = .
OBJ_DIR    = ./obj
SRCS       = $(SRC_DIR)/main.cpp
OBJS       = $(subst $(SRC_DIR),$(OBJ_DIR), $(SRCS:.cpp=.o))
TARGET     = $(BLD_DIR)/main
PYTARGET   = $(BLD_DIR)/games.so
PYFLAGS    = -shared -fPIC $(shell python3-config --cflags --ldflags)
PYINCLUDES = $(INCLUDE) $(shell python3-config --includes) -I./modules/pybind11/include/

DEPENDS  = $(OBJS:.o=.d)

all: $(TARGET) $(PYTARGET)

$(TARGET): $(OBJS) $(LIBS)
	$(CXX) $(OPT) -o $@ $(OBJS) $(LDFLAGS)

$(PYTARGET): $(OBJ_DIR)/pybind.o $(LIBS)
	$(CXX) $(OPT) -o $@ $(OBJ_DIR)/pybind.o $(PYFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@if [ ! -d $(OBJ_DIR) ]; \
		then echo "mkdir -p $(OBJ_DIR)"; mkdir -p $(OBJ_DIR); \
		fi
	$(CXX) $(CXXFLAGS) $(OPT) $(INCLUDES) -o $@ -c $< 

$(OBJ_DIR)/pybind.o: $(SRC_DIR)/pybind.cpp
	$(CXX) $(CXXFLAGS) $(OPT) $(PYINCLUDES) -o $@ -c $<

clean:
	$(RM) -r $(OBJ_DIR) $(TARGET) $(PYTARGET)

-include $(DEPENDS)

.PHONY: all clean
