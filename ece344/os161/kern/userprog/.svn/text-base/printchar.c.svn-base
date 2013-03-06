#include <types.h>
#include <kern/errno.h>
#include <kern/unistd.h>
#include <machine/spl.h>
#include <test.h>
#include <addrspace.h>
#include <synch.h>
#include <thread.h>
#include <scheduler.h>
#include <dev.h>
#include <vfs.h>
#include <vm.h>
#include <syscall.h>
#include <version.h>
#include <lib.h>
#include <curthread.h>

int 
sys_printchar(char word)
{
  kprintf("%c", word);

  return 0;
}
