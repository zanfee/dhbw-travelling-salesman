## Setup C for Visual Studio Code (Windows)
 
#### 1. [Download](https://code.visualstudio.com/) and install Visual Studio Code
 
#### 2. [Download](http://www.mingw.org/) and install MinGW ([Video](https://youtu.be/guM4XS43m4I?t=29))  
  You will need the following packages:
  - mingw32-base-bin (A Basic MinGW Installation)
  - mingw32-gcc-g++-bin (The GNU C++ Compiler)
  
#### 3. Setup Path Variable ([Video](https://youtu.be/guM4XS43m4I?t=338))
  Navigate to environment variables and add the path of the bin folder inside the installation folder of MinGW (by default it's C:\MinGW\bin) to the path variable.
  You will need to restart Visual Studio Code before you can compile inside the internal terminal.
 
#### 4. Install the following extensions for Visual Studio Code ([Video](https://youtu.be/77v-Poud_io?t=51))
- [C/C++](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools)
- [Code Runner](https://marketplace.visualstudio.com/items?itemName=formulahendry.code-runner)
 
#### 5. Adjust preferences for Code Runner ([Video](https://youtu.be/77v-Poud_io?t=349))
- File -> Preferences -> Settings
- Search for "Code runner: Run In Terminal" and make sure it's checked.
- Search for "Code runner: Executor Map" and change the command for c code to include -Wall -pedantic-errors:
`"c": "cd $dir && gcc $fileName -Wall -pedantic-errors -o $fileNameWithoutExt && $dir$fileNameWithoutExt",`
