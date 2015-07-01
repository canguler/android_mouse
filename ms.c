#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define die(str, args...) do { \
  perror(str); \
  exit(EXIT_FAILURE); \
} while(0)

void BT_init(int *s, int *client) {
  struct sockaddr_rc loc_addr = { 0 }, rem_addr = { 0 };
  socklen_t opt = sizeof(rem_addr);

  *s = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

  // bind socket to port 1 of the first available
  // local bluetooth adapter
  loc_addr.rc_family = AF_BLUETOOTH;
  loc_addr.rc_bdaddr = *BDADDR_ANY;
  loc_addr.rc_channel = (uint8_t) 1;
  bind(*s, (struct sockaddr *)&loc_addr, sizeof(loc_addr));

  // put socket into listening mode
  listen(*s, 1);

  // accept one connection
  *client = accept(*s, (struct sockaddr *)&rem_addr, &opt);
}

void mouse_init(int *fd) {
  struct uinput_user_dev uidev;

  *fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
  if(*fd < 0)
  die("error: open");

  if(ioctl(*fd, UI_SET_EVBIT, EV_KEY) < 0)
  die("error: ioctl");
  if(ioctl(*fd, UI_SET_KEYBIT, BTN_LEFT) < 0)
  die("error: ioctl");

  if(ioctl(*fd, UI_SET_EVBIT, EV_REL) < 0)
  die("error: ioctl");
  if(ioctl(*fd, UI_SET_RELBIT, REL_X) < 0)
  die("error: ioctl");
  if(ioctl(*fd, UI_SET_RELBIT, REL_Y) < 0)
  die("error: ioctl");

  memset(&uidev, 0, sizeof(uidev));
  snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "uinput-sample");
  uidev.id.bustype = BUS_USB;
  uidev.id.vendor  = 0x1;
  uidev.id.product = 0x1;
  uidev.id.version = 1;

  if(write(*fd, &uidev, sizeof(uidev)) < 0)
  die("error: write");

  if(ioctl(*fd, UI_DEV_CREATE) < 0)
  die("error: ioctl");
}

void BT_final(int *s, int *client) {
  close(*client);
  close(*s);
}

void mouse_final(int *fd) {
  if(ioctl(*fd, UI_DEV_DESTROY) < 0)
  die("error: ioctl");

  close(*fd);
}

void mouse_step(int fd, int dx, int dy) {
  struct input_event ev;
  int i;

  for(i = 0; i < 1; i++) {
    memset(&ev, 0, sizeof(struct input_event));
    ev.type = EV_REL;
    ev.code = REL_X;
    ev.value = dx;
    if(write(fd, &ev, sizeof(struct input_event)) < 0)
    die("error: write");

    memset(&ev, 0, sizeof(struct input_event));
    ev.type = EV_REL;
    ev.code = REL_Y;
    ev.value = dy;
    if(write(fd, &ev, sizeof(struct input_event)) < 0)
    die("error: write");

    memset(&ev, 0, sizeof(struct input_event));
    ev.type = EV_SYN;
    ev.code = 0;
    ev.value = 0;
    if(write(fd, &ev, sizeof(struct input_event)) < 0)
    die("error: write");

    usleep(10000);
  }
}

void BT_read(int client, int *dx, int *dy) {
  char buf[1024] = { 0 };
  int bytes_read = read(client, buf, sizeof(buf));
  if(bytes_read > 0) {
    printf("Read: %s\n", buf);
    sscanf(buf, " %d %d", dx, dy);
    //printf("%d %d\n", dx, dy);
  } else {
    *dx = *dy = 0;
  }
}

int main(int argc, char **argv) {
  int s, client, dx, dy;
  int fd;

  BT_init(&s, &client);

  mouse_init(&fd);

  puts("ready");

  // read data from the client
  while(1) {
    BT_read(client, &dx, &dy);
    mouse_step(fd, dx, dy);
  }
  mouse_final(&fd);

  // close connection
  BT_final(&s, &client);
  return 0;
}
