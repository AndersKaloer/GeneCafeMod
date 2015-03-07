all: 
	$(MAKE) -C src all
	$(MAKE) -C dts all

clean:
	$(MAKE) -C src clean
	$(MAKE) -C dts clean
