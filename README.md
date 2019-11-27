# windex
 
A header only modern c++ wrapper for the windows API.

# Build

No separate build step is required. since this is a header only library.  This avoids all the challenging configuration management problems that can result from trying to maintain a separate library build, when the library build and application build use different compilers or compiler options.  

 - Add `"include "wex.h";` to application source files that use windex.
 - Add the `include` folder to the compiler search directories.
 - Add the windows library gdi32 to the linker command

# Hello World

Complete [sample code](https://github.com/JamesBremner/windex/wiki/hello-world) for a hello world application.

# Application Programming Guide

[Reference documentation](https://jamesbremner.github.io/windex/hierarchy.html) for the classes and methods used to create a GUI application using windex.

# Apps using windex

## Demo & Test

An application to demonstrate and test the windex features is [available](https://github.com/JamesBremner/windex/releases/latest).  The source is in the demo folder, and a code::blocks project to build it in build/codeblocks.

<img src="https://github.com/JamesBremner/windex/blob/master/doc/demo.png" alt="demo"></a>

## DXF File Viewer

<img src="https://github.com/JamesBremner/windex/blob/master/doc/gears.PNG" alt="gears"></a>

https://github.com/JamesBremner/DXF_Viewer
