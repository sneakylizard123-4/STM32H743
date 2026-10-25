YOSYS ?= yosys
NEXTPNR ?= nextpnr-ice40
ICEPACK ?= icepack

all: top.bin

top.json: top.v
	$(YOSYS) -p "synth_ice40 -top top -json $@" top.v

top.asc: top.json top.pcf
	$(NEXTPNR) --up5k --package sg48 --json top.json --pcf top.pcf --asc $@

top.bin: top.asc
	$(ICEPACK) top.asc $@

clean:
	rm -f top.json top.asc top.bin

.PHONY: all clean
