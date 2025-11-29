# FusionMatchTrade

A unified C++ trading platform that combines order matching, trade booking, and forex services. The repository uses a canonical service layout under `src/services/` and places tests under `tests/services/`.

**Canonical services**

- `src/services/tradebook`
- `src/services/matchengine`
- `src/services/forex`

**Top-level build**: Services are built together via CMake from the `src/services` directory and out-of-source build directory `build/services`.

**CI**: A GitHub Actions workflow runs on push and pull request to `main` and performs a full build + tests. See `.github/workflows/ci.yml`.

**Quick start (Ubuntu / WSL / GitHub Actions runner)**

Prerequisites (install on Debian/Ubuntu):

```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake pkg-config git \
	protobuf-compiler libprotobuf-dev libprotoc-dev libgrpc++-dev libcurl4-openssl-dev libssl-dev libgtest-dev
```

Build and run tests (recommended):

```bash
# from repository root
cmake -S src/services -B build/services -DCMAKE_BUILD_TYPE=Release
cmake --build build/services -- -j$(nproc)
cd build/services
ctest --output-on-failure
```

Notes:

- The CMake wrappers will skip building gRPC server code if generated gRPC headers are not available; the proto-only test will be skipped in that case.
- If your environment doesn't provide a pre-built GoogleTest, the workflow builds and installs it from the system package source (the CI job handles this step).

Local development targets

- Build everything (services):

```bash
cmake -S src/services -B build/services
cmake --build build/services
```

- Build a single service: pass `-S src/services/<service> -B build/services/<service>` to CMake or use the top-level `make` targets if provided.

- Run a single service's tests:

```bash
cd build/services/matchengine && ctest -V
```

Contributing & PR checks

- Open a PR against `main`; the GitHub Actions workflow will build the project and run tests automatically.

Troubleshooting

- If tests fail due to missing system dependencies on your machine, install the listed prerequisites and re-run the CMake configure step.
- If `protoc`/gRPC codegen is required, ensure `protoc` and the gRPC plugin are present in your PATH.

License

See source directories for per-component license details.
