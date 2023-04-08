# rk-integrator

rk-integrator is a C++ implementation of the family of explicit Runge-Kutta integrators. The implementation is general, allowing for any Runge-Kutta algorithm given a Butcher tableau. Users can input their own Butcher tableau or choose a default one from the provided tableaus.hpp header.

## Features

- General implementation of explicit Runge-Kutta integrators
- Support for custom Butcher tableaus
- A set of default Butcher tableaus provided in tableaus.hpp
- Easy integration into existing C++ projects

## Dependencies

- [debug-tools](https://github.com/ismawno/debug-tools): A set of tools for debugging the integrator
- [profile-tools](https://github.com/ismawno/profile-tools): A set of tools for profiling the integrator
- [ini-parser](https://github.com/ismawno/ini-parser): A simple INI file parser that allows for reading and writing the state of an integrator or the data of a butcher tableau to and from a file

There is a Python script located in the `scripts` folder named `fetch_dependencies.py`. This script will automatically add all the dependencies as git submodules, provided that the user has already created their own repository and included the current project as a git submodule (or at least downloaded it into the repository). To ensure all runs smoothly once the script has been executed, do not rename the folders containing the various dependencies. All external dependencies, those not created by the same author, will be added as submodules within a folder called `vendor`.

## Building and Usage

1. Ensure you have `premake5` and `make` installed on your system. `premake5` is used to generate the build files, and `make` is used to compile the project.
2. Create your own repository and include the current project as a git submodule (or at least download it into the repository).
3. Run the `fetch_dependencies.py` script located in the `scripts` folder to automatically add all the dependencies as git submodules.
4. Create an entry point project with a `premake5` file, where the `main.cpp` will be located. Link all libraries and specify the kind of the executable as `ConsoleApp`. Don't forget to specify the different configurations for the project.
5. Create a `premake5` file at the root of the repository describing the `premake` workspace and including all dependency projects.
6. Build the entire project by running the `make` command in your terminal. You can specify the configuration by using `make config=the_configuration`.
7. To use rk-integrator, simply include the `integrator.hpp` header in your project. If you'd like to use a default Butcher tableau, also include the `tableaus.hpp` header.

For more information on how to use rk-integrator, please refer to the documentation.

## License

rk-integrator is licensed under the MIT License. See LICENSE for more information.
