# windex
 
A header only modern c++ wrapper for the windows API.

# Build

No separate build step is required. since this is a header only library.  

Add `"include "wex.h";` to application source files that use windex and add the `include` folder to the compiler search directories.

This avoids all the challenging configuration management problems that can result from trying to maintain a separate library build, when the library build and application build use different compilers or compiler options.

# Hello World

Complete [sample code](https://github.com/JamesBremner/windex/wiki/hello-world) for a hello world application.

# Application Programming Guide

[Reference documentation](https://github.com/JamesBremner/windex/wiki) for the classes and methods used to create a GUI application using windex.
