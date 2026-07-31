# ==================================================
# Variables
# ==================================================

SCRIPTS_PATH = scripts
PACKAGE_SCRIPT = $(SCRIPTS_PATH)/package.py
NEW_PACKAGE_NAME ?= new-package

# ==================================================
# Exported variables
# ==================================================

# ==================================================
# Default phony rules
# ==================================================

.DEFAULT_GOAL := all

.PHONY: all
all:
	@echo "Available targets:"
	@echo "  package - Create a new package using the package script"
	@echo "  sync    - Sync all packages using uv sync"

# ==================================================
# mdp phony rules
# ==================================================

.PHONY: package
package:
	python3 $(PACKAGE_SCRIPT) $(NEW_PACKAGE_NAME)
