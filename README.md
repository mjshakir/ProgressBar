# ProgressBar
his utility provides a progress bar with Estimated Time to Completion (ETC) and elapsed time tracking, perfect for monitoring the progress of lengthy operations, such as those found in machine learning projects. This project is inspired by [gipert's progressbar](https://github.com/gipert/progressbar), and it extends the original implementation by adding ETC and elapsed time functionalities for a more informative experience.

## Features

- **ETC Calculation:** Provides an estimate of the time remaining until completion.
- **Elapsed Time Tracking:** Keeps track of the time elapsed since the start of the operation.
- **Customizable Appearance:** Allows the customization of the progress bar with different characters and formats.
- **Cross-Platform:** Works on different platforms with C++20 support.

## Getting Started

Follow these instructions to integrate MyProgressBar into your own projects.

### Prerequisites

Make sure you have the following installed on your system:
- CMake (version 3.5 or later)
- A C++20 compliant compiler
- fmt library
- Intel Threading Building Blocks (TBB)
- Boost library

### Installing Dependencies on Ubuntu

To install the required libraries on Ubuntu, use the following commands:

```sh
sudo apt-get update; sudo apt-get install libfmt-dev libtbb-dev libboost-all-dev ninja-build -y
```

### Cloning the Repository
First, clone the MyProgressBar repository using git:
```sh
git clone https://github.com/mjshakir/ProgressBar.git
```

### Building the Project

```sh
cd ProgressBar
```

```sh
cmake -DFORCE_COLORED_OUTPUT=ON -DCMAKE_BUILD_TYPE=Release -B build -G Ninja
```

```sh
cd build
```

```sh
ninja
```

## Integration in Other CMake Projects
To integrate MyProgressBar into your project using CMake, follow these steps:
1. Add MyProgressBar as a subdirectory in your project's `CMakeLists.txt`:
```cmake
add_subdirectory(path/to/ProgressBar)
```
2. Link against MyProgressBar in your executable or library:
```cmake
target_link_libraries(your_target_name PRIVATE ProgressBar)
```

### Using Shared or Static Libraries
By default, MyProgressBar is built as a shared library. If you prefer a static library, you can configure this in the CMake command:

```sh
cmake -DBUILD_PROGRESSBAR_SHARED_LIBS=OFF ..
```

### Running Examples and Tests

To run the provided examples:
```sh
./bin/MyProgressBar_example
```

To execute the unit tests:
```sh
ninja test # or ctest
```

## Documentation

Please find more detailed usage instructions and examples in the example directory. The main concepts are commented on in the source code for your convenience.

## Contributions and Support

Contributions are welcome! If you'd like to contribute or have found bugs, please open an issue or a pull request on GitHub.

For questions and support, use the GitHub issues tab to reach out.

## Acknowledgments

This project was inspired by gipert's progressbar implementation. If you're looking for a more lightweight progress bar without ETC and elapsed time tracking, check out [gipert's progressbar](https://github.com/gipert/progressbar).

