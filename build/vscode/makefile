VPATH = ../../include 
INCS=-I../../include -I../../../raven-set -I../../../await/include \
	-I../../../../code_ext/boost/boost_1_84_0
LIBS=-lgdiplus -lgdi32 -lcomdlg32 -lws2_32 -lwsock32 -lComctl32

demo: ../../demo/demo.cpp 
	g++ -g ../../demo/demo.cpp -o../../bin/demo.exe  $(INCS) $(LIBS)

test: unitTest.cpp plot2d.h
	g++ -g  ../../include/unitTest.cpp ../../../raven-set/cutest.cpp \
		-o../../bin/test.exe $(INCS) $(LIBS)
     
tcp: ../../demo/tcpdemo.cpp ../../../raven-set/ctcp.cpp
	g++ -g -std=c++17 -o../../bin/tcpdemo.exe  \
	../../demo/tcpdemo.cpp ../../../raven-set/ctcp.cpp \
	$(INCS) \
	$(LIBS)

comdemo: ../../demo/com.cpp 
	g++ -O2 ../../demo/com.cpp -o../../bin/comdemo.exe  $(INCS) $(LIBS)

