CC = gcc
CFLAGS = -Wall -g
#CFLAGS += -DDEBUG
LDFLAGS = -lrt -pthread
SOURCES = roaster.c lcd.c pwm.c adc.c max31855.c btn.c logserver.c event_queue.c sensor.c actuator_lcd.c actuator_m150.c actuator_logserver.c sensor_knop.c sensor_thermocouple.c sensor_button.c
OBJECTS = $(SOURCES:.c=.o)
EXECUTABLE = roaster
DEV_TREE1 = BB-ROASTER-01-00A0.dts
DEV_TREE1_OUT = $(DEV_TREE1:.dts=.dtbo)
DEV_TREE2 = BB-ROASTERPWM-01-00A0.dts
DEV_TREE2_OUT = $(DEV_TREE2:.dts=.dtbo)

all: $(SOURCES) $(EXECUTABLE) $(DEV_TREE1_OUT) $(DEV_TREE2_OUT)

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

dtc: $(DEV_TREE1_OUT) $(DEV_TREE2_OUT)

$(DEV_TREE1_OUT):
	dtc -O dtb -o $(DEV_TREE1_OUT) -b 0 -@ $(DEV_TREE1)
$(DEV_TREE2_OUT):
	dtc -O dtb -o $(DEV_TREE2_OUT) -b 0 -@ $(DEV_TREE2)

clean:
	-rm $(EXECUTABLE)
	-rm $(OBJECTS)
	-rm $(DEV_TREE1_OUT)
	-rm $(DEV_TREE2_OUT)
