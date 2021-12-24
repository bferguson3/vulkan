```
*install vulkan 1.1 via PiKiss

sudo apt-get install libglfw3-dev

git clone https://github.com/KhronosGroup/Vulkan-ValidationLayers
cd Vulkan-ValidationLayers
mkdir build
cd build
python3 ../scripts/update_deps.py --dir ../external --config debug
cmake -C ../external/helper.cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build . --config Debug
sudo make install

sudo apt-get install libglm-dev

sudo apt-get install lcov

git clone https://github.com/google/shaderc --recursive
cd shaderc
mkdir build
cd build
cmake -GNinja -DCMAKE_BUILD_TYPE=Release ..
ninja
ctest
cd libshaderc
sudo cmake --install .
cd ../glslc
sudo cmake --install .
```
