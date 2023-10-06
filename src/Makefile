
all: IIc.bin Liron.bin
	md5sum --ignore-missing --check md5sums.txt
	@echo Done.

%.o: %.s ca65_a2_strings.inc pc.boot.inc pc.bootspace.inc pc.cread.inc pc.equates.inc pc.main.inc pc.packet.inc
	ca65 --target apple2 -l "$@.list" -o "$@" "$<"

%.bin: %.o
	ld65 --config apple2-asm.cfg -o "$@" "$<"

clean:
	rm -f *.bin *.o
