E5		= "\345"

CPMCP	=	/usr/bin/cpmcp
CPMRM	=	/usr/bin/cpmrm
NEWDISK	=	printf '%*s' 256256 | tr ' ' $(E5) >
DISKS	=	A B C D E F G H I J K L M N O

DISKTARGETS=$(foreach DSK,$(DISKS), DISK_$(DSK).DSK )

all : $(DISKTARGETS)

define HANDLEUNDISK =
undisk$(1):
	@echo "[UNDISK] $(1)"
	@mkdir -p tmp
	@rm -f tmp/*.*
	@$(CPMCP) DISK_$(1).DSK 0:*.* tmp/
endef

$(foreach DSK,$(DISKS),$(eval $(call HANDLEUNDISK,$(DSK))))


define HANDLEDISK =
DISK_$(1).DSK: $(wildcard $(1)/*.*)
	@echo "[MAKEDISK] DISK_$(1).DSK"
	@$(NEWDISK) DISK_$(1).DSK
	@touch -t 197001010000 DISK_$(1).DSK
	@touch $(1)/DUMMY.FIL
	@$(CPMCP) DISK_$(1).DSK $(1)/*.* 0:
	@touch DISK_$(1).DSK
	@rm $(1)/DUMMY.FIL
	@$(CPMRM) DISK_$(1).DSK 0:DUMMY.FIL
endef

$(foreach DSK,$(DISKS),$(eval $(call HANDLEDISK,$(DSK))))

clean:
	@rm -rf *~
	@rm -rf $(DISKTARGETS)
	@rm -rf tmp
