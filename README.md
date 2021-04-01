# windex
 
A header only modern c++ wrapper for the windows API.


|GUI ELEMENTS||
|---|---|
|gui	|	The base class for all windex gui elements|
|label	|	A widget that displays a string|
button	|	A widget that user can click to start an action
menu	|	A drop down list of options that user can click to start an action
menubar	|	A widget that displays across top of a window and contains a number of dropdown menues
checkbox|	A widget that user can click to toggle a true/false value
choice	|	A widget where user can choose from a dropdown list of strings
list	|	A widget where user can choose from a displayed list of strings
editbox	|	A widget where user can enter a string, single line
multiline | A widget where user can enter a string, multiple lines
gauge	|	A widget to display a value on a circular clock face
radiobutton|	A widget that user can click to select one of an exclusive set of options
slider	|	A widget which user can drag to change a value

|PANELS||
|---|---|
|panel	|	A child window that can contain widgets|
|layout	|	A panel which arranges the widgets it contains in a grid|
|groupbox|	A panel displaying a title and box around contents|
|tabbed	|	A widget where user can select which panel to display by clicking a tab button|

|POP-UPS||
|---|---|
msgbox	|	A popup with a message
filebox	|	A popup window where user can browse folders and select a file
inputbox|	A popup window where user can edit a set of values

|PROPERTY GRID||
|---|---|
property	|A name value pair
propertyGrid|	A grid of properties

|PLOT||
|---|---|
plot		|Draw a 2D plot
trace		|Single trace to be plotted

|COMMUNICATIONS||
|---|---|
com		|Read / write to COM serial port
tcp  |Read / write to TCP/IP socket, server or client

|MISCELLANEOUS||
|---|---|
maker		|A class for making windex objects
eventhandler	|A class where application code registers functions to be called when an event occurs
drop		|A widget where users can drop files dragged from windows explorer
shapes		|A class that offers application code methods to draw on a window
sMouse		|A structure containing the mouse status for event handlers
timer		|Generate events at regularly timed intervals
window2file	|Save window contents to an image file in PNG format

# Build

No separate build step is required. since this is a header only library.  This avoids all the challenging configuration management problems that can result from trying to maintain a separate library build, when the library build and application build use different compilers or compiler options.  

 - Add `"include "wex.h";` to application source files that use windex.
 - Add the `include` folder to the compiler search directories.
 - Add the windows libraries gdi32, comdlg32 to the linker command

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

## Simulation Properties

<img src="https://github.com/JamesBremner/windex/blob/master/doc/simparams.png" alt="props"></a>