DEFINES+=PROJECT_CONF_H=\"project-conf.h\"
PROJECT_SOURCEFILES += sniffer-rdc.c netstack.c

CONTIKI_PROJECT = sniffer

all: $(CONTIKI_PROJECT) 

CONTIKI = ../..

### Add the hardware-specific sniffer driver sniffer-arch-$(TARGET).c to the
### list of sources or throw and error if it does not exist
ifeq ($(TARGET),)
  -include Makefile.target
endif

TARGET_DRIVER = $(wildcard sniffer-arch-$(TARGET).c)
ifneq ($(TARGET_DRIVER),)
  PROJECT_SOURCEFILES += $(TARGET_DRIVER)
else
  $(error target $(TARGET) not supported. See README.md for the list of supported targets)
endif

include $(CONTIKI)/Makefile.include
