# XORList
Implementation of XOR Linked list

## Setting up
### Unix:
Variable CMAKE_ARGS is for passing arguments to cmake invocation

* Simple run : `make`  
* Make **DEBUG** build and run tests: `make CMAKE_ARGS="-DCMAKE_BUILD_TYPE=Debug"`  
* Make **RELEASE** build and run tests: `make CMAKE_ARGS="-DCMAKE_BUILD_TYPE=Release"`  
* Testing coverage (**DEBUG** build only): `make CMAKE_ARGS="-DCMAKE_BUILD_TYPE=Debug" collect_coverage`  

### Windows:
Use [cmake](https://cmake.org/download/) for generating MSVS solution / smth else : `cd build && cmake.exe ../`

### Testing coverage (Unix only)
Testing coverage has the next dependencies:
* [gcov](https://gcc.gnu.org/onlinedocs/gcc/Gcov.html)
* [lcov](https://wiki.documentfoundation.org/Development/Lcov) (install using apt : `apt-get install lcov`)
* [genhtml](https://linux.die.net/man/1/genhtml)
