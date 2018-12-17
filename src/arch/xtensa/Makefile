platform-folder-$(CONFIG_BAYTRAIL) = baytrail
platform-folder-$(CONFIG_CHERRYTRAIL)  = baytrail
platform-folder-$(CONFIG_HASWELL) = haswell
platform-folder-$(CONFIG_BROADWELL) = haswell
platform-folder-$(CONFIG_APOLLOLAKE) = apollolake
platform-folder-$(CONFIG_CANNONLAKE) = cannonlake
platform-folder-$(CONFIG_SUECREEK) = suecreek
platform-folder-$(CONFIG_ICELAKE) = icelake

platform-folder := $(platform-folder-y)

need-bootloader := $(CONFIG_APOLLOLAKE) \
	$(CONFIG_CANNONLAKE) \
	$(CONFIG_SUECREEK) \
	$(CONFIG_ICELAKE)

ifneq ($(strip $(need-bootloader)),)
export BUILD_BOOTLOADER = y
export BUILD_MODULE = y
endif

ifeq ($(CONFIG_BROADWELL),y)
platform-ld-script = broadwell.x
else
platform-ld-script = $(platform-folder).x
endif

platform-rom-ld-script = rom.x
platform-bootldr-ld-script = boot_ldr.x

ifeq ($(CC),xt-xcc)
XCC := y
endif

arch-include = -I$(srctree)/src/arch/xtensa/include

ifeq ($(CONFIG_SMP),y)
arch-include += \
	-I$(srctree)/src/arch/xtensa/smp/include \
	-I$(srctree)/src/arch/xtensa/smp/xtos
else
arch-include += \
	-I$(srctree)/src/arch/xtensa/up/include \
	-I$(srctree)/src/arch/xtensa/up/xtos
endif

arch-include += -I$(srctree)/src/platform/$(platform-folder)/include

ifdef XCC
arch-include += -I$(ROOT_DIR)/arch/include
else
arch-include += -I$(srctree)/src/platform/$(platform-folder)/include/arch
endif

ARCH_CPPFLAGS += $(arch-include)

ARCH_CFLAGS += -fno-inline-functions -nostdlib -mlongcalls -mtext-section-literals \
	-O2 -g -Wall -Werror -Wl,-EL -Wmissing-prototypes
ARCH_LDFLAGS += -nostdlib -Wl,--no-check-sections -u call_user_start -Wl,-static
ARCH_AFLAGS += -fno-inline-functions -nostdlib -mlongcalls

# generate linker scripts from platform headers
link-deps = $(srctree)/src/platform/$(platform-folder)/include/platform/memory.h

ifdef XCC
link-deps += \
	$(ROOT_DIR)/arch/include/xtensa/config/core-isa*
else
link-deps += \
	$(srctree)/src/platform/$(platform-folder)/include/arch/xtensa/config/core-isa*
endif

main-lds-in = $(srctree)/src/platform/$(platform-folder)/$(platform-ld-script).in
main-lds-out = $(objtree)/$(platform-ld-script)

$(main-lds-out): Makefile $(main-lds-in) $(link-deps)
	cat $(main-lds-in) | $(CPP) -P $(LINUXINCLUDE) $(arch-include) - >$@

lds-scripts = $(main-lds-out)

ifeq ($(BUILD_BOOTLOADER),y)
boot-lds-in = $(srctree)/src/platform/$(platform-folder)/$(platform-bootldr-ld-script).in
boot-lds-out = $(objtree)/$(platform-bootldr-ld-script)

$(boot-lds-out): Makefile $(boot-lds-in) $(link-deps)
	cat $(boot-lds-in) | $(CPP) -P $(LINUXINCLUDE) $(arch-include) - >$@

lds-scripts += $(boot-lds-out)
endif

ifeq ($(CONFIG_BUILD_VM_ROM),y)
rom-lds-in = $(srctree)/src/platform/$(platform-folder)/$(platform-rom-ld-script).in
rom-lds-out = $(objtree)/$(platform-rom-ld-script)

$(rom-lds-out): Makefile $(rom-lds-in) $(link-deps)
	cat $(rom-lds-in) | $(CPP) -P $(LINUXINCLUDE) $(arch-include) - >$@

lds-scripts += $(rom-lds-out)
endif

archprepare: $(lds-scripts)

archclean:
	$(Q)rm -f $(lds-scripts)

core-y += src/arch/xtensa/

# rimage

build-rimage: FORCE
	$(Q)$(MAKE) $(build)=rimage $@

rimage: build-rimage ;
