# TermuOS tests

Boots the built ISO under QEMU, types commands on the emulated PS/2
keyboard, and asserts on what comes back over the serial port. There is no
test runner inside the OS yet, so the checks are driven from outside.

## Running

```
make test
```

That builds the ISO, creates a fresh `test.img` with the userland, the test
programs and the fixtures installed, and runs `tests/run_tests.py`. It exits
non-zero if any check fails. The full guest output is left in
`tests-serial.log`.

The image is rebuilt from scratch every run on purpose: `tools/tfs_write`
skips a file that already exists, so reusing an image would silently keep
testing yesterday's binaries.

To re-run the checks against an ISO and image you already built:

```
python3 tests/run_tests.py --iso termuos.iso --img test.img
```

Slow machines may need a longer pause after each command:
`--settle 5`.

## What is checked

`tests/run_tests.py` holds the command table. Each entry lists substrings
that must appear in that command's output; `KERNEL PANIC` must never appear
in any of them. The kernel's own `exited with code N` line is used to check
exit status, since the shell cannot report it yet.

Covered:

- `echo` and `uname` produce their output and exit 0
- `run /bin/cat.tsys /etc/motd` — issue #41's acceptance criterion
- a 1008-byte file, read across several capped reads
- a 4600-byte file, which also crosses a 4096-byte TFS block boundary
- an empty file, which must be an immediate EOF rather than a hang
- several files passed to one `cat`
- a missing file: an error message and exit status 1
- `readtest` — read() routing and the descriptor reservation (issue #41)
- `uatest` — the user-buffer helpers (issue #40)

The last two are programs in `tests/tsys/`. They print one `PASS`/`FAIL`
line per check and the harness fails the run if it sees a `FAIL`, so a
broken detail is reported even when the command as a whole still completes.

## stdintest, and a known panic

`tests/tsys/stdintest.c` blocks in `read(0)` for one keystroke. It is built
and installed but is **not** in the automated table, because it needs a key
press and because a long block currently takes the kernel down:

```
run /bin/stdintest.tsys        # then wait a few seconds before typing
*** KERNEL PANIC *** Exception #13: General Protection Fault  (in irq_common)
```

If the key arrives within a few timer ticks the read returns it correctly.
Blocking for longer panics. This is not a `read()` bug — the same binary
panics identically on `main`. Every ring 3 to ring 0 transition uses the one
stack installed by `tss_set_kernel_stack()`, and every syscall shares one
kernel stack, so preempting a thread that is parked inside a syscall
corrupts the frame that `iretq` later returns through. Fixing it means
per-thread kernel stacks with `TSS.RSP0` updated on each context switch.

Run it by hand to reproduce:

```
run /bin/stdintest.tsys
```

## Adding a case

Add a `(command, [expected substrings])` entry to `CASES` in
`tests/run_tests.py`. For anything needing more than string matching, write
a small program in `tests/tsys/` that prints `PASS`/`FAIL` lines — the
Makefile picks up new `.c` files there via `TEST_SRCS`, and installs them to
`/bin`.
