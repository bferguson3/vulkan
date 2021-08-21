APPNAME=app
LDFLAGS = `pkg-config --static --libs glfw3` -lvulkan

default: main.cpp
	g++ -Wall -o $(APPNAME) benvulkan.cpp HelloTriangle.cpp main.cpp $(LDFLAGS)

.PHONY: test clean 

test: default
	./$(APPNAME)

clean:
	rm -f $(APPNAME)

