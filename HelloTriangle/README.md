*install vulkan 1.1

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

