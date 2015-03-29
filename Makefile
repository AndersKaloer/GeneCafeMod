all: 
	$(MAKE) -C src all
	$(MAKE) -C dts all

install:
	cp roaster.service /lib/systemd/system/
	systemctl enable roaster.service

clean:
	$(MAKE) -C src clean
	$(MAKE) -C dts clean
