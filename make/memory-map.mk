# Memory Map Generation Makefile Include
# Usage: include memory-map.mk in your main Makefile
#
# Provides targets for generating memory maps from vasm listing files.
# Requires: vasm with -L option for listing file generation
#
# Configurable variables (set in parent Makefile before include):
#   BUILD_DIR        - Build output directory (default: build)
#   MEMORY_MAP       - Output memory map file (default: $(BUILD_DIR)/MEMORY.MAP)
#   LISTING_FILES    - Space-separated list of .lst files to extract symbols from
#   MAX_SYMBOLS_PER_FILE - Max symbols per file (default: 50, 0=all)
#
# For project-specific layout info, define your own memory-map target that
# writes custom content first, then calls memory-map-symbols to append symbols.

# Defaults
BUILD_DIR ?= build
MEMORY_MAP ?= $(BUILD_DIR)/MEMORY.MAP
LISTING_FILES ?=

# Symbol extraction pattern - matches vasm listing format
# Pattern: SYMBOL_NAME                    A:XXXX
SYMBOL_PATTERN := ^[A-Z][A-Z0-9._]*[[:space:]]+A:

# Number of symbols to extract per listing file (0 = all)
MAX_SYMBOLS_PER_FILE ?= 50

# Extract and append symbols from listing files to MEMORY_MAP
.PHONY: memory-map-symbols
memory-map-symbols:
	@echo "=== Symbols ===" >> $(MEMORY_MAP)
	@for lst in $(LISTING_FILES); do \
		if [ -f "$$lst" ]; then \
			echo "--- From $$lst ---" >> $(MEMORY_MAP); \
			if [ $(MAX_SYMBOLS_PER_FILE) -eq 0 ]; then \
				grep -E '$(SYMBOL_PATTERN)' "$$lst" >> $(MEMORY_MAP) 2>/dev/null || echo "(no symbols)" >> $(MEMORY_MAP); \
			else \
				grep -E '$(SYMBOL_PATTERN)' "$$lst" 2>/dev/null | head -$(MAX_SYMBOLS_PER_FILE) >> $(MEMORY_MAP) || echo "(no symbols)" >> $(MEMORY_MAP); \
			fi; \
			echo "" >> $(MEMORY_MAP); \
		else \
			echo "--- $$lst (not found) ---" >> $(MEMORY_MAP); \
			echo "" >> $(MEMORY_MAP); \
		fi; \
	done
	@echo "Memory map saved to: $(MEMORY_MAP)"

# Default memory map target (creates the actual file)
# Override this in your project Makefile to add custom layout info
$(MEMORY_MAP): $(LISTING_FILES) | $(BUILD_DIR)
	@echo "=== Memory Map ===" > $@
	@echo "Generated: $$(date)" >> $@
	@echo "" >> $@
	@$(MAKE) --no-print-directory memory-map-symbols

# Convenience alias
.PHONY: memory-map
memory-map: $(MEMORY_MAP)

# Display memory map
.PHONY: show-memory-map
show-memory-map: $(MEMORY_MAP)
	@cat $<

# Clean memory map
.PHONY: memory-map-clean
memory-map-clean:
	rm -f $(MEMORY_MAP)

# Help target
.PHONY: memory-map-help
memory-map-help:
	@echo "Memory Map Targets:"
	@echo "  memory-map         - Generate memory map file (header + symbols)"
	@echo "  memory-map-symbols - Append symbols to existing memory map"
	@echo "  show-memory-map    - Generate and display memory map"
	@echo "  memory-map-clean   - Remove memory map file"
	@echo ""
	@echo "Configuration variables:"
	@echo "  BUILD_DIR=$(BUILD_DIR)"
	@echo "  MEMORY_MAP=$(MEMORY_MAP)"
	@echo "  LISTING_FILES=$(LISTING_FILES)"
	@echo "  MAX_SYMBOLS_PER_FILE=$(MAX_SYMBOLS_PER_FILE)"
	@echo ""
	@echo "For custom layout info, override the memory-map target:"
	@echo "  memory-map: | \$$(BUILD_DIR)"
	@echo "      @echo \"My Header\" > \$$(MEMORY_MAP)"
	@echo "      @echo \"My Layout\" >> \$$(MEMORY_MAP)"
	@echo "      @\$$(MAKE) memory-map-symbols"
