APPNAME=app
LDFLAGS = `pkg-config --static --libs glfw3` -lvulkan
CFLAGS = -std=c++17 -Wall
VULKAN_SDK=/usr/
#VK_LAYER_PATH=$(VULKAN_SDK)/share/vulkan/explicit_layer.d/
#VK_ICD_FILENAMES=$(VULKAN_SDK)/share/vulkan/icd.d/broadcom_icd.aarch64.json
LIBS=-L$(VULKAN_SDK)/lib
INCS=-I$(VULKAN_SDK)/include
GLC:=glslc

default: shaders
	g++ $(CFLAGS) $(LIBS) $(INCS) -o $(APPNAME) benvulkan.cpp HelloTriangle.cpp main.cpp $(LDFLAGS)

shaders: shaders/hello.frag.spv shaders/hello.vert.spv 

%.frag.spv: %.frag
	$(GLC) $< -o $@
%.vert.spv: %.vert
	$(GLC) $< -o $@

.PHONY: test clean 

#test: default
#	export VK_LAYER_PATH=$(VK_LAYER_PATH);\
#	export VK_ICD_FILENAMES=$(VK_ICD_FILENAMES);\
#	./$(APPNAME)

clean:
	rm -rf $(APPNAME)
	rm -rf shaders/*.spv 
