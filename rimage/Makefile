hostprogs-y := rimage

HOST_EXTRACFLAGS += -O2 -g -Wall -Werror -Wl,-EL \
	-Wmissing-prototypes -Wimplicit-fallthrough=3 \
	-I$(srctree)/src/include -I$(objtree)/include

HOSTLDLIBS_rimage += -lcrypto

rimage-objs := \
	file_simple.o \
	man_apl.o \
	man_cnl.o \
	man_kbl.o \
	cse.o \
	css.o \
	plat_auth.o \
	hash.o \
	pkcs1_5.o \
	manifest.o \
	elf.o \
	rimage.o

build-rimage: $(obj)/rimage ;
