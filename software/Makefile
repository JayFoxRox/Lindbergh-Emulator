GDB=gdb -ex "handle SIGSEGV pass nostop noprint" --args
STRACE=strace -o logs/strace
APITRACE=apitrace trace

all:
	make -C fusefs
	make -C preload

clean:
	make -C fusefs clean
	make -C preload clean

#LD_DEBUG=files might come in handy
test: all
	#LD_DEBUG=all
#	LD_LIBRARY_PATH="./ld:." LD_PRELOAD="./ld/lindbergh.so" $(GDB) "/home/fox/Data/Transfer/lindy/All-Executables from Jackulus/JenTest_OR2_EnabledSecretMenu"
#	LD_LIBRARY_PATH="./ld:." LD_PRELOAD="./ld/lindbergh.so" $(GDB) "/home/fox/Data/Transfer/lindy/All-Executables from Jackulus/vt3_testmode" -t
	sudo chroot "./fs" /env LD_LIBRARY_PATH="/ld:." LD_PRELOAD="/ld/lindbergh.so" "/home/disk0/abc" -t
#	LD_LIBRARY_PATH="./ld:." LD_PRELOAD="./ld/lindbergh.so" $(GDB) "/home/fox/Data/Transfer/lindy/All-Executables from Jackulus/hummer_Master.elf" -t

# This mounts the necessary folders for the emulator chroot
mount:
	sudo mount /home/fox/Data/Development/Debian-Sarge-3.1/ ./fs/root -o bind
#	sudo mount / ./fs/root -o bind
	sudo mount "/home/fox/Data/Projects/The Firm/Lindbergh Replacement Kit/files/Lindbergh Games/DVP-0009s - Afterburner Climax/installed/home" ./fs/home -o bind
umount:
	sudo umount ./fs/root
	sudo umount ./fs/home
