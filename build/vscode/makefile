INCS=-I../../include -IC:/Users/James/code/boost/boost1_72
LIBS=-lgdiplus -lgdi32 -lcomdlg32 -lws2_32 -lwsock32

demo: ../../demo/demo.cpp
	g++ -g ../../demo/demo.cpp -o../../bin/demo.exe  $(INCS) $(LIBS)
     
tcp: ../../demo/tcpdemo.cpp
	g++ -g ../../demo/tcpdemo.cpp -o../../bin/tcpdemo.exe  $(INCS) $(LIBS)