run: all
	$(MAKE) -C Build debug

release: all
	$(MAKE) -C Build release

all: FirstStage SecondStage 

FirstStage:
	$(MAKE) -C Bootloader/FirstStage
SecondStage:
	$(MAKE) -C Bootloader/SecondStage

run:

clean:
	$(MAKE) -C Build/ clean
	$(MAKE) -C Bootloader/FirstStage clean
	$(MAKE) -C Bootloader/SecondStage clean

