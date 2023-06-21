all: build

AAS_BUILD_INPATH = src/
AAS_BUILD_FLAGS = -o:speed -out:comet

build:
	@odin build $(AAS_BUILD_INPATH) $(AAS_BUILD_FLAGS)


# odin build ./tools/aas/src -o:speed -out:./tools/aas/bin/aas.exe; ./tools/aas/bin/aas.exe ./tools/aas/test/test.aphel -debug -out:tools/aas/test/out.amg
