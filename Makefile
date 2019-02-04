run: all
	$(MAKE) -C Build run

all: FirstStage SecondStage 

FirstStage:
	$(MAKE) -C BootLoader/FirstStage
SecondStage:
	$(MAKE) -C BootLoader/SecondStage

run:

clean:
	$(MAKE) -C Build/ clean
	$(MAKE) -C BootLoader/FirstStage clean
	$(MAKE) -C BootLoader/SecondStage clean

