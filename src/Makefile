
all: IIc.bin Liron.bin SoftSP-SSC.bin SoftSP-Grappler.bin SoftSP-KBOOHK.bin \
	SoftSP-Grappler_S1.bin SoftSP-Grappler_S2.bin SoftSP-Grappler_S3.bin \
	SoftSP-Grappler_S4.bin SoftSP-Grappler_S5.bin SoftSP-Grappler_S6.bin \
	SoftSP-Grappler_S7.bin

	md5sum --ignore-missing --check md5sums.txt
	@echo Done.

ALL_INCS=ca65_a2_strings.inc ca65_assert_branch_page.inc pc.boot.inc pc.boot.boottab.inc \
		pc.bootspace.inc pc.cread.inc pc.equates.inc \
		pc.main.inc pc.main.entry.inc pc.main.paramctab.inc pc.main.assignid.inc \
		pc.packet.inc pc.packet.sendonepack.inc pc.packet.preamble.inc pc.packet.waste.inc \
		pc.packet.markerr.inc pc.packet.receivepack.inc pc.packet.start2.inc \
		pc.packet.resetchain.inc pc.packet.enablechain.inc pc.packet.clrphases.inc \
		pc.packet.setxn0.inc pc.packet.shifttables.inc pc.packet.senddata.inc \
		pc.packet.divide7tables.inc pc.packet.auxptrinc.inc pc.packet.divide7.inc \
		pc.packet.precheck.inc pc.packet.setiwmode.inc pc.packet.waitiwmoff.inc \
		pc.packet.rcvcount.inc

# For Slot N, defines TheOff as "$N0"
SoftSP-Grappler_S%.o: SoftSP-Grappler.s $(ALL_INCS)
	ca65 --target apple2 -l "$@.list" -D TheOff="\$$$*0" -o "$@" "$<"

SoftSP-Grappler_S%.bin: SoftSP-Grappler_S%.o
	ld65 --config apple2-asm.cfg -o "$@" "$<"

%.o: %.s $(ALL_INCS)
	ca65 --target apple2 -l "$@.list" -o "$@" "$<"

%.bin: %.o
	ld65 --config apple2-asm.cfg -o "$@" "$<"

clean:
	rm -f *.bin *.o
