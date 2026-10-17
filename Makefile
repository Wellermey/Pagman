UI       ?= tui
CXX      ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra

# Определение ОС. Можно переопределить: make TARGET_OS=Windows
ifeq ($(OS),Windows_NT)
  TARGET_OS ?= Windows
else
  TARGET_OS ?= $(shell uname -s)
endif

EXE :=
ifeq ($(TARGET_OS),Windows)
  EXE := .exe
endif

# выбор UI и библиотек под платформу
ifeq ($(UI),tui)
  ifeq ($(TARGET_OS),Windows)
    LDLIBS := -lpdcurses
  else ifeq ($(TARGET_OS),Darwin)
    LDLIBS := -lncurses
  else
    LDLIBS := -lncursesw
  endif
else ifeq ($(UI),gl)
  ifeq ($(TARGET_OS),Windows)
    LDLIBS := -lglfw3 -lopengl32 -lgdi32
  else ifeq ($(TARGET_OS),Darwin)
    GLFW_PREFIX := $(shell brew --prefix glfw 2>/dev/null)
    CPPFLAGS += -I$(GLFW_PREFIX)/include -DGL_SILENCE_DEPRECATION
    LDFLAGS  += -L$(GLFW_PREFIX)/lib
    LDLIBS   := -lglfw -framework OpenGL
  else
    LDLIBS := -lglfw -lGL
  endif
else
  $(error Неизвестный UI '$(UI)'. Допустимо: tui, gl)
endif

SRC    := main.cpp $(UI)_front.cpp
BUILD  := build/$(UI)
OBJ    := $(SRC:%.cpp=$(BUILD)/%.o)
TARGET := app_$(UI)$(EXE)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $@ $(LDFLAGS) $(LDLIBS)

$(BUILD)/%.o: %.cpp
	@mkdir -p $(BUILD)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

-include $(OBJ:.o=.d)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf build app_tui app_gl app_tui.exe app_gl.exe

.PHONY: all run clean
