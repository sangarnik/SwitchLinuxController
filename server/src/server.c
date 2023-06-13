#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <linux/uinput.h>
#include <fcntl.h>


void setAbs(int* fd, int abs_code) {
  struct uinput_abs_setup abs = {
		.code     = abs_code,
		.absinfo  = {
		  .minimum = -32768,
		  .maximum =  32767
		},
	};
  ioctl(*fd, UI_SET_ABSBIT, abs_code);
	ioctl(*fd, UI_ABS_SETUP, &abs);
}


int initUInput(int* fd) {
	if ( (*fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK)) < 0 ) {
		perror("Error opening /dev/uinput");
		return 1;
	}

  // Virtual Joystick metadata
  struct uinput_setup setup = {
		.name = "Microsoft X-Box 360 pad",
		.id = {
			.bustype = BUS_USB,
			.vendor = 0x5E04,
			.product = 0x8E02,
			.version = 0x0401,
		}
	};

  // Setup buttons
	ioctl(*fd, UI_SET_EVBIT, EV_KEY);
	ioctl(*fd, UI_SET_KEYBIT, BTN_A);
	ioctl(*fd, UI_SET_KEYBIT, BTN_B);
	ioctl(*fd, UI_SET_KEYBIT, BTN_X);
	ioctl(*fd, UI_SET_KEYBIT, BTN_Y);
	ioctl(*fd, UI_SET_KEYBIT, BTN_TL);
	ioctl(*fd, UI_SET_KEYBIT, BTN_TR);
	ioctl(*fd, UI_SET_KEYBIT, BTN_THUMBL);
	ioctl(*fd, UI_SET_KEYBIT, BTN_THUMBR);
	ioctl(*fd, UI_SET_KEYBIT, BTN_SELECT);
	ioctl(*fd, UI_SET_KEYBIT, BTN_START);
	ioctl(*fd, UI_SET_KEYBIT, BTN_MODE);

  // Setup analog input
	ioctl(*fd, UI_SET_EVBIT, EV_ABS);
	setAbs(fd, ABS_X);
	setAbs(fd, ABS_Y);
	setAbs(fd, ABS_Z);
	setAbs(fd, ABS_RX);
	setAbs(fd, ABS_RY);
	setAbs(fd, ABS_RZ);
	setAbs(fd, ABS_HAT0X);
	setAbs(fd, ABS_HAT0Y);

	if ( ioctl(*fd, UI_DEV_SETUP, &setup) ) {
		perror("UI_DEV_SETUP");
		return 2;
	}

	if ( ioctl(*fd, UI_DEV_CREATE) ) {
		perror("UI_DEV_CREATE");
		return 3;
	}

	return 0;
}


int initServer(int* server_fd, char* port) {
	struct sockaddr_in server_address;

	*server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if ( *server_fd < 0 ) {
		perror("Error opening server socket");
		return 1;
	}

	bzero((char *) &server_address, sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = INADDR_ANY;
	server_address.sin_port = htons(atoi(port));

	if ( bind(*server_fd, (struct sockaddr *) &server_address, sizeof(server_address)) < 0 ) {
		perror("Error binding server socket");
		return 2;
	}

  printf("Server started in port %s\n", port);
  return 0;
}
 

int clientListen(int* server_fd, int* client_fd) {
	char                client_ip[16];
	struct sockaddr_in  client_address;
  socklen_t           addrlen = sizeof(client_address);

  printf("Listening for clients...\n");

  listen(*server_fd, 5);
  *client_fd = accept(*server_fd, (struct sockaddr*) &client_address, &addrlen);

  if ( *client_fd < 0 ) {
    perror("Error connecting to client");
    return 1;
  }

  inet_ntop(AF_INET, &client_address.sin_addr, client_ip, sizeof(client_ip));
  printf("Client connected: %s\n", client_ip);

  return 0;
}

int updateUInput(int* fd, short data[]) {
	struct input_event ev[20];
	memset(&ev, 0, sizeof ev);

	ev[0].type    = EV_KEY;
	ev[0].code    = BTN_B;
	ev[0].value   = (data[0] & 1);
	data[0]       = data[0] >> 1;

	ev[1].type    = EV_KEY;
	ev[1].code    = BTN_A;
	ev[1].value   = (data[0] & 1);
	data[0]       = data[0] >> 1;

	ev[2].type    = EV_KEY;
	ev[2].code    = BTN_Y;
	ev[2].value   = (data[0] & 1);
	data[0]       = data[0] >> 1;

	ev[3].type    = EV_KEY;
	ev[3].code    = BTN_X;
	ev[3].value   = (data[0] & 1);
	data[0]       = data[0] >> 1;

	ev[4].type    = EV_ABS;
	ev[4].code    = ABS_HAT0Y;
	ev[4].value   = ((data[0] & 1)*-32767) + (((data[0] & 2) >> 1)*32767);
	data[0]       = data[0] >> 2;

	ev[5].type    = EV_ABS;
	ev[5].code    = ABS_HAT0X;
	ev[5].value   = ((data[0] & 1)*-32767) + (((data[0] & 2) >> 1)*32767);
	data[0]       = data[0] >> 2;

	ev[6].type    = EV_KEY;
	ev[6].code    = BTN_TL;
	ev[6].value   = (data[0] & 1);
	data[0]       = data[0] >> 1;

	ev[7].type    = EV_KEY;
	ev[7].code    = BTN_TR;
	ev[7].value   = (data[0] & 1);
	data[0]       = data[0] >> 1;

	ev[8].type    = EV_ABS;
	ev[8].code    = ABS_Z;
	ev[8].value   = ((data[0] & 1) == 0) ? -32768 : 32767;
	data[0]       = data[0] >> 1;

	ev[9].type    = EV_ABS;
	ev[9].code    = ABS_RZ;
	ev[9].value   = ((data[0] & 1) == 0) ? -32768 : 32767;
	data[0]       = data[0] >> 1;

	ev[10].type   = EV_KEY;
	ev[10].code   = BTN_THUMBL;
	ev[10].value  = (data[0] & 1);
	data[0]       = data[0] >> 1;

	ev[11].type   = EV_KEY;
	ev[11].code   = BTN_THUMBR;
	ev[11].value  = (data[0] & 1);
	data[0]       = data[0] >> 1;

	ev[12].type   = EV_KEY;
	ev[12].code   = BTN_SELECT;
	ev[12].value  = (data[0] & 1);
	data[0]       = data[0] >> 1;

	ev[13].type   = EV_KEY;
	ev[13].code   = BTN_START;
	ev[13].value  = (data[0] & 1);
	data[0]       = data[0] >> 1;

	ev[14].type   = EV_ABS;
	ev[14].code   = ABS_X;
	ev[14].value  = data[1];

	ev[15].type   = EV_ABS;
	ev[15].code   = ABS_Y;
	ev[15].value  = (data[2] == 0) ? data[2] : ~data[2];

	ev[16].type   = EV_ABS;
	ev[16].code   = ABS_RX;
	ev[16].value  = data[3];

	ev[17].type   = EV_ABS;
	ev[17].code   = ABS_RY;
	ev[17].value  = (data[4] == 0) ? data[4] : ~data[4];

	ev[18].type   = EV_KEY;
	ev[18].code   = BTN_MODE;
	ev[18].value  = 0;

	ev[19].type   = EV_SYN;
	ev[19].code   = SYN_REPORT;
	ev[19].value  = 0;

	if ( write(*fd, &ev, sizeof ev) < 0 ) {
		perror("Error updating uinput");
		return 1;
	}

	return 0;
}


int destroyUInput(int *fd) {
	if ( ioctl(*fd, UI_DEV_DESTROY) ) {
		perror("UI_DEV_DESTROY");
		return 1;
	}

	close(*fd);
	return 0;
}


void phelp(int exit_code, char* bin_name) {
  fprintf(stderr, "Usage: %s [-pde] [port...]\n", bin_name);
  fprintf(stderr, "-p [port] : Port the server runs on (default: 8090)\n");
  fprintf(stderr, "-d        : Connect joystick only when client connects (default: false)\n");
  fprintf(stderr, "-e        : Exit when client disconnects (default: false)\n");
  fprintf(stderr, "-h        : Show this help\n");
  exit(exit_code);
}


int main(int argc, char* argv[]) {
  int     server_fd;
  int     client_fd;
  int     uinput_fd;
  short   buffer[5] = {0, 0, 0, 0, 0};
  size_t  buff_size = 10;
  int     opt;
  char*   port = "8090";
  bool    exit_on_disconnect = false;
  bool    input_on_connection = false;

  while ( (opt = getopt(argc, argv, ":p:deh")) != -1 ) {
    switch (opt) {
      case 'p':
        port = optarg;
        break;
      case 'd':
        input_on_connection = true;
        break;
      case 'e':
        exit_on_disconnect = true;
        break;
      case 'h':
        phelp(0, argv[0]);
        break;
      case ':':
        printf("Option needs a value\n");
        phelp(254, argv[0]);
        break;
      case '?':
        printf("Unknown option: %c\n", optopt);
        phelp(255, argv[0]);
        break;
    }
  }

	if ( !input_on_connection && initUInput(&uinput_fd) > 0 ) 
    return 1;
	if ( initServer(&server_fd, port) > 0 )
    return 2;

	while (true) {
    if ( clientListen(&server_fd, &client_fd) == 0 ) {
      if ( input_on_connection && initUInput(&uinput_fd) > 0 )
        return 1;

      while (true) {
        bzero(&buffer, buff_size);
        if ( read(client_fd, &buffer, buff_size) <= 0 || updateUInput(&uinput_fd, buffer) > 0 )
          break;
      }
      
      printf("Client disconnected\n");
      close(client_fd);
      if ( input_on_connection )
        destroyUInput(&uinput_fd);
    }

    if ( exit_on_disconnect )
      break;
	}

	printf("Stopping server...\n");
	close(server_fd);
  if ( !input_on_connection )
    destroyUInput(&uinput_fd);
	return 0;
}
