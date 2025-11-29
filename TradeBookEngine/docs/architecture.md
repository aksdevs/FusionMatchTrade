# Trade Booking Engine Documentation

## Architecture Overview

The Trade Booking Engine follows a modern C++ project structure with clear separation of concerns:

```
TradeBookEngine/
├── include/           # Public API headers
├── lib/              # Library implementations
│   └── core/         # Core business logic
├── apps/             # Applications
│   └── console/      # Console demo application
├── tests/            # Unit and integration tests
├── docs/             # Documentation
├── examples/         # Usage examples
└── scripts/          # Build and utility scripts
```

## Core Components

### Models
- **Trade**: Core trade entity with full business logic
- **TradeDto**: Data transfer object for API boundaries
- **Enums**: Asset classes, trade status, and side definitions

### Services
- **TradeService**: Main business logic for trade booking
- **Validation**: Asset-specific validation framework
- **Repository**: Pluggable storage abstraction

### Events
- **TradeBookedEvent**: Published when trades are successfully booked
- **Event Publisher**: Abstraction for event publishing

## Design Patterns Used

1. **Repository Pattern**: `ITradeRepository` for data access abstraction
2. **Strategy Pattern**: `IAssetValidator` for asset-specific validation
3. **Factory Pattern**: C-style factories for dependency injection
4. **Observer Pattern**: Event publishing for trade notifications
5. **DTO Pattern**: Separation of API models from domain models

## Thread Safety

The engine is designed to be thread-safe:
- `InMemoryTradeRepository` uses mutex protection
- Immutable value objects where possible
- Stateless service classes

## Extension Points

1. **New Asset Classes**: Implement `IAssetValidator`
2. **Storage Backends**: Implement `ITradeRepository` 
3. **Event Sinks**: Implement `IEventPublisher`
4. **Applications**: Use the core library as dependency