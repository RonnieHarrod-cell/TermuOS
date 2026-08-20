DG = doomgeneric-src/doomgeneric

CFLAGS = -static -nostdlib -no-pie -ffreestanding \
  -fno-stack-protector -fno-asynchronous-unwind-tables \
  -fcf-protection=none -O2 \
  -fno-builtin \
  -D_FORTIFY_SOURCE=0 \
  -I$(DG)

# List every .c, then delete the ones we do not want
ALL_C := $(wildcard $(DG)/*.c)

ENG_C := $(ALL_C)
ENG_C := $(filter-out $(DG)/doomgeneric_allegro.c,$(ENG_C))
ENG_C := $(filter-out $(DG)/doomgeneric_emscripten.c,$(ENG_C))
ENG_C := $(filter-out $(DG)/doomgeneric_linuxvt.c,$(ENG_C))
ENG_C := $(filter-out $(DG)/doomgeneric_sdl.c,$(ENG_C))
ENG_C := $(filter-out $(DG)/doomgeneric_soso.c,$(ENG_C))
ENG_C := $(filter-out $(DG)/doomgeneric_sosox.c,$(ENG_C))
ENG_C := $(filter-out $(DG)/doomgeneric_win.c,$(ENG_C))
ENG_C := $(filter-out $(DG)/doomgeneric_xlib.c,$(ENG_C))
ENG_C := $(filter-out $(DG)/i_allegromusic.c,$(ENG_C))
ENG_C := $(filter-out $(DG)/i_allegrosound.c,$(ENG_C))
ENG_C := $(filter-out $(DG)/i_sdlmusic.c,$(ENG_C))
ENG_C := $(filter-out $(DG)/i_sdlsound.c,$(ENG_C))

SRC := main.c doomgeneric_termuos.c shim.c crt0.S $(ENG_C)

.PHONY: all dump clean
all: doom

doom: $(SRC)
	@echo "Building with $(words $(ENG_C)) engine files"
	gcc $(CFLAGS) -o doom $(SRC)

dump:
	@echo "$(ENG_C)" | tr ' ' '\n' | grep -E 'sdl|allegro|doomgeneric_' || echo "(no sdl/allegro/platform — good)"

clean:
	rm -f doom
