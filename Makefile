all: 
	$(MAKE) -C dev
	$(MAKE) -C daemon

flash:
	$(MAKE) -C dev flash

clean:
	$(MAKE) -C dev clean
	$(MAKE) -C daemon clean
