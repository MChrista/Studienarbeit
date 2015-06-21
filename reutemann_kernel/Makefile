#=============================================================================
#
# Makefile
#
#=============================================================================

SUBDIRS     = boot libkernel
SUBDIRS    += welcome timeoday tickdemo
SUBDIRS    += pmhello pgftdemo pgftdemo/tests

.PHONY: all subdirs $(SUBDIRS)
.SECONDARY:

all: subdirs

subdirs : $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

timeoday: boot
pmtimer: boot

.PHONY: clean
clean:
	@for d in $(SUBDIRS); \
	do \
	    $(MAKE) --directory=$$d clean; \
	done

