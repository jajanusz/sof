fw-name-$(CONFIG_BAYTRAIL) = byt
fw-name-$(CONFIG_CHERRYTRAIL)  = cht
fw-name-$(CONFIG_HASWELL) = hsw
fw-name-$(CONFIG_BROADWELL) = bdw
fw-name-$(CONFIG_APOLLOLAKE) = apl
fw-name-$(CONFIG_CANNONLAKE) = cnl
fw-name-$(CONFIG_SUECREEK) = sue
fw-name-$(CONFIG_ICELAKE) = icl

fw-name := $(fw-name-y)

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

quiet_cmd_lds_main = LDS     $@
      cmd_lds_main = cat $(main-lds-in) | $(CPP) -P $(LINUXINCLUDE) $(arch-include) - >$@

$(main-lds-out): Makefile $(main-lds-in) $(link-deps) FORCE
	$(call if_changed,lds_main)

lds-scripts = $(main-lds-out)

ifeq ($(BUILD_BOOTLOADER),y)
boot-lds-in = $(srctree)/src/platform/$(platform-folder)/$(platform-bootldr-ld-script).in
boot-lds-out = $(objtree)/$(platform-bootldr-ld-script)

quiet_cmd_lds_boot = LDS     $@
      cmd_lds_boot = cat $(boot-lds-in) | $(CPP) -P $(LINUXINCLUDE) $(arch-include) - >$@

$(boot-lds-out): Makefile $(boot-lds-in) $(link-deps) FORCE
	$(call if_changed,lds_boot)

lds-scripts += $(boot-lds-out)
endif

ifeq ($(CONFIG_BUILD_VM_ROM),y)
rom-lds-in = $(srctree)/src/platform/$(platform-folder)/$(platform-rom-ld-script).in
rom-lds-out = $(objtree)/$(platform-rom-ld-script)

quiet_cmd_lds_rom = LDS     $@
      cmd_lds_rom = cat $(rom-lds-in) | $(CPP) -P $(LINUXINCLUDE) $(arch-include) - >$@

$(rom-lds-out): Makefile $(rom-lds-in) $(link-deps) FORCE
	$(call if_changed,lds_rom)

lds-scripts += $(rom-lds-out)
endif

archprepare: $(lds-scripts)

$(lds-scripts): prepare1

archclean: bin-clean
	$(Q)rm -f $(objtree)/*.x

core-y += src/arch/xtensa/

# rimage

build-rimage: buildsystem-prepare FORCE
	$(Q)$(MAKE) $(build)=rimage $@

rimage: build-rimage ;

# TODO separate boot loader build, it should be in boot/
# do we need to again build all hal & xtos objects?
ifeq ($(CONFIG_SMP),y)
libreset-o-files = \
	smp/xtos/memctl_default.o \
	smp/xtos/reset-vector.o

libreset-o-files := $(addprefix  $(objtree)/src/arch/xtensa/,$(libreset-o-files))
libhal-o-files := $(objtree)/src/arch/xtensa/smp/hal/built-in.a
boot-vectors-o := $(objtree)/src/arch/xtensa/smp/xtos/_vectors.o
else
libreset-o-files = \
	up/xtos/reset-vector.o

libreset-o-files := $(addprefix  $(objtree)/src/arch/xtensa/,$(libreset-o-files))
libhal-o-files := $(objtree)/src/arch/xtensa/up/hal/built-in.a
boot-vectors-o := $(objtree)/src/arch/xtensa/up/xtos/_vectors.o
endif

ifeq ($(BUILD_BOOTLOADER),y)
do-module-copy = $(OBJCOPY) -O binary $(objtree)/src/platform/$(platform-folder)/base_module.o mod-$(fw-name).bin
do-module-insert = $(OBJCOPY) --add-section .module=mod-$(fw-name).bin \
		--set-section-flags .module=load,readonly sof-$(fw-name)
rimage-boot-bin = boot_ldr-$(fw-name)
else
do-module-copy =
do-module-insert =
rimage-boot-bin =
endif

ifneq ($(MEU_PATH),)
check-version := 12.0.0.1035
meu-version=$(shell $(MEU_PATH)/meu -ver | grep "Version:" | cut -d" " -f6)
greater-version=$(shell printf '%s\n' $(check-version) $(meu-version) | sort -Vr | head -n 1)
meu-offset=$(if $(filter $(check-version),$(greater-version)),1152,1088)

otc-private-key=$(srctree)/rimage/keys/otc_private_key.pem

do-rimage=$(objtree)/rimage/rimage -o sof-$(fw-name).ri -p sof-$(fw-name).ldc -m $(fw-name) $(rimage-boot-bin) sof-$(fw-name) -s $(meu-offset) -k $(otc-private-key)
do-meu=$(MEU_PATH)/meu -w ./ -s sof-$(fw-name) -key $(MEU_PRIVATE_KEY) -stp /usr/bin/openssl -f $(MEU_PATH)/generic_meu_conf.xml \
	-mnver 0.0.0.0 -o sof-$(fw-name).ri
else
do-rimage=$(objtree)/rimage/rimage -o sof-$(fw-name).ri -p sof-$(fw-name).ldc -m $(fw-name) $(rimage-boot-bin) sof-$(fw-name)
do-meu=
endif

# Build bootloader
quiet_cmd_linkbootldr_objects = AR      $@
      cmd_linkbootldr_objects = rm -rf $@; ${AR} rcsTP${KBUILD_ARFLAGS} $@ \
		$(objtree)/src/arch/xtensa/boot_entry.o \
		$(objtree)/src/arch/xtensa/boot_loader.o \
		$(boot-vectors-o) $(libreset-o-files) $(libhal-o-files)

quiet_cmd_linkbootldr = LD      $@
      cmd_linkbootldr = $(CC) $(KBUILD_LDFLAGS) $(KBUILD_CPPFLAGS) $(LINUXINCLUDE) \
		-T $(boot-lds-out) -o $@ boot_ldr-objects.a -lgcc

$(objtree)/boot_ldr-objects.a: sof FORCE
	$(call if_changed,linkbootldr_objects)

boot_ldr: boot_ldr-objects.a
	$(call if_changed,linkbootldr)

# Build firmware image
quiet_cmd_dumpbootldr = DUMP    boot_ldr
      cmd_dumpbootldr = cp boot_ldr boot_ldr-$(fw-name) ;\
	$(OBJCOPY) -O binary $(objtree)/src/platform/$(platform-folder)/boot_module.o mod-boot-$(fw-name).bin ;\
	$(OBJCOPY) --add-section .module=mod-boot-$(fw-name).bin \
		--set-section-flags .module=load,readonly boot_ldr-$(fw-name) ;\
	$(OBJCOPY) -O binary boot_ldr boot_ldr-$(fw-name).bin ;\
	$(OBJDUMP) -h -D boot_ldr > boot_ldr-$(fw-name).map ;\
	$(OBJDUMP) -S boot_ldr > boot_ldr-$(fw-name).lst ;\
	$(OBJDUMP) -D boot_ldr > boot_ldr-$(fw-name).dis

boot_ldr-objdump: boot_ldr
	$(call if_changed,dumpbootldr)

quiet_cmd_rimagesof = RIMAGE  sof-$(fw-name)
      cmd_rimagesof = cp sof sof-$(fw-name) ;\
	$(do-module-copy) ;\
	$(do-module-insert) ;\
	$(OBJDUMP) -S sof-$(fw-name) > sof-$(fw-name).lst ;\
	$(OBJDUMP) -D sof-$(fw-name) > sof-$(fw-name).dis ;\
	$(do-rimage);\
	$(do-meu)

bin: boot_ldr-objdump rimage
	$(call if_changed,rimagesof)

bin-clean:
	$(Q)$(MAKE) $(clean)=rimage
	$(Q)rm -fr mod-* sof-* boot_ldr boot_ldr-*
	$(Q)rm -fr *.a *.map *.dis *.lst *.bin
