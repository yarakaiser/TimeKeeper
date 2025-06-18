# TimeKeeper

A simple app for windows to track the time spend on different applications.
Needs to be started as admin to properly track all applications.

To build:  
mkdir build  
cmake -S . -B build -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build

To start:  
build\Timekeeper.exe

![Timer](./Screenshot%20(111).png)
![Logger](./Screenshot%20(112).png)