VERSION := 1
PATCHLEVEL := 1
SUBLEVEL := 0
NAME := "Sound Open Firmware"

PROJECT_SHORT_NAME := SOF
PROJECT_BINARY_NAME := sof

MAKEFLAGS += --no-builtin-rules --no-builtin-variables --include-dir=$(CURDIR)

include scripts/Makefile.include

PROJECT_PHONY =

objs-y		:= src/

sof-dirs	:= $(patsubst %/,%,$(filter %/, $(objs-y)))
sof-deps	:= $(patsubst %/, %/built-in.a, $(objs-y))

# Link final binary
sof-link: $(sof-deps) FORCE
	${AR} rcsTP${KBUILD_ARFLAGS} built-in.a	$(sof-deps)

quiet_cmd_linksof = LD $@
      cmd_linksof = $(CC) $(LDFLAGS) -o $@ built-in.a

sof: sof-link FORCE
	$(call if_changed,linksof)

# Prevent implicit rules for our dirs
$(sort $(sof-deps)): $(sof-dirs) ;

# Make our top dirs depend on kbuild preparations
PROJECT_PHONY += $(sof-dirs)
$(sof-dirs): buildsystem-prepare
	$(Q)$(MAKE) $(build)=$@ need-builtin=1

# Additional cleaning for project
PROJECT_CLEAN_FILES := $(PROJECT_BINARY_NAME)

project-clean-dirs := $(addprefix _project_clean_, $(sof-dirs))

PROJECT_PHONY += $(project-clean-dirs)
$(project-clean-dirs):
	$(Q)$(MAKE) $(clean)=$(patsubst _project_clean_%,%,$@)

project-clean: rm-files := $(wildcard $(PROJECT_CLEAN_FILES))
project-clean: $(project-clean-dirs)
	$(call cmd,rmfiles)

clean: project-clean

# Kbuild needs .PHONY targets in PHONY variable
PHONY += $(PROJECT_PHONY)
.PHONY: $(PROJECT_PHONY)
