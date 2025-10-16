# Week 3 – Interprocess Communication

**Course:** CS334 – Operating Systems Lab
**Assignment:** 3 – *Interprocess Communication* (Task 3.1: IPC kernel primitives · Task 3.2: The Intertwined Memory Challenge)

This is a single xv6-riscv tree — Task 3.2 builds directly on the primitives from
Task 3.1, so the two are kept together rather than split into parts. Only the files
that were actually changed from stock xv6 are included; see the report
`Assignment 3 Report - Interprocess Communication.pdf` for the full write-up and figures.

## Goal

Stock xv6 has almost no interprocess communication beyond pipes. This assignment adds
two kernel primitives and then uses them to build a small concurrent application.

**Task 3.1 — IPC primitives.** Two mechanisms, exposed as new system calls:

* **Shared memory** — a one-page region identified by an integer key. `shm_create`
  allocates the page, `shm_get` maps it into the calling process and returns a pointer,
  `shm_close` detaches it; the physical page is freed only when the last attached
  process closes it. All processes using the same key see the same physical page.
* **Mailboxes** — a fixed-size circular queue of integer messages identified by a key.
  `mbox_create` allocates one, `mbox_send` / `mbox_recv` enqueue / dequeue and **block**
  (via `sleep`/`wakeup`) when the mailbox is full / empty.

**Task 3.2 — The Intertwined Memory Challenge.** Two processes navigate an "intertwined"
path held in shared memory. Neither knows its own next position — each can only read the
*other* process's next position and must hand it over by message. `master` seeds the
path and the mailboxes and forks the two workers; each `process` instance walks its path
by repeatedly sending its partner's next index and receiving its own.

## Changes

All claims below were checked against the code in `xv6-riscv/`, not just the report.

| File | Change |
| --- | --- |
| `Makefile` | Adds `$K/shm.o` and `$K/mbox.o` to the kernel `OBJS`, and `$U/_shmtest`, `$U/_mboxtest`, `$U/_master`, `$U/_process` to `UPROGS`. *(This snapshot's `UPROGS` also lists `_cowtest` and `_bigfile`, which belong to other assignments sharing the same working tree.)* |
| `kernel/shm.h` | **New.** `struct mem_reg { void *pa; int no_of_process; struct spinlock lock; }`; global table `gt[1000]`; prototypes `shm_init/shm_create/shm_get/shm_close`. |
| `kernel/shm.c` | **New.** Shared-memory regions keyed by an integer (used as a direct index into `gt[]`, range 0–999, no bounds check). `shm_create` `kalloc`s and zeroes one page if the key is free; `shm_get` maps that page into the caller at the fixed virtual address `0x40000000 - (key+1)*PGSIZE` with `PTE_R|PTE_W|PTE_U`, increments the per-region process count, and returns the VA; `shm_close` `uvmunmap`s it for the caller and frees the physical page once the last holder detaches. `shm_init` clears the table at boot. |
| `kernel/mbox.h` | **New.** `struct mbox { int queue[10]; int front, rear, length, valid; struct spinlock lock; }`; global `array_of_mb[1000]`; prototypes for `enqueue/dequeue/mbox_init/mbox_create/mbox_send/mbox_recv`. |
| `kernel/mbox.c` | **New.** 1000 mailboxes, each a 10-slot circular integer queue guarded by a spinlock. `mbox_create` marks a key valid (returns −1 if already valid). `mbox_send` / `mbox_recv` take the lock and loop on `enqueue` / `dequeue`; on full / empty they `wakeup` the other side and then `sleep` on the mailbox, which releases the lock — so a blocked sender or receiver never holds the lock (the report's deadlock argument). Each call prints a trace line (`sent %d…` / `receieved %d…`) and `wakeup`s waiters before releasing. |
| `kernel/syscall.h` | Adds `SYS_shm_create 22`, `SYS_shm_get 23`, `SYS_shm_close 24`, `SYS_mbox_create 25`, `SYS_mbox_send 26`, `SYS_mbox_recv 27`, and `SYS_va2pa 28`. |
| `kernel/syscall.c` | `extern` declarations and syscall-table entries for the seven new handlers. |
| `kernel/sysproc.c` | Includes `shm.h` / `mbox.h`; implements `sys_shm_create` / `sys_shm_get` / `sys_shm_close`, `sys_mbox_create` / `sys_mbox_send`, `sys_mbox_recv` (copies the received `int` back to user space with `copyout`), and `sys_va2pa` (returns `walkaddr(myproc()->pagetable, va)` — a VA→PA debug helper, not one of the report's six calls). |
| `user/user.h` | Declares `shm_create`, `shm_get`, `shm_close`, `mbox_create`, `mbox_send`, `mbox_recv`, and `va2pa`. |
| `user/usys.pl` | `entry(...)` lines that generate the user-space stubs for the same seven calls. |
| `user/shmtest.c` | **New.** Parent `shm_get`s key 1, writes `"hello from proc1"`, forks; the child attaches the same key, prints what it reads, overwrites it with `"hello from child"`, and detaches; the parent waits, then reads the child's value back through its own mapping. |
| `user/mboxtest.c` | **New.** Creates mailbox key 1, forks; the child `mbox_send`s integers 0–4, the parent `mbox_recv`s five messages and prints each. |
| `user/master.c` | **New.** Task 3.2 setup. Attaches shared key 1, writes the hard-coded intertwined path into it as an int array (`ptr[0]=5, ptr[1]=6, ptr[5]=11, ptr[6]=10, ptr[10]=21, ptr[11]=20`), creates mailboxes 1 and 2, forks two children that `exec("process", …)` with start/end arguments `("0","21")` and `("1","20")`, then `shm_close`s. |
| `user/process.c` | **New.** Task 3.2 worker. Attaches shared key 1 and reads `start` / `end` from `argv`. The `start==0` role ("process A") sends on mailbox 1 and receives on mailbox 2; the `start==1` role ("process B") sends on mailbox 2 and receives on mailbox 1. Each iteration it sends the partner's next index (`ptr[pos]`) and blocks to receive its own, looping until it reaches `end`. Sending before receiving on every step is the deadlock-avoidance rule from §2.4 of the report. |

## Build & run

```sh
cd xv6-riscv
make qemu
```

## Test

At the xv6 shell prompt:

```sh
$ shmtest
```
Expected: `child: read from shared memory: hello from proc1` followed by
`parent: read from shared memory: hello from child`, then `shmtest: finished.` — the
child's write is visible to the parent through the shared page.

```sh
$ mboxtest
```
Expected: interleaved `child:sending0..4` and `parent:received0..4` (plus the kernel's
`sent … / receieved …` trace lines) — the parent drains all five queued messages.

```sh
$ master
```
Expected: alternating `process A currently at: …` / `process B currently at: …` lines
as the two workers hand each other positions `0 → 6 → 11 → 21` and `1 → 5 → 10 → 20`,
ending once both reach their end points (21 and 20).

## Notes / discrepancies with the report

* The report's **title page** says "Scheduling Algorithms", but its table of contents and
  every section are about IPC (Task 3.1 primitives, Task 3.2 intertwined paths). Treated
  as an IPC assignment — the title line looks like a leftover from a template.
* The report says **"Six system calls were introduced"**; the code adds **seven** — the
  extra one is `va2pa` (a VA→PA translation helper). The report also calls the receive
  call `mbox_receive`, while the actual syscall / stub is `mbox_recv`.
* The report lists **`defs.h`** among the changed files, and the working tree confirms it
  is modified (it adds `inc` / `dec` / `get` refcount helpers and a COW-fault prototype).
  **`kernel/main.c`** is *also* modified in the working tree — it adds the `shm_init()`
  and `mbox_init()` calls to `main()`. Neither `defs.h` nor `main.c` is included here,
  per the file list scoped for this snapshot; the kernel side will not link / boot
  without them.
* `master.c` calls `shm_get(1)` directly (which creates the region internally) rather
  than `shm_create` as the report's step 1 describes, and it calls `wait(0)` once for its
  two children.
