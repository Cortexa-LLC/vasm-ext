# Unix, using gcc

CC = gcc
TARGET =
TARGETEXTENSION =

CCOUT = -o $(DUMMY)
CFLAGS = -c -std=c90 -O2 -pedantic -Wno-long-long -Wno-parentheses -Wno-shift-count-overflow -DUNIX $(OUTFMTS)

LD = $(CC)
LDOUT = $(CCOUT)
LDFLAGS = -lm

RM = rm -f

# Installation paths
PREFIX ?= /usr/local
BINDIR = $(PREFIX)/bin
SHAREDIR = $(PREFIX)/share/vasm

include make.rules

# Installation target
.PHONY: install install-bin install-share
install: install-bin install-share

install-bin:
	@echo "Installing vasm binaries to $(BINDIR)..."
	@mkdir -p $(BINDIR)
	@for bin in vasm6502_*; do \
		if [ -f "$$bin" ] && [ -x "$$bin" ]; then \
			echo "  Installing $$bin"; \
			install -m 755 "$$bin" $(BINDIR)/; \
		fi; \
	done
	@echo "✓ Binaries installed"

install-share:
	@echo "Installing vasm make utilities to $(SHAREDIR)/make/..."
	@mkdir -p $(SHAREDIR)/make
	@install -m 644 make/memory-map.mk $(SHAREDIR)/make/
	@install -m 755 make/memory-map.py $(SHAREDIR)/make/
	@install -m 644 make/prodos.mk $(SHAREDIR)/make/
	@install -m 644 make/README.md $(SHAREDIR)/make/
	@echo "✓ Make utilities installed"
	@echo ""
	@echo "Installation complete!"
	@echo "Binaries:       $(BINDIR)/vasm6502_*"
	@echo "Make utilities: $(SHAREDIR)/make/"

.PHONY: uninstall
uninstall:
	@echo "Removing vasm binaries from $(BINDIR)..."
	@rm -f $(BINDIR)/vasm6502_*
	@echo "Removing vasm make utilities from $(SHAREDIR)..."
	@rm -rf $(SHAREDIR)
	@echo "✓ Uninstall complete"
