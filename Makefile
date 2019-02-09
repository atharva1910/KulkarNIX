run: all
	$(MAKE) -C Build run

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

