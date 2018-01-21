CC = gcc
CFLAGS = -Wall -g
#CFLAGS += -DDEBUG
LDFLAGS = -lrt -pthread
SOURCES = roaster.c lcd.c pwm.c adc.c max31855.c btn.c logserver.c event_queue.c sensor.c actuator_lcd.c actuator_m150.c actuator_logserver.c actuator_filelogger.c sensor_knop.c sensor_thermocouple.c sensor_button.c
OBJECTS = $(SOURCES:.c=.o)
EXECUTABLE = ../roaster

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $^ -o $@

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	-rm $(EXECUTABLE)
	-rm $(OBJECTS)
