# dxfeed-native-api

![Build status](https://github.com/dxFeed/dxfeed-native-api/actions/workflows/build.yml/badge.svg?branch=main)

A project repository that is a dxFeed API written in C++ and provides a C\Obj-C\C++ interface for using the dxFeed Java 
API features via dxfeed-graal-native-api as a backend

# Dependencies

- dxFeed Native Graal API v1.0.0
- catch2 v3.1.0 
- utfcpp v3.2.1
- fmt v9.1.0


# TBD
- [ ] Endpoint entity
- [ ] Subscription entity
- [ ] Native events mapper
- [x] System properties processing
- [ ] Obj-C artifacts (also for the Swift's FFI)
- [ ] C\C++ samples
- [ ] CI\CD
  - [x] Build 
  - [ ] Tests
    - [x] System properties processing
  - [ ] Packaging
- [ ] Docs
  - [x] System properties processing

# How To Build

## Prerequisites

- python
- Conan
- CMake
- clang\gcc\Visual Studio

## Build

```shell
mkdir build
cd build
conan install ..
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release 

```

## Run Tests

```shell
cd build
ctest -C Release --extra-verbose
```

### Known Issues

**Ubuntu**: it is possible that Conan incorrectly sets the C ABI for the C++ standard library (5 instead of 11), 
in which case it needs to be run with the following options:
```shell
conan install -s compiler.libcxx=libstdc++11 ..
```

**Windows**: it is possible that the Conan repository does not contain packages for the current version of the Visual 
Studio compiler (which is detected automatically and set to the default profile). You can set the previous version, 
for example 16:
```shell
conan install -s compiler.version=16 ..
```

# License
MPL-2.0
