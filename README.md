# Hydrogen: MVICFG Generator

Table of Contents
=================

* [Quick Start Guide:](#quick-start-guide)
  * [Building Hydrogen](#building-hydrogen)
  * [Using Hydrogen](#using-hydrogen)
* [Dependencies](#dependencies)
* [Documentation](#documentation)
* [Questions and Bug reporting:](#questions-and-bug-reporting)
* [Acknowledgment](#acknowledgment)

## Quick Start Guide:
It is advised to go through `Docs` to get an understanding of the project.  If you are in a hurry, this will get you set
up.

### Building Hydrogen
1) Before building the project, make sure the [dependencies](#dependencies) are met. You can also make use of the
 [docker image](https://hub.docker.com/r/ashwinkj/hydrogen_env), where the environment is already set up for you.
2) Clone `Hydrogen` from GitHub. If you are using the Docker, you can clone it into `/home/Hydrogen/MVICFG` folder.
3) Compile `Hydrogen` with the help of `CMakeLists.txt`. You can also use `GNU Make`, if that is the preferred method.
4) Assuming you are using the Docker and Ninja, the steps would be like below. But first [install](https://docs.docker.com/install/) Docker using the recommended method for your system.
```sh
# Download and run the Docker from your system.
$ docker run -it --name Hydrogen_Env ashwinkj/hydrogen_env
# The above command will put you inside the Docker Container.
$$ git clone https://github.com/iowastateuniversity-programanalysis/hydrogen /home/Hydrogen/MVICFG
$$ cd /home/Hydrogen/MVICFG
$$ mkdir BuildNinja
$$ cmake -B BuildNinja -G Ninja .
$$ cd BuildNinja
$$ ninja
```

### Using Hydrogen
1) Hydrogen needs both the source code and LLVM IR code to generate MVICFG and output it as `MVICFG.dot` for
 visualization.
2) To compile a single file program into LLVM IR code necessary for Hydrogen invoke `clang` with `-O0 -Xclang
 -disable-O0-optnone -g -emit-llvm -S` flag.
3) To generate MVICFG, call Hydrogen with both the LLVM IR and paths to their source files. Hydrogen will generate the
 diff from the source files to generate the MVICFG.
4) Assuming that you have two versions of `Prog.c`  in two folder `Buggy` and `Correct`, the tentative steps to generate MVICFG
 is shown below.
```sh
# In folder Buggy, compile Prog.c into LLVM IR (ProgV1.bc)
$ cd TestPrograms/Buggy
$ clang -c -O0 -Xclang -disable-O0-optnone -g -emit-llvm -S Prog.c -o ProgV1.bc
# Similary in folder Correct, compile Prog.c into LLVM IR (ProgV2.bc)
$ cd ../Correct
$ clang -c -O0 -Xclang -disable-O0-optnone -g -emit-llvm -S Prog.c -o ProgV2.bc
```
5) Once the LLVM IR are generated, then use Hydrogen to generate the MVICFG.
```sh
# Generic Command
$ Hydrogen.out <Path-to-LLVMIR_1> <Path-to-LLVMIR_2> .. <Path-to-LLVMIR_N> :: <Path-to-file1-for-Prog_V1> ..\
 <Path-to-fileN-for-Prog_V1> :: <Path-to-file1-for-Prog_V2> .. <Path-to-fileN-for-Prog_V2> ..
# Command for the above example from BuildNinja folder
$ ./Hydrogen.out ../TestPrograms/Buggy/ProgV1.bc ../TestPrograms/Correct/ProgV2.bc :: ../TestPrograms/Buggy/Prog.c ::\
 ../TestPrograms/Correct/Prog.c
```
6) A python script `BuildSystem.py` is provided to ease the process of invoking the Hydrogen executable. It will also
 rebuild Hydrogen (if necessary) and transfer the resulting `MVICFG.dot` file into the parent directory. *This python script
 is an example to base your own scripts. You might need to adpat this into bash script if running in Docker.*

## Dependencies
Hydrogen depends on the `LLVM Framework` and `Boost Libraries`. Roughly, the following are required for Hydrogen to
 build properly

| Program | Version |
|---------|---------|
| `Clang` | 8.0     |
| `LLVM`  | 8.0     |
| `Boost` | 1.69    |
| `Cmake` | 3.14    |
| `Ninja` | 1.9     |

While slightly older versions for `Cmake` and `Ninja` can be used without any problem, using older versions of
 `LLVM Framework` and `Boost` can have unwanted consequences and may even result in build failure.

## Documentation
Comments and more details for the program including the class structure with their supporting functions and their purpose can be
 found in `Doc` folder. Follow the `README.txt` inside the folder for more information.

## Questions and Bug reporting:
In case of questions about the usage/functionality of the framework or if you want to report a bug, please email `hydrogen-mvicfg@iastate.edu`. The repository will be updated with more features and bug fixes in time, but there will be a delay in responding to the emails.

## Acknowledgment
This work is developed under the support of the National Science Foundation (NSF) under Award 1542117
