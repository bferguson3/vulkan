LDFLAGS = `pkg-config --static --libs glfw3` -lvulkan

VulkanTest: main.cpp
	g++ -o VulkanTest main.cpp $(LDFLAGS)

.PHONY: test clean 

test: VulkanTest
	./VulkanTest 

clean:
	rm -f VulkanTest

