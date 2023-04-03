# rk-integrator

rk-integrator is a C++ implementation of the family of explicit Runge-Kutta integrators. The implementation is general, allowing for any Runge-Kutta algorithm given a Butcher tableau. Users can input their own Butcher tableau or choose a default one from the provided tableaus.hpp header.

## Features

- General implementation of explicit Runge-Kutta integrators
- Support for custom Butcher tableaus
- A set of default Butcher tableaus provided in tableaus.hpp
- Easy integration into existing C++ projects

## Dependencies

- [debug-tools](https://github.com/Ismael99Bueno/debug-tools): A set of tools for debugging the integrator
- [profile-tools](https://github.com/Ismael99Bueno/profile-tools): A set of tools for profiling the integrator
- [ini-parser](https://github.com/Ismael99Bueno/ini-parser): A simple INI file parser that allows for reading and writing the state of an integrator or the data of a butcher tableau to and from a file

## Usage

To use rk-integrator, simply include the `integrator.hpp` header in your project. If you'd like to use a default Butcher tableau, also include the `tableaus.hpp` header.

rk-integrator is built using premake5 as a static library. To use it, you must create a premake5 workspace with a user-implemented entry point that uses the rk-integrator library. You can then build the workspace with premake5 to create an executable.

It is advisable to include rk-integrator in a user-made repository as a git submodule. This allows you to keep the rk-integrator code separate from your own code, making it easier to manage dependencies and track changes.

For more information on how to use rk-integrator, please refer to the documentation.

## License

rk-integrator is licensed under the MIT License. See LICENSE for more information.
