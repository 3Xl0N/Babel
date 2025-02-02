Pour compiler
mkdir build
cd build 
Dans build faire : conan install .. --output-folder=.
ensuite dans build faire après ça: cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake

Et enfin faire make pour la génération des binaires.

