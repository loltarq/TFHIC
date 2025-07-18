clone repository along with ThermalFIST submodule, i.e.

  git clone --recursive-submodules https://github.com/loltarq/TFHIC.git

after git clone, to build ThermalFIST, in thermal_yields subfolder run bash command:

    mkdir build
    cd build
    cmake ../
    make
  
this will build ThermalFIST along with the code in scr.
When modifying/adding source code files in scr, it is necessary to update the CMakeLists.txt in the scr folder as well.
To build only the scr files, from build folder run

    cmake ../
    make <filename>
