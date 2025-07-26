# study-cpp 🧠💻

![Nightly Build](https://github.com/w15eacre/study-cpp/actions/workflows/nightly.yml/badge.svg?branch=main)

**An educational C++ repository exploring multithreading, C++20 features, and modern development practices.**

This project is built for self-learning purposes — examples and tests are added while reading books or exploring new topics.

---

## 📚 About the Project

This repository includes focused examples on:

* Modern **C++20** features (e.g., **modules**, **coroutines**);
* Using **threads**, **mutexes**, and concurrency primitives;
* Writing robust unit tests with **GoogleTest**;
* Building **safe and performant concurrent systems**.

## 🛠️ Build Instructions

```bash
conan install . \
  --profile:build=profiles/<profile> \
  --profile:host=profiles/<profile> \
  --output-folder=<build dir> \
  --build=missing \
  -s build_type=<build type>

source <build dir>/conanbuildenv-<lowercase build type>-<arch>.sh

cmake -B <build dir> \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=<build type> \
  -DCMAKE_TOOLCHAIN_FILE=<build dir>/conan_toolchain.cmake

cmake --build <build dir> --parallel <njobs> --target <target name>
```

## ✅ Running Tests

> Ensure the target is built before running tests.

```bash
cd <build dir>
ctest --output-on-failure
```

To run performance-specific tests:

```bash
ctest -R gtest_thread_safe_queue_perf
```

## How to build
```bash
conan install . \
  --profile:build=profiles/<profile> \
  --profile:host=profiles/<profile> \
  --output-folder=<build dir> \
  --build=missing \
  -s build_type=<build type>

source <build dir>/conanbuildenv-<lowercase build type>-<arch>.sh

cmake -B <build dir> \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=<build type> \
  -DCMAKE_TOOLCHAIN_FILE=<build dir>/conan_toolchain.cmake

cmake --build <build dir> --parallel <njobs> --target <target name>

```

## How to run test
> Note: You need build the target

```bash
cd <build dir>
ctest --output-on-failure
```

---

## List of subprojects

- [ThreadSafeQueue](thread_safe_queue/threadsafe_queue.md)
- [ThreadSafeList](thread_safe_list/threadsafe_list.md)