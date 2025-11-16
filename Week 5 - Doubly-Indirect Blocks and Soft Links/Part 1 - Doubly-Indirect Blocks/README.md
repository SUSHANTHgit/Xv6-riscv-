# Part 1 – Task 5.1: Doubly-Indirect Block Support

**Course:** CS334 – Operating Systems Lab · Assignment 5 · Group 12

## Goal

Stock xv6 gives each inode **12 direct** block addresses and **1 single-indirect**
block, capping a file at `12 + 256 = 268` blocks (≈ 268 KB). This task adds a
**doubly-indirect** block pointer so much larger files are possible.

```
Before:  12 direct + 1 single-indirect          = 12 + 256          = 268 blocks   (~268 KB)
After:   11 direct + 1 single-indirect + 1 double-indirect
                                                 = 11 + 256 + 256*256 = 65 803 blocks (~64 MB)
```

One entry of the `addrs[]` array is repurposed: `NDIRECT` drops from 12 → 11, and the
freed slot plus a new slot hold the single- and doubly-indirect pointers. The on-disk
inode still fits in 128 bytes.

## Changes

| File | Change |
| --- | --- |
| `kernel/param.h` | `FSSIZE` raised `2000 → 200000` blocks so the file system can actually hold a 64 MB file. |
| `kernel/fs.h` | `NDIRECT` `12 → 11`; new macros `NDOUBLY_INDIRECT = NINDIRECT*NINDIRECT` and `MAXFILE = NDIRECT + NINDIRECT + NDOUBLY_INDIRECT`; `dinode.addrs[]` sized `NDIRECT+2`. |
| `kernel/file.h` | In-memory `struct inode` `addrs[]` sized `NDIRECT+2` (`addrs[NDIRECT]` = single-indirect, `addrs[NDIRECT+1]` = doubly-indirect). |
| `kernel/fs.c` | `bmap()` extended: for logical block numbers past the single-indirect range, walk the doubly-indirect block → an indirect block → the data block, allocating each level lazily with `balloc()`. `itrunc()` extended to free both indirection levels (inner indirect blocks, then the outer doubly-indirect block) without corruption. |
| `Makefile` | Adds the `bigfile` user program. |
| `user/bigfile.c` | Test program: writes ~10 000 blocks to one file, closes, reopens, and verifies every block. |

## Build & run

```sh
cd xv6-riscv
make qemu
```

> Note: `mkfs` builds an ~195 MB `fs.img` because of the larger `FSSIZE`; ensure you have
> the disk space. The `fs.img` and other build outputs are not checked in.

## Test

At the xv6 shell prompt:

```sh
$ bigfile
```

Expected: it writes at successive blocks (`write at block 0, 500, 1000, …`), prints
`done with write`, then `bigfile test passed: N blocks (... bytes) written and verified`
with `N` far above the stock limit of 268 (the report shows ~9000+ blocks). Stock xv6
fails this test.
