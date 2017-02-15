#! /usr/bin/gawk -f

# chkihex.awk
#
# check intel-hex file
# 2012/10/09 K.Takesita
#
# Intel HEX format
# http://en.wikipedia.org/wiki/Intel_HEX
#
# :bbaaaarrdddd..ddcc
# bb Bytecount
# aaaa Address
# rr Recordtype
# dd.. Data
# cc Checksum
#
BEGIN{
	H2D[0]=0;H2D[1]=1;H2D[2]=2;H2D[3]=3;
	H2D[4]=4;H2D[5]=5;H2D[6]=6;H2D[7]=7;
	H2D[8]=8;H2D[9]=9;
	H2D["A"]=10;H2D["B"]=11;H2D["C"]=12;H2D["D"]=13;H2D["E"]=14;H2D["F"]=15;
	H2D["a"]=10;H2D["b"]=11;H2D["c"]=12;H2D["d"]=13;H2D["e"]=14;H2D["f"]=15;

	D2H[0]=0;D2H[1]=1;D2H[2]=2;D2H[3]=3;
	D2H[4]=4;D2H[5]=5;D2H[6]=6;D2H[7]=7;
	D2H[8]=8;D2H[9]=9;
	D2H[10]="A";D2H[11]="B";D2H[12]="C";D2H[13]="D";D2H[14]="E";D2H[15]="F";
}
#
$0 !~/^:[0-9A-F]+$/ { # unmatch
	printf("%d ; Unmatch Intel-HEX format\n",NR); print $0;
	next;
}
{
	Bytecount=substr($0,2,2);
	Address=substr($0,4,4);
	AddressH=substr(Address,1,2);
	AddressL=substr(Address,3,2);
	Recordtype=substr($0,8,2);
	Datalength=hex2dec(Bytecount);

	if (length(Recordtype)<2) {
		printf("%d ; Recordtype too short\n",NR); print $0;
		next;
	}
	if (length(Address)<4) {
		printf("%d ; Address too short\n",NR); print $0;
		next;
	}
	if (length(Bytecount)<2) {
		printf("%d ; Bytecount too short\n",NR); print $0;
		next;
	}

	sum = (Datalength + hex2dec(AddressH) + hex2dec(AddressL) + hex2dec(Recordtype))%256;

	if (Recordtype=="05") {
		printf("%d ; SLA\n",NR); print $0;
		# NOCHECK
		next;
	}
	if (Recordtype=="04") {
		printf("%d ; ELA\n",NR); print $0;
		# NOCHECK
		next;
	}
	if (Recordtype=="03") {
		printf("%d ; SSA\n",NR); print $0;
		# NOCHECK
		next;
	}
	if (Recordtype=="02") {
		printf("%d ; ESA\n",NR); print $0;
		# NOCHECK
		next;
	}
	if (Recordtype=="01") {	# EOF
		printf("%d ; EOF address= %s\n",NR,Address);
		if (length($0)<11) {
			printf("%d ; EOF record too short\n",NR); print $0;
			next;
		}
		if (length($0)>11) {
			printf("%d ; EOF record too long\n",NR); print $0;
			next;
		}
		Checksum=substr($0,10,2);
		if (hex2dec(Checksum)!=compl2(sum)) {
			printf("%d ; EOF Checksum unmatch\n",NR); print $0;
			next;
		}
		next;
		}
	if (Recordtype!="00") {
		printf("%d ; Illegal Recordtype\n",NR); print $0;
		next;
	}

	if (Datalength!=16 && Datalength!=32) {
		## printf("%d ; Warning, Usual Bytecount?\n",NR); print $0;
	}

	# check data
	for(i=0;i<Datalength;i++) {
		d=substr($0,10+i*2,2);
		if (length(d)<2) {
			printf("%d ; Data too short\n",NR); print $0;
			next;
		}
		sum = (sum + hex2dec(d))%256;
	}

	Checksum=substr($0,10+Datalength*2,2);
	if (length(Checksum)<2) {
		printf("%d ; Checksum too short\n",NR); print $0;
		next;
	}
	if (length($0)>11+Datalength*2) {
		printf("%d ; Record too long\n",NR); print $0;
		next;
	}
	if (hex2dec(Checksum)!=compl2(sum)) {
		printf("%d ; Checksum unmatch\n",NR); print $0;
		printf("Record sum= %02X ,expected value= %02X\n",sum,compl2(sum));
		next;
	}

	# passed.
}
END{
	print "END." ;
}
#
func chkrtype(r ,rr) {
	rr=r+0;
	if (rr>5) rr=-1;
	return rr;
}
#
func compl2(num ,t) {
	t=num%256;
	return t?256-t:0;
}

func hex2dec(str ,i,a) {
	a=0;
	for(i=1;i<=length(str);i++) a= a*16 + H2D[substr(str,i,1)];
	return a;
}
