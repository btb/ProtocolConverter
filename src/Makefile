
all: IIc.bin Liron.bin
	md5sum --ignore-missing --check md5sums.txt
	@echo Done.

%.o: %.s ca65_a2_strings.inc pc.boot.inc pc.boot.boottab.inc \
		pc.bootspace.inc pc.cread.inc pc.equates.inc \
		pc.main.inc pc.main.entry.inc pc.main.paramctab.inc pc.main.assignid.inc \
		pc.packet.inc pc.packet.sendonepack.inc pc.packet.preamble.inc pc.packet.waste.inc \
		pc.packet.markerr.inc pc.packet.receivepack.inc pc.packet.start2.inc \
		pc.packet.resetchain.inc pc.packet.enablechain.inc pc.packet.clrphases.inc \
		pc.packet.setxn0.inc pc.packet.shifttables.inc pc.packet.senddata.inc \
		pc.packet.divide7tables.inc pc.packet.auxptrinc.inc pc.packet.divide7.inc \
		pc.packet.precheck.inc pc.packet.setiwmode.inc pc.packet.waitiwmoff.inc \
		pc.packet.rcvcount.inc


	ca65 --target apple2 -l "$@.list" -o "$@" "$<"

%.bin: %.o
	ld65 --config apple2-asm.cfg -o "$@" "$<"

clean:
	rm -f *.bin *.o
