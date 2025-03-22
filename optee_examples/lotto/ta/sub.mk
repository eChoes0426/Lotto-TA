global-incdirs-y += include
srcs-y += lotto_ta.c syscalls_stubs.c

cflags-y += -D__OPTEE__

ldadd-y += /home/harris/Desktop/optee_p/optee_examples/lotto/ta/lib/libsodium.a
global-incdirs-y += /include/libsodium/src/libsodium/include




# To remove a certain compiler flag, add a line like this
#cflags-template_ta.c-y += -Wno-strict-prototypes
