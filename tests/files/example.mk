APP_NAME := sweetline
BUILD_DIR := build
BIN_DIR := $(BUILD_DIR)/bin
OBJ_DIR := $(BUILD_DIR)/obj
GEN_DIR := $(BUILD_DIR)/generated
SRC_DIR := src
INCLUDE_DIR := include
DOCS_URL := https://example.com/make

CXX ?= c++
AR ?= ar
RM ?= rm -rf
MKDIR_P ?= mkdir -p

CONFIG ?= debug
PLATFORM := $(shell uname -s | tr A-Z a-z)
VERSION := 1.4.2
GIT_SHA := $(shell git rev-parse --short HEAD 2>/dev/null || echo dev)

COMMON_FLAGS := -I$(INCLUDE_DIR) -I$(GEN_DIR) -DSWEETLINE_DOCS=\"$(DOCS_URL)\"
WARN_FLAGS := -Wall -Wextra -Wconversion -Wshadow
DEBUG_FLAGS := -O0 -g3 -DSWEETLINE_DEBUG=1
RELEASE_FLAGS := -O2 -DNDEBUG
LDLIBS +=

ifeq ($(CONFIG),debug)
  OPT_FLAGS := $(DEBUG_FLAGS)
else
  OPT_FLAGS := $(RELEASE_FLAGS)
endif

CPPFLAGS += $(COMMON_FLAGS)
CXXFLAGS += -std=c++20 $(WARN_FLAGS) $(OPT_FLAGS)

SOURCES := \
	$(SRC_DIR)/main.cpp \
	$(SRC_DIR)/highlight.cpp \
	$(SRC_DIR)/parser.cpp \
	$(SRC_DIR)/syntax.cpp \
	$(SRC_DIR)/url.cpp

OBJECTS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SOURCES))
DEPFILES := $(OBJECTS:.o=.d)
TOOLS := dump_tokens render_preview syntax_probe
TOOL_BINS := $(addprefix $(BIN_DIR)/,$(TOOLS))

define newline


endef

define banner
	@printf '%s\n' "$(1)"
endef

define compile_template
$(OBJ_DIR)/$(1).o: $(SRC_DIR)/$(1).cpp | $(OBJ_DIR) $(GEN_DIR)
	$$(call banner,[CXX] $$<)
	$$(CXX) $$(CPPFLAGS) $$(CXXFLAGS) -MMD -MP -c $$< -o $$@
endef

$(foreach unit,main highlight parser syntax url,$(eval $(call compile_template,$(unit))))

.PHONY: all clean distclean run test lint print-config docs package install help

all: $(BIN_DIR)/$(APP_NAME) $(BIN_DIR)/lib$(APP_NAME).a $(TOOL_BINS)

$(BIN_DIR) $(OBJ_DIR) $(GEN_DIR):
	@$(MKDIR_P) $@

$(GEN_DIR)/build_config.hpp: Makefile | $(GEN_DIR)
	@printf '%s\n' \
		'#pragma once' \
		'#define SWEETLINE_VERSION "$(VERSION)"' \
		'#define SWEETLINE_GIT_SHA "$(GIT_SHA)"' \
		'#define SWEETLINE_DOCS_URL "$(DOCS_URL)"' > $@

$(BIN_DIR)/$(APP_NAME): $(OBJECTS) $(GEN_DIR)/build_config.hpp | $(BIN_DIR)
	$(call banner,[LINK] $@)
	$(CXX) $(OBJECTS) $(LDLIBS) -o $@

$(BIN_DIR)/lib$(APP_NAME).a: $(filter-out $(OBJ_DIR)/main.o,$(OBJECTS)) | $(BIN_DIR)
	$(call banner,[AR] $@)
	$(AR) rcs $@ $^

$(BIN_DIR)/dump_tokens: tools/dump_tokens.cpp $(BIN_DIR)/lib$(APP_NAME).a | $(BIN_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $< $(BIN_DIR)/lib$(APP_NAME).a -o $@

$(BIN_DIR)/render_preview: tools/render_preview.cpp $(BIN_DIR)/lib$(APP_NAME).a | $(BIN_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $< $(BIN_DIR)/lib$(APP_NAME).a -o $@

$(BIN_DIR)/syntax_probe: tools/syntax_probe.cpp $(BIN_DIR)/lib$(APP_NAME).a | $(BIN_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $< $(BIN_DIR)/lib$(APP_NAME).a -o $@

run: $(BIN_DIR)/$(APP_NAME)
	$(BIN_DIR)/$(APP_NAME) --docs "$(DOCS_URL)" --mode preview

test: $(BIN_DIR)/$(APP_NAME)
	@printf '%s\n' "Running tests with docs at $(DOCS_URL)"
	@./scripts/test.sh --config "$(CONFIG)" --binary "$<"

lint:
	@./scripts/lint.sh $(SOURCES) tools/*.cpp

docs:
	@printf '%s\n' "See $(DOCS_URL)"
	@printf '%s\n' "Generated on $(PLATFORM) for $(VERSION)"

package: all
	@tar -czf $(BUILD_DIR)/$(APP_NAME)-$(VERSION)-$(PLATFORM).tar.gz $(BIN_DIR)

install: all
	@install -d $(DESTDIR)/usr/local/bin
	@install -m 755 $(BIN_DIR)/$(APP_NAME) $(DESTDIR)/usr/local/bin/$(APP_NAME)

print-config:
	@printf '%s\n' \
		'APP_NAME=$(APP_NAME)' \
		'CONFIG=$(CONFIG)' \
		'PLATFORM=$(PLATFORM)' \
		'VERSION=$(VERSION)' \
		'GIT_SHA=$(GIT_SHA)' \
		'DOCS_URL=$(DOCS_URL)'

help:
	@printf '%s\n' \
		'Targets:' \
		'  all test run clean distclean docs package install print-config' \
		'Variables:' \
		'  CONFIG=debug|release' \
		'  DOCS_URL=https://example.com/make'

clean:
	$(RM) $(OBJ_DIR) $(BIN_DIR)

distclean: clean
	$(RM) $(GEN_DIR) $(BUILD_DIR)/$(APP_NAME)-*.tar.gz

-include $(DEPFILES)
