INCS=-I../../include -I../../../raven-set -I../../../await/include \
	-I../../../boost/boost1_78 
LIBS=-lgdiplus -lgdi32 -lcomdlg32 -lws2_32 -lwsock32 -lComctl32

demo: ../../demo/demo.cpp 
	g++ -g ../../demo/demo.cpp -o../../bin/demo.exe  $(INCS) $(LIBS)
     
tcp: ../../demo/tcpdemo.cpp ../../../raven-set/ctcp.cpp
	g++ -g -std=c++17 -o../../bin/tcpdemo.exe  \
	../../demo/tcpdemo.cpp ../../../raven-set/ctcp.cpp \
	$(INCS) \
	$(LIBS)

comdemo: ../../demo/com.cpp 
	g++ -g ../../demo/com.cpp -o../../bin/comdemo.exe  $(INCS) $(LIBS)

