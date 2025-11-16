#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"
#include "kernel/fs.h"
#define O_NOFOLLOW 0x800 
#define BUFSZ 128

void
check(int cond, char *msg)
{
  if (!cond) {
    printf("FAILED: %s\n", msg);
    exit(1);
  }
}

void
test_basic()
{
  printf(" Basic symlink test \n");

  int fd = open("base.txt", O_CREATE | O_RDWR);
  write(fd, "xv6 symlink demo\n", 17);
  close(fd);

  check(symlink("base.txt", "linkA") == 0, "symlink creation failed");

  char buf[BUFSZ] = {0};
  fd = open("linkA", O_RDONLY);
  check(fd >= 0, "open linkA failed");

  read(fd, buf, sizeof(buf));
  close(fd);

  printf("linkA -> contents: %s\n", buf);
  unlink("base.txt");
  unlink("linkA");
}

void
test_nested_links()
{
  printf(" Nested symlink test \n");

  int fd = open("f1.txt", O_CREATE | O_RDWR);
  write(fd, "Deep link success!\n", 20);
  close(fd);

  check(symlink("f1.txt", "l1") == 0, "symlink l1 creation failed");
  check(symlink("l1", "l2") == 0, "symlink l2 creation failed");
  check(symlink("l2", "l3") == 0, "symlink l3 creation failed");

  char buf[BUFSZ] = {0};
  fd = open("l3", O_RDONLY);
  check(fd >= 0, "open l3 failed");

  read(fd, buf, sizeof(buf));
  close(fd);

  printf("Resolved through 3-level link: %s\n", buf);

  unlink("f1.txt");
  unlink("l1");
  unlink("l2");
  unlink("l3");
}

void
test_loop_detection()
{
  printf(" Loop detection test \n");

  // create circular symlinks: A -> B, B -> A
  check(symlink("loopB", "loopA") == 0, "symlink loopA failed");
  check(symlink("loopA", "loopB") == 0, "symlink loopB failed");

  int fd = open("loopA", O_RDONLY);
  check(fd < 0, "loop detection failed (should not resolve infinitely)");

  printf("Loop correctly detected and blocked.\n");

  unlink("loopA");
  unlink("loopB");
}

void
test_on_directory()
{
  printf(" Symlink to directory test \n");

  mkdir("dir1");
  check(symlink("dir1", "dirlink") == 0, "symlink to dir failed");

  int fd = open("dirlink", O_RDONLY);
  if (fd < 0)
    printf("Symlink to directory cannot be opened as file (expected)\n");
  else {
    printf("Unexpectedly opened symlinked directory\n");
    close(fd);
  }

  unlink("dirlink");
  unlink("dir1");
}

void
test_nofollow()
{
  printf(" O_NOFOLLOW test \n");

  int fd = open("target.txt", O_CREATE | O_RDWR);
  write(fd, "no-follow", 9);
  close(fd);

  check(symlink("target.txt", "lnk") == 0, "symlink creation failed");

  int fd_link = open("lnk", O_RDONLY | O_NOFOLLOW);
  check(fd_link >= 0, "open with O_NOFOLLOW failed");

  struct stat st;
  fstat(fd_link, &st);
  check(st.type == T_SYMLINK, "O_NOFOLLOW opened target instead of link");

  printf("O_NOFOLLOW correctly opened symlink inode.\n");

  close(fd_link);
  unlink("lnk");
  unlink("target.txt");
}

int
main(void)
{
  printf("\n== Starting symlink tests ==\n");

  test_basic();
  test_nested_links();
  test_loop_detection();
  test_on_directory();
  test_nofollow();

  printf("== All symbolic link tests passed ==\n");
  exit(0);
}
