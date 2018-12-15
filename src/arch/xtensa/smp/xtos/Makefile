# Builds .o file for each level from single the same source files
# TODO should there be level 6 for apl+ ?
levels := 2 3 4 5
level-handler-outputs := $(foreach lvl,$(levels),xlevel$(lvl)-handler.o)
$(foreach lvl,$(levels),$(eval AFLAGS_xlevel$(lvl)-handler.o=-D_INTERRUPT_LEVEL=$(lvl)))

$(addprefix $(obj)/,$(level-handler-outputs)): $(addprefix  $(src)/,int-handler.S) FORCE
	$(call if_changed_rule,as_o_S)

level-vector-outputs := $(foreach lvl,$(levels),xlevel$(lvl)-vector.o)
$(foreach lvl,$(levels),$(eval AFLAGS_xlevel$(lvl)-vector.o=-D_INTERRUPT_LEVEL=$(lvl)))

$(addprefix $(obj)/,$(level-vector-outputs)): $(addprefix  $(src)/,int-vector.S) FORCE
	$(call if_changed_rule,as_o_S)

level-initlevel-outputs := $(foreach lvl,$(levels),xlevel$(lvl)-initlevel.o)
$(foreach lvl,$(levels),$(eval AFLAGS_xlevel$(lvl)-initlevel.o=-D_INTERRUPT_LEVEL=$(lvl)))

$(addprefix $(obj)/,$(level-initlevel-outputs)): $(addprefix  $(src)/,int-initlevel.S) FORCE
	$(call if_changed_rule,as_o_S)

obj-y += $(level-handler-outputs) $(level-vector-outputs) $(level-initlevel-outputs)

vector-defs = \
	-D__SPLIT__vector \
	-D__SPLIT__handler \
	-D__SPLIT__user \
	-D__SPLIT__level1int \
	-D__SPLIT__level2 \
	-D__SPLIT__level3 \
	-D__SPLIT__level4 \
	-D__SPLIT__level5

apl-or-above := $(CONFIG_APOLLOLAKE) \
	$(CONFIG_CANNONLAKE) \
	$(CONFIG_SUECREEK) \
	$(CONFIG_ICELAKE)

ifneq ($(strip $(apl-or-above)),)
vector-defs += -D__SPLIT__level6
endif

ccflags-y += $(vector-defs)
asflags-y += $(vector-defs)

xtos-objects = \
	core-restore.o \
	core-save.o \
	core-shutoff.o \
	double-vector.o \
	debug-vector.o \
	xea1/exc-alloca-handler.o \
	xea1/exc-c-wrapper-handler.o \
	xea2/exc-c-wrapper-handler.o \
	xea1/exc-return.o \
	xea2/exc-return.o \
	exc-sethandler.o \
	exc-syscall-handler.o \
	exc-table.o \
	exc-unhandled.o \
	interrupt-table.o \
	int-sethandler.o \
	xea1/intlevel-restore.o \
	xea2/intlevel-restore.o \
	xea1/intlevel-setmin.o \
	xea2/intlevel-setmin.o \
	xea1/intlevel-set.o \
	xea2/intlevel-set.o \
	xea1/int-lowpri-dispatcher.o \
	xea2/int-lowpri-dispatcher.o \
	ints-off.o \
	ints-on.o \
	kernel-vector.o \
	memep-enable.o \
	memep-initrams.o \
	memerror-vector.o \
	nmi-vector.o \
	xea2/reloc-vectors.o \
	user-vector.o \
	xea1/window-vectors.o \
	xea2/window-vectors.o

obj-y += $(xtos-objects)
