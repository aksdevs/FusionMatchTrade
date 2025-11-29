SHELL := /bin/bash
CMAKE  := cmake
CTEST  := ctest
BUILD_DIR := build

.PHONY: all tradebook matchengine forex test clean help

all: tradebook matchengine forex

# New service-based build targets (preferred)
.PHONY: service-tradebook service-matchengine service-forex services

services: service-tradebook service-matchengine service-forex

service-tradebook:
	mkdir -p $(BUILD_DIR)/services/tradebook
	cd $(BUILD_DIR)/services/tradebook && $(CMAKE) ../../../src/services/tradebook && $(CMAKE) --build . -- -j$(shell nproc || 2)

service-matchengine:
	mkdir -p $(BUILD_DIR)/services/matchengine
	cd $(BUILD_DIR)/services/matchengine && $(CMAKE) ../../../src/services/matchengine && $(CMAKE) --build . -- -j$(shell nproc || 2)

service-forex:
	mkdir -p $(BUILD_DIR)/services/forex
	cd $(BUILD_DIR)/services/forex && $(CMAKE) ../../../src/services/forex && $(CMAKE) --build . -- -j$(shell nproc || 2)


help:
	@echo "Usage: make [all|tradebook|matchengine|forex|test|clean]"
	@echo "  all        Builds all components"
	@echo "  tradebook  Builds TradeBookEngine (CMake out-of-source)"
	@echo "  matchengine Builds TradeMatchEngine (uses ./build.sh)"
	@echo "  forex      Builds forex-grpc-cpp (CMake out-of-source)"
	@echo "  test       Run available tests (where configured)"
	@echo "  clean      Remove build artifacts"

tradebook:
	@$(MAKE) service-tradebook

matchengine:
	@if [ -x TradeMatchEngine/build.sh ]; then \
		cd TradeMatchEngine && ./build.sh; \
	else \
		@echo "No build script found for TradeMatchEngine"; exit 1; \
	fi

forex:
	@$(MAKE) service-forex

test:
	@echo "Running tests where available..."
	# Run ctest for built service targets if present
	@if [ -d $(BUILD_DIR)/services/tradebook ]; then \
		cd $(BUILD_DIR)/services/tradebook && $(CTEST) --output-on-failure || true; \
	fi
	@if [ -d $(BUILD_DIR)/services/matchengine ]; then \
		cd $(BUILD_DIR)/services/matchengine && $(CTEST) --output-on-failure || true; \
	fi
	@if [ -d $(BUILD_DIR)/services/forex ]; then \
		cd $(BUILD_DIR)/services/forex && $(CTEST) --output-on-failure || true; \
	fi

clean:
	@echo "Removing build directories..."
	rm -rf $(BUILD_DIR)
	@if [ -d TradeMatchEngine ]; then cd TradeMatchEngine && if [ -x build.sh ]; then ./build.sh clean || true; fi; fi
