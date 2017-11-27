#include <stdlib.h>
#include <conio.h>
#include <dos.h>
#include <i86.h>
#include "OPL2.h"
#include "demotune.h"

#define STR(x) #x
#define XSTR(x) STR(x)


static short get_lpt_port(int i)
{
  return *(short __far *)MK_FP(0x40, 6 + 2*i);
}

static short setup(void)
{
  char num_ports, port, i;

  num_ports = 0;
  for (i = 1; i < 4; i++) {
    if (get_lpt_port(i)) {
      num_ports++;
      port = i;
    }
  }

  if (num_ports == 0) {
    cputs("Sorry, no printer port found...\r\n");
    exit(1);
  }
  else if (num_ports == 1) {
    cprintf("Found one printer port: LPT%d\r\n", port);
    return get_lpt_port(port);
  }
  else {
    cputs("Found multiple printer ports:");
    for (i = 1; i < 4; i++) {
      if (get_lpt_port(i)) {
        cprintf(" LPT%d", i);
      }
    }
    cputs("\r\nWhich one is the OPL2LPT connected to? [");
    for (i = 1; i < 4; i++) {
      if (get_lpt_port(i)) {
        cprintf("%d", i);
      }
    }
    cputs("]? ");
    do {
      port = getch() - '0';
    } while (port < 1 || port > 3 || !get_lpt_port(port));
    cprintf("LPT%d\r\n", port);
    return get_lpt_port(port);
  }
  return 0;
}

static volatile int interrupted = 0;

static void __interrupt __far ctrlc_handler()
{
  interrupted = 1;
}

int main(void)
{
  cputs("== OPL2LPT test program (" XSTR(VERSION) ") ==\r\n\r\n");
  short lpt_base = setup();
  cputs("\r\nPress any key to play some music...");
  do {
    getch();
  } while (kbhit());
  _dos_setvect(0x23, ctrlc_handler);
  extern OPL2 opl2;
  opl2.init(lpt_base);
  music_setup();
  cputs("\r\n\r\nPress any key to stop...");
  while (!kbhit() && !interrupted) {
    music_loop();
  }
  music_shutdown();
  while (kbhit()) {
    getch();
  }
  cputs("\r\n\r\n");
  return 0;
}
