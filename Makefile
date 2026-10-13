UI       ?= tui
CXX      ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra

# выбор UI: файл реализации и библиотеки
ifeq ($(UI),tui)
  LDLIBS := -lncurses
else ifeq ($(UI),gl)
  LDLIBS := -lglfw -lGL
else
  $(error Неизвестный UI '$(UI)'. Допустимо: tui, gl)
endif

SRC    := main.cpp $(UI)_front.cpp
BUILD  := build/$(UI)
OBJ    := $(SRC:%.cpp=$(BUILD)/%.o)
TARGET := app_$(UI)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $@ $(LDLIBS)

$(BUILD)/%.o: %.cpp
	@mkdir -p $(BUILD)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

-include $(OBJ:.o=.d)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf build app_tui app_gl

.PHONY: all run clean
