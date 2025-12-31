# ProDOS Disk Image Creation Makefile Include
# Usage: include prodos.mk in your main Makefile
#
# Provides targets for creating bootable ProDOS disk images with AppleCommander acx.jar
# Requires: Java, AppleCommander acx.jar
#
# Configurable variables (can be overridden in parent Makefile or command line):
#   ACX_JAR          - Path to AppleCommander acx.jar (default: /usr/local/share/java/acx.jar)
#   PRODOS_TEMPLATE  - Path to blank bootable ProDOS disk image (default: prodos/blank140k.dsk)
#   PRODOS_IMAGE     - Output disk image path (default: build/SCASM.dsk)
#
# Example override:
#   make disk PRODOS_TEMPLATE=/path/to/my/blank.dsk

# Paths
ACX_JAR ?= /usr/local/share/java/acx.jar
PRODOS_TEMPLATE ?= prodos/blank140k.po
PRODOS_IMAGE ?= build/SCASM.po

# AppleCommander command wrapper
AC := java -jar $(ACX_JAR)

# ProDOS file types for -t option
FTYPE_BIN := BIN    # Binary file ($06)
FTYPE_SYS := SYS    # System file ($FF)
FTYPE_TXT := TXT    # Text file ($04)
FTYPE_INT := INT    # Integer BASIC
FTYPE_BAS := BAS    # Applesoft BASIC

# Create a new ProDOS disk image from template
.PHONY: prodos-disk
prodos-disk: $(PRODOS_IMAGE)

$(PRODOS_IMAGE): $(PRODOS_TEMPLATE) | build
	@echo "Creating ProDOS disk image: $(PRODOS_IMAGE)"
	cp $(PRODOS_TEMPLATE) $(PRODOS_IMAGE)

# Add a binary file to disk (usage: make prodos-add-bin FILE=build/SCASM NAME=SCASM ADDR=0x2000)
.PHONY: prodos-add-bin
prodos-add-bin:
	@test -n "$(FILE)" || (echo "ERROR: FILE not specified"; exit 1)
	@test -n "$(NAME)" || (echo "ERROR: NAME not specified"; exit 1)
	@test -n "$(ADDR)" || (echo "ERROR: ADDR not specified (e.g. 0x2000)"; exit 1)
	@echo "Adding $(NAME) to $(PRODOS_IMAGE) at address $(ADDR)"
	$(AC) import -d $(PRODOS_IMAGE) --raw --stdin -t $(FTYPE_BIN) -a $(ADDR) -n $(NAME) < $(FILE)

# Add a system file to disk (usage: make prodos-add-sys FILE=build/SCASM NAME=SCASM ADDR=0x2000)
.PHONY: prodos-add-sys
prodos-add-sys:
	@test -n "$(FILE)" || (echo "ERROR: FILE not specified"; exit 1)
	@test -n "$(NAME)" || (echo "ERROR: NAME not specified"; exit 1)
	@test -n "$(ADDR)" || (echo "ERROR: ADDR not specified (e.g. 0x2000)"; exit 1)
	@echo "Adding $(NAME) (SYS) to $(PRODOS_IMAGE) at address $(ADDR)"
	$(AC) import -d $(PRODOS_IMAGE) --raw --stdin -t $(FTYPE_SYS) -a $(ADDR) -n $(NAME) < $(FILE)

# List contents of ProDOS disk
.PHONY: prodos-list
prodos-list:
	@echo "Contents of $(PRODOS_IMAGE):"
	$(AC) list -d $(PRODOS_IMAGE)

# Show detailed disk information
.PHONY: prodos-info
prodos-info:
	@echo "Disk information for $(PRODOS_IMAGE):"
	$(AC) info -d $(PRODOS_IMAGE)

# Show disk usage map
.PHONY: prodos-map
prodos-map:
	@echo "Disk usage map for $(PRODOS_IMAGE):"
	$(AC) map -d $(PRODOS_IMAGE)

# Extract a file from disk (usage: make prodos-get NAME=SCASM OUT=SCASM.extracted)
.PHONY: prodos-get
prodos-get:
	@test -n "$(NAME)" || (echo "ERROR: NAME not specified"; exit 1)
	@test -n "$(OUT)" || (echo "ERROR: OUT not specified"; exit 1)
	@echo "Extracting $(NAME) from $(PRODOS_IMAGE) to $(OUT)"
	$(AC) export -d $(PRODOS_IMAGE) -n $(NAME) --output=$(OUT)

# Delete a file from disk (usage: make prodos-delete NAME=SCASM)
.PHONY: prodos-delete
prodos-delete:
	@test -n "$(NAME)" || (echo "ERROR: NAME not specified"; exit 1)
	@echo "Deleting $(NAME) from $(PRODOS_IMAGE)"
	$(AC) delete -d $(PRODOS_IMAGE) -n $(NAME)

# Clean ProDOS disk image
.PHONY: prodos-clean
prodos-clean:
	rm -f $(PRODOS_IMAGE)

# Help target
.PHONY: prodos-help
prodos-help:
	@echo "ProDOS Disk Image Targets (using AppleCommander acx.jar):"
	@echo "  prodos-disk        - Create blank ProDOS disk image"
	@echo "  prodos-add-bin     - Add binary file (FILE, NAME, ADDR required)"
	@echo "  prodos-add-sys     - Add system file (FILE, NAME, ADDR required)"
	@echo "  prodos-list        - List disk contents"
	@echo "  prodos-info        - Show disk information"
	@echo "  prodos-map         - Show disk usage map"
	@echo "  prodos-get         - Extract file (NAME, OUT required)"
	@echo "  prodos-delete      - Delete file (NAME required)"
	@echo "  prodos-clean       - Remove disk image"
	@echo ""
	@echo "Example: make prodos-disk prodos-add-bin FILE=build/SCASM NAME=SCASM ADDR=0x2000"
