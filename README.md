# FusionMatchTrade

A unified C++ trading platform that combines order matching, trade booking, and forex services.

## Projects

This monorepo contains the following integrated components:

### [TradeMatchEngine](./TradeMatchEngine/)
A high-performance C++ trade matching engine implementing a limit order book with price-time priority matching.

**Features:**
- Price-Time Priority Matching
- Multiple Order Types (Market, Limit)
- Multi-Symbol Support
- Real-time Portfolio Tracking
- Order Management with Cancellation
- High Performance: >80,000 orders/second

### [TradeBookEngine](./TradeBookEngine/)
A C++ trade booking engine with a modular architecture supporting multiple asset classes.

**Features:**
- Multi-Asset Support (Equity, Bond)
- Idempotency for Duplicate Prevention
- Event-Driven Architecture
- Repository Pattern for Pluggable Storage
- Thread-Safe Operations
- Extensible Validation Framework

### [forex-grpc-cpp](./forex-grpc-cpp/)
A C++ gRPC service that fetches forex rates from the Open Exchange Rates API.

**Features:**
- gRPC Service Interface
- Real-time Forex Rate Retrieval
- Docker Support
- GKE Deployment Ready

## Building

Each project has its own build system. See the README in each project directory for specific build instructions.

### TradeMatchEngine
```bash
cd TradeMatchEngine
./build.sh
```

### TradeBookEngine
```bash
cd TradeBookEngine
mkdir build && cd build
cmake ..
cmake --build .
```

### forex-grpc-cpp
```bash
cd forex-grpc-cpp
mkdir build && cd build
cmake ..
make
```

## Prerequisites

- CMake 3.16 or higher
- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- Google Test (for TradeMatchEngine tests)
- gRPC and Protobuf (for forex-grpc-cpp)
- CURL (for forex-grpc-cpp)

## License

See individual project directories for license information.
- TradeBookEngine: MIT License