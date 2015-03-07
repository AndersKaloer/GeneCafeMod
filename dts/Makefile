DTC = dtc
DTCFLAGS = -O dtb -b 0
SOURCES = BB-ROASTER-01-00A0.dts BB-ROASTERPWM-01-00A0.dts BB-SPI0-01-00A0.dts
OBJECTS = $(SOURCES:.dts=.dtbo)

all: $(SOURCES) $(OBJECTS)

%.dtbo: %.dts
	$(DTC) $(DTCFLAGS) -o $@ -@ $^

clean:
	-rm $(OBJECTS)
