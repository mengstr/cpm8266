	E5	= "\345"

	CPMCP	=	/usr/bin/cpmcp
	CPMRM	=	/usr/bin/cpmrm
	NEWDISK	=	printf '%*s' 256256 | tr ' ' $(E5) >

all:	DISK_A.DSK DISK_B.DSK DISK_C.DSK DISK_D.DSK DISK_E.DSK \
        DISK_F.DSK DISK_G.DSK DISK_H.DSK DISK_I.DSK DISK_J.DSK \
        DISK_K.DSK DISK_L.DSK DISK_M.DSK DISK_N.DSK DISK_O.DSK 

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

DISKS	=	A B C D E F G H I J K L M N O
$(foreach DSK,$(DISKS),$(eval $(call HANDLEDISK,$(DSK))))
        
clean:
	@rm -rf *.DSK
		