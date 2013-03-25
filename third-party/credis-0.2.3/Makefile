CFLAGS = -g  -Wall
LDFLAGS =
#CPPFLAGS = -DPRINTDEBUG

# build shared lib under OS X or Linux
OS = $(shell uname -s)
ifeq ($(OS),Darwin)
	SHAREDLIB_LINK_OPTIONS=-dynamiclib -Wl,-install_name -Wl,
else
	SHAREDLIB_LINK_OPTIONS=-shared -Wl,-soname,
endif

# targets to build with 'make all'
TARGETS = credis-test libcredis.a libcredis.so

all: $(TARGETS)

credis-test: credis-test.o libcredis.a
	$(CC) $(CFLAGS) $(LDFLAGS) $(CPPFLAGS) -o $@ $^

libcredis.a: credis.o
	$(AR) -cvq $@ $^

libcredis.so: credis.o
	$(CC) $(SHAREDLIB_LINK_OPTIONS)$@ -o $@ $^

credis.o: credis.c credis.h Makefile
	$(CC) -c -fPIC $(CFLAGS) $(CPPFLAGS) -o $@ credis.c

install:
	@echo "Installing library (to be done)"
clean:
	rm -f *.o *~ $(TARGETS)
