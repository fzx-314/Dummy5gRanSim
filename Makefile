include $(RTE_SDK)/mk/rte.vars.mk
APP = Core_Ran


ifeq ($(RTE_SDK),)
$(error "Please define RTE_SDK environment variable")
endif

ifeq ($(RTE_TARGET),)
$(error "Please define RTE_TARGET")
endif

ifeq ($(CORE),1)
SRCS-y += upf.c
SRCS-y += tap.c
SRCS-y += thread.c
SRCS-y += poolCreation.c
SRCS-y += gtp.c
endif


ifeq ($(RAN),1)
SRCS-y += ranSimulator.c
SRCS-y += tap.c
SRCS-y += thread.c
SRCS-y += poolCreation.c
SRCS-y += gtp.c
endif

ifeq ($(PING),1)
SRCS-y += ranSimulator.c
SRCS-y += tap.c
SRCS-y += thread.c
SRCS-y += poolCreation.c
SRCS-y += gtp.c
CFLAG += -DPING
endif

CFLAGS += -g -w 

include $(RTE_SDK)/mk/rte.app.mk
build:
	@mkdir -p $@
.PHONY: clean
clean:
	rm -rf build/$(APP) build/$(APP)-static build/$(APP)-shared
	rm -rf *.o
	rm -rf build
	rm -rf _*
