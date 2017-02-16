# How to Bootstrap CP/M on a Classic S-100 System When No System Disk is Available

January 20, 2008
Richard A. Cini
http://www.classiccmp.org/cini
___

One of the greatest challenges in restoring a classic S-100 microcomputer system to full operation is finding known-good operating system disks in the proper configuration. This is not to say that this is the only challenge, but it’s probably one of the most difficult (on the surface) to overcome because original system disks are no longer widely available, notwithstanding the efforts of some to create and maintain archives of system disks. Don Maslin probably had the largest and most diverse collection of system disks which he would provide to anyone who asked for nominal cost. When Don passed away a few years ago, the archive unfortunately went with him. 

Unlike the IBM-PC standard to which manufacturers built their “clone” systems, there were no common hardware configurations across S-100 manufacturers because of the high level of customization that could be done with the S-100 platform. The high degree of hardware customization afforded to users presents a significant challenge to the system restorer. Without access to “virgin” system masters from the original manufacturer (a copy of CP/M which has not been modified or tweaked for use with a specific configuration but otherwise works with the hardware combination), one must locate a copy of CP/M with a configuration that matches the configuration of the system being restored, or employ alternative methods such as what is presented in this paper.

Not having a system disk has frustrated me for months in the restoration of an IMSAI system that I received as a donation in 2006. This how-to is as a result of the efforts of a single person who contacted me and offered to help me build a new floppy system from scratch; a process he went through just several months prior with nearly an identical configuration. 

Not to get ahead of myself, but not all CBIOSes are created equal. The base CP/M distribution has what’s called a “skeletal CBIOS” that the end user would customize for his hardware. As time progressed, peripheral manufacturers (primarily disk controller manufacturers) provided a template CBIOS to the end user (or at least included a listing of the routines required to be added to the user’s CBIOS). This made the customization process somewhat easier for the end user because the code was ready-made and tested. 

But, even a finished CBIOS doesn’t have all of the capabilities needed to pull a resurrection project like this off. A skeletal CBIOS usually includes only the minimum hardware support routines as required by CP/M. Nice features like a monitor or a disk formatting routine are either separate programs or not included at all and left to the user to write. 

The code for this project and the overall method was provided John Singleton, who provided a lot of guidance and hand-holding throughout the process. To the skeletal CBIOS from CompuPro (which I believe was written by Bill Godbout, but I’m sure there were other contributors), John added the monitor function, the routine to write CP/M to the diskette, and a disk formatting program, in addition to any required support subroutines.

It was this complete package that enabled me to get the project off the ground and be able to use my IMSAI. To John, I’m very thankful.

## Restoration of my IMSAI Floppy System

### Introduction

In 2006, I adopted a fully-loaded IMSAI system that was in very good physical and good operational condition. It came with lots of spare cards and 8” disks, and was configured as follows:

* IMSAI with a Z80 CPU card
* 48k of SRAM in three, 16k boards
* No terminal card (I added a Solid State Music 2p+2s card and configured it for both Altair and IMSAI compatibility).
* iCOM “Frugal Floppy” disk interface and a separate dual-8” floppy drive subsystem from SDS Systems with CP/M 1.4 disks. This is a completely TTL-based system - there is no LSI floppy controller chip such as the i8272 - and therefore requires very specific (and uncommon) software to run.

During the initial check-out of this system I wanted to archive the 200 or so 8” disks that came with the system, many of which included some interesting programs. It was during this phase that I discovered two things. First, the FD400 floppy drives that are part of the Frugal Floppy system were generally unreliable - out of four drives (which include working spares) there’s only one drive that works reliably. Maybe this was due to being shipped cross-country, but it didn’t much matter as they are operationally unreliable. So, straight diskette duplication before archiving was out unless I wanted to spend months swapping floppies in a single drive or archiving each disk file-by-file.

I considered using a great disk archiving program from Dave Dunfield, which consists of a small client program that’s uploaded to the target machine and which reads and transmits the disk data over a serial port to the host program running on a PC. This utility requires CP/M v.2.2 but unfortunately my system only came with v.1.4. I polled several mailing lists and the comp.os.cpm news group about obtaining a copy of a disk from someone. Based on the lack of feedback I received, I must be the only one with a working iCOM-based floppy system.

### Evaluating Alternatives

I became a little discouraged at the prospect of not having a floppy system for my IMSAI, so I embarked on a campaign of building a working floppy system around one of the other floppy cards I had acquired over time “just in case.” The 8” floppy system is huge, heavy and noisy so converting to a 5.25”-based floppy system was somewhat desirable.

At my disposal I had the following disk controllers: an SD Sales “Versafloppy”; a NorthStar MDS-2A; and a CompuPro Disk 1. All of these use or can be configured to use the more reliable (and more widely available) 5.25” floppy disk drive. 

I knew that the NorthStar controller worked because it had been used previously in my NorthStar system. The operating condition of the other two cards was “unknown” although expected to be working. The positive for the NorthStar controller was that I have a working Horizon system that I could use to build a new CP/M for the IMSAI. However, it was eliminated from consideration because this controller requires hard-sectored floppy disks, which are pretty rare, and I don’t have enough to support two working systems. 

The other two controllers are reasonably flexible, are designed around the i8272 LSI floppy controller from Intel and can handle both 8” and 5.25” low-density floppy disk drives. As a side note, high-density (1.2mb) 5.25” floppy disk drives can “look like” an 8” floppy drive to CP/M, widening the restoration possibilities. The electrical interface on the 5.25” HD drive is different in number of pins, but the recording density and spindle speed is the same (500kbps and 360 RPM, respectively) and it is capable of stepping the required number of tracks (77 for a standard 8” disk). It is this similarity that will prove to be very useful in order for this project to proceed. 

I opted to start with the CompuPro controller only because I was able to locate an 8” disk image for a CompuPro system with a Disk 1A controller. The Disk 1A is operationally very similar to the Disk 1 and thus the boot disk should work with the Disk 1. I planned on using the associated disk image tool to write the image onto a 5.25” HD floppy diskette and see if it would work. Even if converting the disk image worked, the CP/M on the disk image probably wouldn’t work because it had been customized for a different original system. Even so, I was prepared to give this method a try when help arrived.

### Help Arrives

With the efforts of scores of people interested in preserving the legacy of these “pre-PCs”, most if not all of the ingredients to perform this 8->5 conversion existed “in the Ether” but it wasn’t until a chance correspondence from another hobbyist (John Singleton) shined the light on the process that I was searching for. 

Our systems are very similar, but changes to my system would be needed. My system had only 48k of SRAM and the model system had a 64k CompuPro RAM17 board with the RAM at $F000 replaced with ROM. The 2k block of RAM at $F800 was used for variables and disk buffer space for the monitor ROM. 

The model system used a CompuPro Interfacer I serial card (8251 ACIA-based). My console card is UAR/T based but could be configured to look like the 8251 to the software using it.

In order to make my system look as much like the model system as possible, and at John’s suggestion, I located and purchased a RAM17 board from someone who was a former CompuPro dealer. The RAM17 is a 64k SRAM card with 32, 6116 (2k x 8) SRAM chips. Since the pinout of the 6116 is compatible with the 2716 EPROM, you can drop an EPROM right into a socket on the board and it’ll work perfectly.

It was this dealer’s last RAM17…finally some good fortune. The RAM17 was pretty expensive but worth it since I could replace 4 memory cards and an EPROM card with a single card. At this point, I removed any extraneous boards, resulting in a very simple system consisting of a CPU card, dual-serial card, 64k of RAM and the CompuPro Disk 1 floppy controller.

### The Goal and the Process 

The goal is to get a properly-configured copy of CP/M 2.2 onto a 5.25” diskette when no original system master exists. Broadly, the process is as follows:

0.	Locate and collect all of the required manuals and datasheets. I won’t elaborate on this below as a separate section, but a restoration project cannot be completed without access to the manuals and schematics for the hardware. There are many manual archives on the Internet, so start collecting documents before beginning this process. The hardware needs to be configured properly and working (obviously) for the rest of the process to work.
1.	Obtain or write a “monitor” program that is configured for the hardware. In addition to basic monitor commands (like dumping and editing memory and being able to “jump” to code), it should have the ability to receive Intel HEX-formatted files and save them to memory (HEX files contain memory location information and are self-verifying) and to cold- and warm-boot CP/M. I’ve found having the CP/M Customized BIOS (CBIOS) as part of the monitor convenient because of the link between the jump table and the CBIOS (this is described later).
2.	Validate that the host computer can communicate with the disk controller and the disk drives.
3.	Build and properly locate in memory a CP/M CBIOS and cold-start boot loader that matches the system hardware configuration and provides the essential hardware drivers for the CP/M BDOS (see point #1 above). Having the CP/M cold-start loader in EPROM will ease the job of moving the first CP/M programs to disk.
4.	Build and properly locate in memory CP/M.
5.	Upload the various parts to memory and write them out to a blank, formatted diskette using a PutCPM program.
6.	Copy the standard CP/M programs to the newly-created bootable diskette.

CP/M consists of four distinct parts that reside on a diskette in consecutive sectors occupying tracks 0 and 1. The core parts of CP/M are fixed in size for a given version and require no modification to accommodate various hardware configurations. The four parts are as follows:

* The cold-start/warm-start boot loader. This purpose of the boot loader is to load CP/M from diskette into high RAM and to transfer execution to the Console Command Processor. Most of the time, this loader resides on sector 1 of track 0 of a diskette, but it can also reside in EPROM. In the system configuration I’m working with, it resides in EPROM and is part of the monitor code John wrote. Doing it this way avoids having to maintain a separate bootstrap program and to write a separate program to copy the loader to disk. Further, the code can be configured in such a way that control is returned to the monitor after CP/M is loaded into memory from the disk. This capability makes it easier to initially move the base CP/M programs to the diskette.
* The Console Command Processor (CCP). This module is the “front end” of CP/M that handles user input (through the command prompt) and dispatches commands.
* The Basic Disk Operating System (BDOS). The BDOS is the “back end” of CP/M that provides the callable system services API. The BDOS is the application-level abstraction of the hardware drivers provided by the next module.
* The Customized Basic Input Output System (CBIOS). The CBIOS provides the low-level hardware drivers which act as the interface between the hardware itself and the system services provided to user application by the BDOS.

### Step 1: The Monitor

Although I already had a working monitor program from David Dunfield, John’s CBIOS had a monitor program which contained everything required: it can examine and modify memory, test RAM, perform direct port I/O (needed for testing the controller), and load Intel HEX-formatted files into memory. Further, it has disk-related tools to format diskettes and to load and write CP/M to and from the diskette. 

As alluded to above, the last steps in this process require that CP/M be loaded to memory but not run. The CP/M loader in your monitor program should not automatically jump to the CP/M entry point until the initial system disk build process has been completed.

The specific version of monitor program or where it was obtained is less important than its capabilities. So long as it provides basic monitor commands, one can add the required file uploading and disk interface features from code available on the Internet, in the original hardware documents, or written from scratch. Since I was trying to use John’s system as a model, it made sense to recycle as much of his code as possible to minimize potential problems and since I’m not that good at 8080 code. I could handle changing the console port stuff, but the disk code was beyond me.

These capabilities will be important to rebuild a working system using the method John created and which is presented below.

### Step 2: Validate the Hardware

I bought the CompuPro controller as working, but I had previously not been able to test it. The Disk 1 can accommodate both 8” and 5.25” floppy drives through a single 50-pin 0.1” grid connector, using jumpers to reassign certain control signals to the correct pins for each drive format.

The first step in verifying that the controller works and can communicate with the disk drive is to ensure that each is cabled and configured properly in accordance with the manual and as it applies to your particular situation. My configuration is as follows:

#### Cabling

8” drives use a 50-pin interface while 5.25” drives use a 34-pin interface. The electrical interfaces are designed in such a way that they overlap: pin 1 of the 34-pin interface is pin 16 of the 50-pin interface, etc. Thus, if you line up pin 34 of the board connector with pin 50 of the ribbon cable and check backwards for each signal line the signals will line-up perfectly. 

For the cable, I used a conventional 34-conductor flat ribbon cable to which I crimped a 50-pin female IDC connector on one end and a 34-pin female edge connector on the other, observing the pin 1 orientation. Since the number of conductors required for the 5.25” interface is less than the 8”, I crimped a bit of unused cable to the remaining empty pins so that all of the pins were filled.

Alternatively, one could take an internal SCSI-I cable (also a 50-conductor cable) and modify it to work by trimming the cable at one end and crimping the 34-pin edge connector. 

I build mine from scratch only because I had the parts and I needed a longer cable to reach my work bench from the IMSAI.

#### Jumpers/Switches on the Controller

The Disk 1 has several jumpers which move around the control signals to the correct drive interface pins depending on the drive type (shorthand, “8” or “5”). The controller’s DIP switches and pin jumpers were set as follows for my drive configuration:

```
Switch
12345678
S1:	1001000a0 
S2:	11001111

J11: “5”	J12: “5”	J13: “5”	J14: closed
J15: A-C	J16: B-C	J17: A-C
```

In this configuration, the drive motor that rotates the diskette will run continuously with a diskette inserted and the latch closed. In this configuration, if the controller is connected to a drive or drives which have the heads engaged all of the time (most if not all late-model 5.25” drives), there could be an increase in head and disk surface wear because of the constant movement of the disk surface over the head. One may be tempted to use the head_load signal from the controller to control the drive motor, but in many cases, the timing of the control signals and the time it takes the drive to reach the ready state prevents this from working properly.

There are four possible solutions to deal with this. First, accept the potential for additional wear. Second, open the drive latch when not accessing the disk. Third, configure the disk controller to put the motor signal under software control (and modify the CBIOS to activate the motor when needed). And, fourth, change the type of drive to one with a head load solenoid (which more closely mimics the operation of an 8” drive anyway).

The fourth option is the one I ultimately selected. I was originally using TEAC drives (FD-55GFR-7149) for my project because it’s what I had “in stock”. As a modern drive, it has fewer jumper configurations and no head load solenoid. Although it works, I was concerned about the long-term head and disk surface wear.

I discovered that there are a few substitute drives which have head load solenoids or are otherwise properly configurable: early models of the FD-55GFR, a Panasonic [insert model#], and the YE Data YE-380. There may be others, but these are the ones I’ve readily discovered. I happened to have access a YE drive as well so I tried both and ultimately selected the YE drive over the TEAC because the YE drive is closest to the 8” in operation and won’t cause excessive head or diskette wear.

The YE-380 drive was used in certain models of the IBM PC/AT (the other model used in the AT is an Alps drive which is more like the TEAC drive). The drives I used have manufacturing dates in mid-1986.

#### Jumpers on the Drive and Cabling

One of the more challenging aspects of using a modern 5.25” HD floppy drive with 30 year old hardware is that modern drives have signals on certain pins that have other functions on old controllers but don’t have the flexibility to be reconfigured to work. Pin 2 (density_select) and pin 34 (disk_change) are two examples of signals on modern drives which conflict with the pin definition on old controllers (head_load and drive_ready, respectively). In addition, older drives sometimes have additional jumpers to set various features. The IBM PC and compatibles used fewer features or options so over time drives were built without these jumpers. Try to obtain the technical manual for whatever drive you use so you know what jumpers and options you have.

I searched for a while for the manual for this drive but it doesn’t seem to be available on the Internet. The modification below was provided by a fellow hobbyist and friend who used YE drives to rehabilitate his CompuPro system with a similar 8”->5” conversion. 	

The controller outputs the head_load signal on pin 2 and we don’t want that signal transition to inadvertently change the recording density of the drive. There is no jumper for pin 2 on the YE-380 so I used a low-tech way to prevent this conflict - a piece of clear cellophane tape placed over pin 2 of the drive edge connector isolates it from the cable. Another option would be to use a thin piece of paper folded over the edge of the PC board, providing isolation without the adhesive.

There is also no jumper to change the definition of pin 34 from disk_change to drive_ready (which happens to be on pin 4 of this model of drive). Thus, a minor modification to the drive PCB is required. The modification involves cutting the trace on the PCB leading to pin 34 and connecting pin 34 to the RY pad closest to the rear edge of the board (the RY jumper is unpopulated and is above the power connector). I made the modification reversible by wiring a 3-pin header to the RY pad, pin 34, and the trace leading to pin 34 and using a jumper block to select the pin’s function.

Next, the jumper that controls when the heads engage should be changed from HM (head-on-motor) to HS (head-on-select) so that it engages at the right time. 

Finally, each drive should be strapped for the correct drive select. Unlike the PC/AT (where all drives are strapped for D1 and the cable twist fixes everything), straight-through cables are used, so each drive has to be strapped for its proper drive select.

#### Testing the Drives and Controller

The drives I used for this project are “working pulls” from other machines (I always keep a stash of random hardware from older machines “just in case”). The drives are PC-compatible floppy drives so they can be connected to a regular PC to confirm their operation before being used for a restoration project. 

The host-to-drive interface also has to be tested. This will test communication between the host and the controller, and from the controller to the drive (which has already been confirmed to be working). Small test programs can be written and uploaded to memory using the HEX file loader in the monitor, or if the monitor program contains direct I/O commands, those can be used to communicate directly with the controller.

The monitor I used had direct I/O capability so I used this capability to send commands directly to the floppy controller (located at ports $C0-C1). Most commands are multiple-byte commands, but an easy one to use is the “seek” command. Seek is a three-byte command: $0F $XX, 0 (where “XX” is the track number in hexadecimal). If everything is connected properly and the hardware works, the disk head carriage should move to the appropriate track. Sending the command $0F, 0, 0 will “home” the heads. The 8272 data sheet provides a listing of some of the other commands available – “get master status” is another easy one to try (which should return $80 if all is well).

If the hardware passes this test, then building the software part of the system can proceed. If it does not work, troubleshooting will be necessary until controller-to-drive communications can be confirmed - without that, the software part won’t matter. Also, keep in mind that untested features of the drive and controller will be in doubt until you run software which operates those features.

### Step 3: Build and Locate the CBIOS

The standard approach for generating a new CP/M configuration from a system master is outlined in the CP/M Alteration Guide, but it unfortunately assumes the existence of a master system disk. 

As previously mentioned, the basic CBIOS contains the base hardware drivers for booting the system, communicating with the console, paper tape reader/punch, line printer (if so configured), and interfacing with the disk controller. Normally these routines are stored on disk and loaded by the cold-start loader along with the rest of CP/M. The CBIOS has two parts - it starts with a table of JMP instructions to the included routines and the routines themselves. The table always refers to the same 17 CBIOS routines and the jumps are always in the same order. Because of the way a normal CP/M system is built (consecutively on disk and in RAM memory), CP/M always knows the location of the jump table.
 
In the configuration I used, the CBIOS is in EPROM because the controller code is part of the monitor in EPROM. But the jump table is loaded into RAM by the cold-start loader. If the CBIOS exists in EPROM but the jump table is to be loaded into RAM by the loader, how do we get this to work? The magic is this: when CP/M is saved to disk for the first time, CP/M is located in memory such that only the code up to the end of the jump table is written to disk, leaving the remainder of the CBIOS in EPROM. But the CBIOS table must be assembled with the monitor, so its table of jumps to monitor code will be correct. The illustration later on will make this clearer.

The memory configuration in this system is somewhat unique because it is configured with a “hole” in memory (i.e., RAM below and above EPROM). RAM above the EPROM is used for monitor variables and disk buffer storage, thus maximizing the memory available to CP/M as transient program memory. Here’s what the system memory map looks like before considering CP/M:

```
+--------------+
+	RAM		   +	2k@ $F800 (buffers, vars, etc.)
+--------------+
+	EPROM      +	2k@ $F000 (monitor, CBIOS)
+--------------+
~		 	   ~
~	RAM		   ~	(60k)
~		 	   ~
+--------------+	0
```

The source file for the monitor EPROM contains the CBIOS jump table with an ORG statement locating it at the right place in RAM memory. This is done so that the HEX file has the correct memory address and the jump table contains the correct target addresses. When the monitor is cross-assembled, the resulting jump table can be split out of the HEX file and loaded into RAM memory separately. This is part of the magic.

The Location Table in the Alteration Guide indicates at what address the CBIOS jump table is to be located based on the memory size (in increments of 1K hex or 1024 decimal) of the system. This table should be located as high in memory as possible while not overlapping the monitor located at $F000. 

Looking at the table, a 62k system configuration has the CBIOS jump table located at $F200 (which is clearly in the middle of the monitor and won’t work). A 61k system configuration locates this table at $EE00 - far enough below the monitor that there will be no conflicts. The jump table ends at $EE00 + (18*3) -1 or $EE35. 

In addition to the location of the CBIOS, the Location Table also indicates where the CCP and BDOS are to be located. Working backwards from the system size, the table indicates that for a 61k system, the CCP will be located at $D800 and the BDOS will be located at $E000. 

At the end of this step, the memory map looks as follows:

```
+==============+	$FFFF
+	RAM		   +		2k (RAM buffers, vars, etc.)
+--------------+	$F800
+	ROM		   +		2k (monitor, CBIOS)
+--------------+	$F000
+     RAM	   +		unused RAM/inaccessible
+--------------+	$EE36
+ 	RAM		   + 		CBIOS jump table
+--------------+	$EE00 
+ 	RAM		   + 		BDOS
+--------------+	$E000
+	RAM		   +  	    CCP
+--------------+	$D800
~			   ~
~	RAM		   ~	TPA (53.8k)
~			   ~
+--------------+	$0100
+	RAM		   +		CP/M SysVars
+==============+	0
```

I used the TASM cross-assembler for the PC to build the monitor/CBIOS/jump table. The assembler can produce an Intel-formatted HEX file that is split into two pieces - one for the jump table and one to be used to burn an EPROM for the monitor and CBIOS. The jump table file is later merged with the HEX file containing the CP/M system compiled in Step 4 of this process. 

### Step 4: Build and Locate CP/M

The memory map above indicates all of the address information that is needed in order to build the CCP and BDOS pieces of CP/M, but we need the code for CP/M and some way to compile it.

The process utilized here is very different from the procedures in the Digital Research (DRI) manuals, primarily because the manuals are written from the perspective of having access to new system disks from system builders – it assumes that both DRI and the system builder are in business. 

The DRI System Alteration Guide identifies the exact procedure one would use to configuring a system from scratch when new disks were available. The process involves using the MOVCPM utility provided by DRI to locate the BDOS and CCP as object files which are then merged in memory with the desired CBIOS. This new memory image is then saved to disk using the SYSGEN utility. So that the reader has a complete view of the process, it is suggested that you download from the Internet a of the DRI manual and familiarize yourself with the DRI process before proceeding.

As I didn’t have access to new system disks (both DRI and ComprPro have long ceased to exist), I’m rebuilding CP/M from source code. There is a substantial CP/M source code archive located at "the unofficial CP/M archive" at http://www.cpm.z80.de/source.html (which has an agreement with Lineo, the current owners of CP/M, to have this archive of CP/M products for personal use). About half-way down the page, there is a link to the "source code" for CP/M 2.2. It’s not the original DRI source code, but rather a decompilation of a memory image that has been made recompilable. The archive contains both 8080- and Z80-nmemonic source files. 

The 8080 code is designed to be built with Digital Research's assembler, ASM, a CP/M-based assembler distributed as part of CP/M that compiles only 8080 code. Thus, I worked with the 8080-target code from the above archive; 8080 object code runs on both 8080 and Z80-based systems. The only change to the source file that should be required is to define the proper starting location for the CCP, and should match the CCP value calculated in Step 3.

I did not have ready access to another CP/M machine to build a new system so I turned to another useful tool - an emulator running under MS-DOS - to complete this part of the task. I used the MS-DOS-based MyZ80 emulator by Simeon Cran (www.znode51.de/specials/myz80.htm and mirrored elsewhere) that emulates a Z80-based computer running CP/M 2.2.

I imported the modified source code for CP/M into the emulation environment using the emulator’s built-in tools, assembled the code using ASM, and exported the resulting Intel-formatted HEX file back to the host PC environment. This file is merged with the CBIOS jump table file to produce a single HEX file containing a complete CP/M working set that will be uploaded to memory and written to disk in Step 5 of this process.

### Step 5: Final EPROM Changes and Create New Bootable Diskette

The final step of this process can be broken down into several steps outlined below. The monitor program from John already had some key routines built in that are critical for the final phase of this project -- the ability to format blank diskettes, to copy CP/M from memory to a diskette, and to cold-boot CP/M from a diskette.

Format blank diskettes. The monitor program used contains a small routine to send the proper commands to the disk controller in order to format blank disks. The sector fill byte is 0xE5 and the format is 77 tracks of 26, 128-byte sectors per track.

Diskette formats can vary widely and were customizable by the manufacturer of the disk controller. So, CP/M didn’t come with a “format” program – it was up to the system OEM to provide it. The monitor I used had a small formatting program built it which basically runs in a loop counting tracks and sectors and writing a 128-byte buffer filled with the value $E5 to the disk.

The format of 77/26/128 (single-density, double-side, FM recording method) was selected because it appears most like the original 8” format (which was SSSD). The format program was included in the monitor because you cannot use an unformatted disk with CP/M and one cannot assume that another CP/M system is available in order to format disks. It’s much easier to format a disk using a transient program than having to return to the monitor to do it, so eventually I will write a stand-alone formatting program.

Create routine to put CP/M on a blank diskette. Again, the monitor program I used contains a routine that saves a specified region of memory to the disk (the Alteration Guide calls this “PutCPM”). The 8272 has a multi-sector write DMA mode which takes as parameters the starting memory location, the number of sectors to write and the starting track and sector as parameters and uses that information to write the area of memory to disk. For this system a total of 45 sectors are to be written to disk ($EE35-$D800+1 = 5686d/128 bytes/sector = 44.4 sectors). The sectors are written to disk in two parts - 26 sectors (all of Track 0) and 19 sectors (of Track 1). There’s the magic in action - the CBIOS jump table is written to disk during the PutCPM call.

The monitor EPROM also contains a corresponding GetCPM routine which uses a multi-sector read call to re-load CP/M from the boot tracks during the cold-start or warm-start routines. Both of these routines are standard parts of the CBIOS template.

Load the CP/M image to memory. Once the GetCPM/PutCPM routines have been modified to load and save the correct number of sectors, it’s time to load CP/M into memory and write it to disk. The monitor’s built-in HEX file loader is used to load the CP/M HEX file into memory. Again, a HEX file contains address location information, so the memory addresses can be discontinuous. This file contains all of the critical parts of CP/M that are to be saved to disk using the PutCPM program.

Move CP/M to diskette. Using the PutCPM program in the monitor EPROM, the range of memory containing CP/M is written to Tracks 0 and 1 of a blank formatted diskette.

If all goes well, jumping to the cold-start loader located in EPROM should read CP/M from disk and jump to the CCP that would put the “A>” prompt on the screen. If this occurs, congratulations -- a new CP/M disk has been created!

This new diskette has only CP/M on it. It does not include the transient programs that would normally be included with a commercial CP/M distribution. As with the CP/M source code, these programs are likewise available on the Internet at the CP/M archive site and other archive sites. The next task is to use various tools at our disposal to move these programs onto the new master disk just created.

Step 6: Move the “Base CP/M” Transient Programs to the Master Disk

We have just created a new CP/M master diskette for our system but it contains no transient programs. If you do “DIR” at the prompt, you will get a “No File” message, indicating that the disk is empty.

The “standard” CP/M distribution included 11 transient programs and a library file which should be transferred to this master disk:

* ASM - an 8080-target assembler
* DDT - Dynamic Debugging Tool (a code debugger)
* DUMP - dump a program to the console in HEX format
* ED - a text editor
* LOAD - make a COM file from a HEX file
* MOVCPM - relocate and execute a CP/M system for a new memory size
* PIP - Peripheral Interchange Program (copies files among devices)
* STAT - disk and file statistics
* SUBMIT - execute a SUB batch file
* SYSGEN - write a copy of CP/M from memory to a new diskette
* XSUB - the “extended” SUBMIT utility
* DISKDEF.LIB - disk definition library

In order to get these standard transient programs onto the new master diskette, I used the process described below. This process relies on the HEX loader and CP/M cold-start loader in the monitor EPROM. The cold-start loader needs to be modified to return to the monitor at the end of the bootstrap process rather than jumping to the CCP. It’s this linkage back to the monitor that enables us to move the initial files to the diskette.

* Convert the transient COM files downloaded from the Internet into Intel HEX files using the “bin2hex” utility (widely available through a Google search, but I downloaded mine from Keil Labs). The CP/M transient program loader loads COM programs to RAM starting at location $100. When converting these programs to HEX, a command line parameter “-o256” should be added so that the HEX file targeted at address 0x100 rather than 0 (the default).
* Start up the monitor and load CP/M into memory using the monitor’s cold-boot command (my monitor uses Control-C to load CP/M). When this is done, the system should return to the monitor prompt. As mentioned in the monitor section above, preventing the automatic JMP to CP/M returns control to the user and saves some time.
* Upload the HEX file to memory using the terminal (I use an old Compaq laptop as my terminal). Moving HEX files is more reliable than moving binary files as the HEX format is self-verifying (with checksums) and explicitly target specific memory. Binary files have no checksums and no obvious memory location information.

At this point, both CP/M and the program that is to be written to disk have been loaded into memory. From the monitor prompt start CP/M by jumping to the address CCP+$03 (in my system, that address is $D803). The CP/M prompt should appear on the terminal screen.

To save the transient program from memory to disk the CP/M “SAVE” internal command can be used. SAVE requires two parameters: the number of 256-byte blocks to save and the file name. When calculating the number of blocks, round up to the nearest block.

For example, the program STAT is 5,248 bytes in size. 5248/256 = 20.5. Thus, the save command would be:

```
A>SAVE 21 STAT.COM
```

Finally, return to the monitor from CP/M by typing Control-C (which automatically re-loads CP/M from diskette and, because the loader doesn’t jump to CP/M, returns us to the monitor) and repeat the process for each file. When this process has been completed, the monitor can again be modified to jump to CCP on a cold- or warm-start rather than jumping back to the monitor. 

Once the base CP/M programs exist on diskette, a different process can be used to move other programs onto the diskette (using ED, the editor, to capture the HEX file and save it to disk; LOAD to load the saved HEX file into memory as a binary file; and SAVE as was done above).

Now that a new master disk exists, make several copies of it should be made. With MS-DOS, this was fairly easy using Diskcopy, but the base CP/M distribution does not provide such a tool. The CP/M User Group Archive has a “diskdupe” utility, but without that, the system tracks and the user applications have to be copied separately using SYSGEN and PIP utility:

```
A>SYSGEN B:	(copies the system tracks)
A>PIP B:*.*=A:*.*	(copies files from A to B)
```

### Conclusion
	
This has been an interesting and rewarding process which has restored my system and increased its usability greatly. Now that I’ve gone through this process, I may attempt to generate a CP/M 2.2 disk for the original 8” system and see if I can use that new system master to archive the original 8” disks.
