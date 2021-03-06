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

#define BUTTONCOUNT 2
#define MAXPROFNUM 255

#define die(str, args...) do { \
  perror(str); \
  exit(EXIT_FAILURE); \
} while (0)

int configBut[MAXPROFNUM][BUTTONCOUNT];
int config_cnt = 0;
char configName[MAXPROFNUM][MAXPROFNUM];
int curr_config = 0;

void BT_init(int *s, int *client) {
  struct sockaddr_rc loc_addr = { 0 }, rem_addr = { 0 };
  socklen_t opt = sizeof(rem_addr);

  *s = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

  // bind socket to port 1 of the first available
  // local bluetooth adapter
  loc_addr.rc_family = AF_BLUETOOTH;
  loc_addr.rc_bdaddr = *BDADDR_ANY;
  loc_addr.rc_channel = (uint8_t) 0;
  bind(*s, (struct sockaddr *)&loc_addr, sizeof(loc_addr));

  // put socket into listening mode
  listen(*s, 1);

  // accept one connection
  *client = accept(*s, (struct sockaddr *)&rem_addr, &opt);
}

void mouse_init(int *fd) {
  struct uinput_user_dev uidev;
  int i, j;

  FILE *fconfig = fopen("profiles.txt", "r");

  for (config_cnt = 0; !feof(fconfig); config_cnt++) {
    fscanf(fconfig, " %s", configName[config_cnt]);
    for(i = 0; i<BUTTONCOUNT; i++) {
      fscanf(fconfig, " %d", &(configBut[config_cnt][i])) ;
    }
    // printf("%s %d %d\n", configName[config_cnt], configBut[config_cnt][0], configBut[config_cnt][1]);
  }
  fclose(fconfig);
  config_cnt--;
  *fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
  if (*fd < 0) {
    die("error: open");
  }

  if (ioctl(*fd, UI_SET_EVBIT, EV_KEY) < 0) {
    die("error: ioctl");
  }

  if (ioctl(*fd, UI_SET_KEYBIT, BTN_LEFT) < 0) {
    die("error: ioctl");
  }

  if (ioctl(*fd, UI_SET_KEYBIT, BTN_RIGHT) < 0) {
    die("error: ioctl");
  }

  for (j = 0; j < config_cnt; j++) {
    for(i = 0; i < BUTTONCOUNT; i++) {
      if(ioctl(*fd, UI_SET_KEYBIT, configBut[j][i]) < 0) {
        die("error: ioctl");
      }
    }
  }

  if (ioctl(*fd, UI_SET_EVBIT, EV_REL) < 0) {
    die("error: ioctl");
  }

  if (ioctl(*fd, UI_SET_RELBIT, REL_X) < 0) {
    die("error: ioctl");
  }

  if (ioctl(*fd, UI_SET_RELBIT, REL_Y) < 0) {
    die("error: ioctl");
  }

  if (ioctl(*fd, UI_SET_RELBIT, REL_WHEEL) < 0) {
    die("error: ioctl");
  }

  if (ioctl(*fd, UI_SET_RELBIT, REL_HWHEEL) < 0) {
    die("error: ioctl");
  }

  memset(&uidev, 0, sizeof(uidev));
  snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "uinput-sample");
  uidev.id.bustype = BUS_USB;
  uidev.id.vendor  = 0x1;
  uidev.id.product = 0x1;
  uidev.id.version = 1;

  if (write(*fd, &uidev, sizeof(uidev)) < 0) {
    die("error: write");
  }

  if (ioctl(*fd, UI_DEV_CREATE) < 0) {
    die("error: ioctl");
  }
}

void BT_final(int *s, int *client) {
  close(*client);
  close(*s);
}

void mouse_final(int *fd) {
  if (ioctl(*fd, UI_DEV_DESTROY) < 0) {
    die("error: ioctl");
  }

  close(*fd);
}

void mouse_click(int fd, char button, char actType) {
  struct input_event ev;

  memset(&ev, 0, sizeof(struct input_event));
  // printf("Button pressed: %c\n", button);
  ev.type = EV_KEY;
  if (button == 'L') {
    ev.code = BTN_LEFT;
  } else if (button == 'R') {
    ev.code = BTN_RIGHT;
  } else if (button == '1') {
    ev.code = configBut[curr_config][0];
  } else if (button == '2') {
    ev.code = configBut[curr_config][1];
  }

  if (actType == 'P') {
    // printf("Pressed\n");
    ev.value = 1;
  } else if (actType == 'R') {
    ev.value = 0;
  }

  if (write(fd, &ev, sizeof(struct input_event)) < 0) {
    die("error: write");
  }

  memset(&ev, 0, sizeof(struct input_event));
  ev.type = EV_SYN;
  ev.code = 0;
  ev.value = 0;

  if (write(fd, &ev, sizeof(struct input_event)) < 0) {
    die("error: write");
  }
}

void scroll_wheel(int fd, int dx, int dy) {
  struct input_event ev;

  memset(&ev, 0, sizeof(struct input_event));
  ev.type = EV_REL;
  ev.code = REL_HWHEEL;
  ev.value = dx;
  if (write(fd, &ev, sizeof(struct input_event)) < 0) {
    die("error: write");
  }

  memset(&ev, 0, sizeof(struct input_event));
  ev.type = EV_REL;
  ev.code = REL_WHEEL;
  ev.value = dy;
  if (write(fd, &ev, sizeof(struct input_event)) < 0) {
    die("error: write");
  }

  memset(&ev, 0, sizeof(struct input_event));
  ev.type = EV_SYN;
  ev.code = 0;
  ev.value = 0;
  if (write(fd, &ev, sizeof(struct input_event)) < 0) {
    die("error: write");
  }
}

void mouse_step(int fd, int dx, int dy) {
  struct input_event ev;
  int i;

  int dxCurrent, dyCurrent;
  float dxAcc, dyAcc;
  float dxUnit, dyUnit;
  dxCurrent = 0;
  dyCurrent = 0;
  dxAcc = 0;
  dyAcc = 0;
  dxUnit = (float) dx / 10.0f;
  dyUnit = (float) dy / 10.0f;
  for (i = 0; i < 10; i++) {

    dx = dy = 0;
    dxAcc += dxUnit;
    dyAcc += dyUnit;
    if (dxUnit > 0 && (int) dxAcc > dxCurrent) {
      dx = (int) dxAcc - dxCurrent;
      dxCurrent = (int) dxAcc;
    }
    if (dxUnit < 0 && (int) dxAcc < dxCurrent) {
      dx = (int) dxAcc - dxCurrent;
      dxCurrent = (int) dxAcc;
    }
    if (dyUnit > 0 && (int) dyAcc > dyCurrent) {
      dy = (int) dyAcc - dyCurrent;
      dyCurrent = (int) dyAcc;
    }
    if (dyUnit < 0 && (int) dyAcc < dyCurrent) {
      dy = (int) dyAcc - dyCurrent;
      dyCurrent = (int) dyAcc;
    }

    memset(&ev, 0, sizeof(struct input_event));
    ev.type = EV_REL;
    ev.code = REL_X;
    ev.value = dx;
    if (write(fd, &ev, sizeof(struct input_event)) < 0) {
      die("error: write");
    }

    memset(&ev, 0, sizeof(struct input_event));
    ev.type = EV_REL;
    ev.code = REL_Y;
    ev.value = dy;
    if (write(fd, &ev, sizeof(struct input_event)) < 0) {
      die("error: write");
    }

    memset(&ev, 0, sizeof(struct input_event));
    ev.type = EV_SYN;
    ev.code = 0;
    ev.value = 0;
    if (write(fd, &ev, sizeof(struct input_event)) < 0) {
      die("error: write");
    }
    // usleep(1000);
  }
}

void BT_read(int client, int *dx, int *dy, char *type) {
  char buf[1024] = { 0 };
  int bytes_read = read(client, buf, sizeof(buf));
  if (bytes_read > 0) {
    // printf("Read: %s\n", buf);
    sscanf(buf, " %c %d %d", type, dx, dy);
    //printf("%d %d\n", dx, dy);
  } else {
    *dx = *dy = 0;
  }
}

void change_config() {
  curr_config = (curr_config + 1) % config_cnt;
  printf("C %s %d %d\n", configName[curr_config], configBut[curr_config][0], configBut[curr_config][1]);
  fflush(stdout);
}

int main(int argc, char **argv) {
  int s, client, dx, dy;
  int fd;
  char button, actType;
  char type;

  BT_init(&s, &client);

  mouse_init(&fd);

  puts("A 0 0 0");
  fflush(stdout);
  // read data from the client
  while (1) {
    BT_read(client, &dx, &dy, &type);
    if (type == 'M') {
      mouse_step(fd, dx, dy);
    } else if (type == 'B') {
      if (dx == 0) {
        button = 'L';
      } else if (dx == 1) {
        button = 'R';
      } else if (dx == 2) {
	      button = '1';
      } else if (dx == 3) {
	      button = '2';
      }

      if (dy == 0) {
        actType = 'P';
      } else {
        actType = 'R';
      }

      mouse_click(fd, button, actType);
    } else if (type == 'S') {
      scroll_wheel(fd, dx, dy);
    } else if (type == 'C') {
      change_config();
    }
  }
  mouse_final(&fd);

  // close connection
  BT_final(&s, &client);
  return 0;
}
