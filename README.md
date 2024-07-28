# rk-integrator

rk-integrator is a C++ implementation of the family of explicit Runge-Kutta integrators. The implementation is general, allowing for any Runge-Kutta algorithm given a Butcher tableau. Users can input their own Butcher tableau or choose a default one from the provided tableaus.hpp header.

## Features

- General implementation of explicit Runge-Kutta integrators
- Support for custom Butcher tableaus
- A set of default Butcher tableaus provided in tableaus.hpp

## Dependencies

- [cpp-kit](https://github.com/ismawno/cpp-kit)
- [yaml-cpp](https://github.com/ismawno/yaml-cpp) (optional)
- [spdlog](https://github.com/gabime/spdlog) (optional)

## Building and Usage

This project is intended to be used as a git submodule within another project (parent repo). A premake file is provided for building and linking rk-integrator.

While these build instructions are minimal, this project is primarily for personal use. Although it has been built and tested on multiple machines (MacOS and Windows), it is not necessarily fully cross-platform or easy to build.

## License

rk-integrator is licensed under the MIT License. See LICENSE for more information.
