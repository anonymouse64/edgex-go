#
# apt-get install build-essential
# make clean
# make
# make install

CFLAGS += -g -O0 -Wall -Wstrict-prototypes

# snapcraft will copy anything from here
INSTALL_DIR := ../install

all: wraplib.so drop

wraplib.so:
	${CC} ${CFLAGS} -fPIC -shared ${LDFLAGS} lib.c -o $@ -ldl

drop:
	${CC} ${CFLAGS} ${LDFLAGS} drop.c -o $@

install: wraplib.so drop
	mkdir -p ${INSTALL_DIR}
	cp -f wraplib.so ${INSTALL_DIR}/wraplib.so
	cp -f drop ${INSTALL_DIR}/drop

clean:
	rm -f ./wraplib.so ./drop
