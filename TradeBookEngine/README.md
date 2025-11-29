# Trade Booking Engine

A C++ trade booking engine with a modular architecture supporting multiple asset classes.

## Architecture

The project follows a layered architecture with clear separation of concerns:

### Core Library (`TradeBookEngine.Core.Cpp`)
- **Models**: Trade entities and DTOs
- **Services**: Business logic (TradeService)
- **Interfaces**: Abstractions for repositories and event publishers
- **Validators**: Asset-specific validation logic
- **Utils**: Utility functions for ID generation, date handling, and validation

### Console Application (`TradeBookEngine.ConsoleCpp`)
- Command-line interface demonstrating the core functionality
- Example usage of the trade booking engine

## Features

- **Multi-Asset Support**: Equity and Bond trades with asset-specific validation
- **Idempotency**: Duplicate trade prevention using idempotency keys
- **Event-Driven Architecture**: Trade booking events are published
- **Repository Pattern**: Pluggable storage implementations
- **Thread-Safe**: In-memory repository with proper locking
- **Validation Framework**: Extensible validation system per asset class

## Asset Classes Supported

### Equity
- Requires: Exchange information
- Validates: Instrument ID, notional amount, counterparty

### Bond  
- Requires: Maturity date, credit rating
- Validates: Instrument ID, notional amount, counterparty

## Building the Project

### Prerequisites
- CMake 3.20 or higher (recommended)
- C++17 compatible compiler (GCC, Clang, MSVC)

### Build Steps

#### Using CMake (Recommended)
```bash
# Create build directory
mkdir build
cd build

# Configure
cmake ..

# Build
cmake --build .

# Run the console application
./bin/tradebook_console    # Linux/Mac
.\bin\tradebook_console.exe # Windows
```

#### Using the provided build script (Windows)
```cmd
# Run the build script
scripts\build.bat
```

#### Using PowerShell build script
```powershell
# Run the PowerShell build script  
.\scripts\build.ps1
```

#### Manual compilation (if CMake is not available)
```bash
# Compile core library
g++ -c -std=c++17 -Iinclude lib/core/src/*.cpp
ar rcs libTradeBookEngineCore.a *.o

# Compile and link console application
g++ -std=c++17 -Iinclude apps/console/main.cpp libTradeBookEngineCore.a -o tradebook_console
```

## Usage Example

```cpp
#include "TradeBookEngine/Core/TradeService.hpp"

// Create dependencies
auto repo = std::make_shared<InMemoryTradeRepository>();
auto publisher = std::make_shared<NoOpEventPublisher>();

// Create service
auto tradeService = std::make_shared<TradeService>(repo, publisher);

// Add validators
tradeService->AddValidator(std::make_shared<EquityValidator>());

// Create and book a trade
TradeDto tradeDto;
tradeDto.AssetClass = AssetClass::Equity;
tradeDto.InstrumentId = "MSFT";
tradeDto.Counterparty = "Goldman Sachs";
tradeDto.Notional = 1000000.0;
tradeDto.Currency = "USD";
tradeDto.Additional["Exchange"] = "NASDAQ";

auto trade = tradeService->BookTrade(tradeDto);
```

## Project Structure

```
TradeBookEngine/                            # Root directory
├── CMakeLists.txt                          # Root CMake configuration  
├── README.md                               # Project documentation
├── LICENSE                                 # License file
├── include/                                # Public API headers
│   └── TradeBookEngine/Core/               # Core library public interface
│       ├── Enums.hpp                       # Asset classes, trade status, etc.
│       ├── Trade.hpp                       # Trade entity model
│       ├── TradeDto.hpp                    # Data transfer object
│       ├── TradeService.hpp                # Main service class
│       ├── Utils.hpp                       # Utility functions
│       ├── Events/
│       │   └── TradeBookedEvent.hpp        # Event definitions
│       ├── Interfaces/
│       │   ├── IEventPublisher.hpp         # Event publisher interface
│       │   └── ITradeRepository.hpp        # Repository interface
│       └── Validators/
│           └── IAssetValidator.hpp         # Validator interface
├── lib/                                    # Library implementations
│   └── core/                               # Core business logic library
│       ├── CMakeLists.txt                  # Core library build config
│       └── src/                            # Private implementation files
│           ├── Trade.cpp                   # Trade entity implementation
│           ├── TradeService.cpp            # Service implementation
│           ├── Utils.cpp                   # Utility implementations
│           ├── InMemoryTradeRepository.cpp # In-memory repository
│           ├── NoOpEventPublisher.cpp      # No-op event publisher
│           └── Validators.cpp              # Asset validators
├── apps/                                   # Applications using the core library
│   └── console/                            # Console demo application
│       ├── CMakeLists.txt                  # Console app build config
│       └── main.cpp                        # Application entry point
├── tests/                                  # Unit and integration tests
│   └── CMakeLists.txt                      # Test configuration (placeholder)
├── docs/                                   # Documentation
│   └── architecture.md                     # Architecture overview
├── examples/                               # Usage examples
│   └── basic_usage.cpp                     # Basic usage example
└── scripts/                                # Build and utility scripts
    ├── build.bat                           # Windows batch build script
    └── build.ps1                           # PowerShell build script
```

## Extending the Engine

### Adding New Asset Classes

1. Add the new asset class to `Enums.hpp`:
```cpp
enum class AssetClass {
    Equity,
    Bond,
    Derivative,  // New asset class
    Commodity,
    Currency
};
```

2. Create a validator in `Validators.cpp`:
```cpp
class DerivativeValidator : public IAssetValidator {
    // Implementation
};
```

3. Register the validator in your application:
```cpp
tradeService->AddValidator(std::make_shared<DerivativeValidator>());
```

### Custom Repository Implementation

Implement the `ITradeRepository` interface:
```cpp
class DatabaseTradeRepository : public ITradeRepository {
    // Your database implementation
};
```

### Custom Event Publisher

Implement the `IEventPublisher` interface:
```cpp
class KafkaEventPublisher : public IEventPublisher {
    void Publish(const TradeBookedEvent& event) override {
        // Publish to Kafka
    }
};
```

## Thread Safety

The `InMemoryTradeRepository` implementation is thread-safe using std::mutex. All public methods are protected against concurrent access.

## Error Handling

The engine uses exceptions for error handling:
- `std::invalid_argument`: Validation errors
- `std::runtime_error`: Runtime errors (e.g., repository failures)

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.