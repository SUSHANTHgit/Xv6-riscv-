# Week 5 – Doubly-Indirect Block and Soft Links

**Course:** CS334 – Operating Systems Lab
**Assignment:** 5 – *Doubly indirect block and soft links*

This assignment extends the xv6-riscv kernel in two independent tasks. Each task is a
self-contained copy of the xv6 source tree.

| Folder | Task | What it does |
| --- | --- | --- |
| `Part 1 - Doubly-Indirect Blocks/` | Task 5.1 | Adds a doubly-indirect block pointer to the inode so a single file can grow from ~268 KB to ~64 MB. |
| `Part 2 - Soft Links/` | Task 5.2 | Adds a `symlink(target, path)` system call and a new `T_SYMLINK` file type, with transparent link following, `O_NOFOLLOW`, and cycle detection. |

The two parts are **not** cumulative — Part 2 starts from stock xv6, not from Part 1.
See `Assignment 5 Report - Doubly Indirect Block and Soft Links.pdf` for the full write-up,
figures, and test output. Per-part build/run/test instructions are in each folder's `README.md`.

## Notes on the source trees

The xv6 trees here are cleaned copies: build artifacts (`*.o`, `*.d`, `*.asm`, `*.sym`,
`kernel/kernel`, `mkfs/mkfs`, `user/_*`), the generated `fs.img`, and the original nested
`.git` directory have been removed. Run `make qemu` to rebuild from source.
