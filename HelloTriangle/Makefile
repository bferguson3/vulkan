APPNAME=app
LDFLAGS = `pkg-config --static --libs glfw3` -lvulkan
CFLAGS = -std=c++17 -Wall
VULKAN_SDK=/usr/
#VK_LAYER_PATH=$(VULKAN_SDK)/share/vulkan/explicit_layer.d/
#VK_ICD_FILENAMES=$(VULKAN_SDK)/share/vulkan/icd.d/broadcom_icd.aarch64.json
LIBS=-L$(VULKAN_SDK)/lib
INCS=-I$(VULKAN_SDK)/include

default: main.cpp
	g++ $(CFLAGS) $(LIBS) $(INCS)  -o $(APPNAME) benvulkan.cpp HelloTriangle.cpp main.cpp $(LDFLAGS)

.PHONY: test clean 

test: default
	export VK_LAYER_PATH=$(VK_LAYER_PATH);\
	export VK_ICD_FILENAMES=$(VK_ICD_FILENAMES);\
	./$(APPNAME)

clean:
	rm -f $(APPNAME)

