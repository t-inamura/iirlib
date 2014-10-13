GLIB_CFLAGS  = `pkg-config --cflags glib-2.0`
GLIB_LDFLAGS = `pkg-config --libs   glib-2.0`
CFLAGS   = -Wall -g -O2 -D__cplusplus $(GLIB_CFLAGS) -I.
LDFLAGS  = $(GLIB_LDFLAGS) -L/usr/local/lib -fPIC
OBJS     = iirlib.o threshold.o connection.o multisocket.o
LIB_DIR  = /usr/local/lib
CC       = g++

OS = $(shell /bin/uname | sed s/-.*//)
ifeq ($(OS), CYGWIN_NT)
LINK	 = g++ -shared
LPFX	 = cyg
LSFX	 = dll
TARGET   = cygiirlib.dll
else
LINK	 = g++ -shared
LPFX	 = lib
LSFX	 = so
TARGET   = libiirlib.so
CFLAGS   += -fPIC
endif

TEST_TARGET = test_server test_client

all:
	make $(TARGET)

test:
	make $(TEST_TARGET)

iirlib:	$(OBJS)

$(TARGET):	$(OBJS)
	g++ $(LDFLGAS) -shared -o $@ $^ $(GLIB_LDFLAGS)

iirlib.o:	iirlib.cpp iirlib.h
	g++ $(CFLAGS) -c $< -o $@ 

threshold.o:	threshold.cpp threshold.h
	g++ $(CFLAGS) -c $< -o $@ 

connection.o:	connection.cpp connection.h
	g++ $(CFLAGS) -c $< -o $@ 

multisocket.o:	multisocket.cpp multisocket.h
	g++ $(CFLAGS) -c $< -o $@ 

test_server:	test_server.o
	g++ $(LDFLAGS) -o $@ $< -liirlib
test_server.o:	test_server.cpp
	g++ $(CFLAGS) -c $< -o $@ 

test_client:	test_client.o
	g++ $(LDFLAGS) -o $@ $< -liirlib
test_client.o:	test_client.cpp
	g++ $(CFLAGS) -c $< -o $@ 


install:
	install $(TARGET) $(LIB_DIR)
	install iirlib.h /usr/local/include/
	install threshold.h /usr/local/include/
	install connection.h /usr/local/include/
	install multisocket.h /usr/local/include/

clean:
	$(RM) -f *.o *~ *.a *.so $(OBJS) $(TARGET) $(TEST_TARGET)
