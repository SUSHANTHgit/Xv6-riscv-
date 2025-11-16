# Part 2 – Task 5.2: Symbolic Links (Soft Links)

**Course:** CS334 – Operating Systems Lab · Assignment 5 · Group 12

## Goal

Add UNIX-style **symbolic (soft) links** to xv6. Unlike hard links, a symlink stores a
**path** to its target, can point at directories, and can cross devices. Opening a
symlink transparently opens whatever it points to.

New system call:

```c
int symlink(const char *target, const char *path);
```

## How it works

* **New file type `T_SYMLINK`** (`kernel/stat.h`) distinguishes links from files/dirs.
* **`sys_symlink()`** (`kernel/sysfile.c`) `create()`s an inode of type `T_SYMLINK` at
  `path` and writes the `target` string into its data block with `writei()`. The inode
  holds only the path, no file data.
* **`sys_open()`** (`kernel/sysfile.c`): after `namei()`, if the inode is `T_SYMLINK`
  and `O_NOFOLLOW` was **not** passed, it loops:
  1. `readi()` the stored target path,
  2. `namei(target)` to get the next inode,
  3. `ilock` and re-check the type,
  until a non-symlink is reached, then continues as a normal open.
* **`O_NOFOLLOW`** (`0x800`): opens the link inode itself instead of following it —
  useful for inspecting or removing a link.
* **Cycle / depth limit:** a `depth` counter aborts with `-1` once it reaches **10**
  consecutive symlink hops, so `A → B → A` cannot loop forever.

## Changes

| File | Change |
| --- | --- |
| `kernel/stat.h` | Add `#define T_SYMLINK 4`. |
| `kernel/syscall.h` | Add `#define SYS_symlink 27`. |
| `kernel/syscall.c` | Declare and register `sys_symlink` in the syscall table. |
| `kernel/sysfile.c` | Implement `sys_symlink()`; extend `sys_open()` with symlink resolution, `O_NOFOLLOW` handling, and the depth-10 loop guard (`O_NOFOLLOW` defined locally as `0x800`). |
| `user/usys.pl` | Add `entry("symlink")` so the user stub is generated. |
| `user/user.h` | Declare `int symlink(const char *target, const char *path);`. |
| `Makefile` | Add the `symlinktest` user program. |
| `user/symlinktest.c` | Tests: basic file link, nested 3-level link, cycle detection, symlink to a directory, and `O_NOFOLLOW`. |

> Stock xv6 defaults (`NDIRECT = 12`, `FSSIZE = 2000`) are unchanged here — this part is
> independent of Part 1.

## Build & run

```sh
cd xv6-riscv
make qemu
```

## Test

At the xv6 shell prompt:

```sh
$ symlinktest
```

Expected output ends with `== All symbolic link tests passed ==`, having shown:
`Basic symlink test`, `linkA -> contents: xv6 symlink demo`, `Resolved through 3-level
link`, `Loop correctly detected and blocked.`, and `O_NOFOLLOW correctly opened symlink
inode.`
