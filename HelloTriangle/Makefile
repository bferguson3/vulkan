APPNAME=app
LDFLAGS = `pkg-config --static --libs glfw3` -lvulkan

default: main.cpp
	g++ -Wall -o $(APPNAME) HelloTriangle.cpp main.cpp $(LDFLAGS)

.PHONY: test clean 

test: 
	./$(APPNAME)

clean:
	rm -f $(APPNAME)

