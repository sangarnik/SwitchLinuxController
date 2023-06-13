#include <util.hpp>
#include <network.hpp>

enum input_type {BTN, LX, LY, RX, RY};
short data[5] = {0, 0, 0, 0, 0};
size_t BUF_SIZE = 10;

int handleInput(PadState* pad, int host_socket, u64 kDown, u64 kUp) {
  HidAnalogStickState analog_stick_l = padGetStickPos(pad, 0);
  HidAnalogStickState analog_stick_r = padGetStickPos(pad, 1);
    
  data[LX] = analog_stick_l.x;
  data[LY] = analog_stick_l.y;
  data[RX] = analog_stick_r.x;
  data[RY] = analog_stick_r.y;
  
  if (kDown & HidNpadButton_A      || kUp & HidNpadButton_A)      data[BTN] ^= 0x1;
  if (kDown & HidNpadButton_B      || kUp & HidNpadButton_B)      data[BTN] ^= 0x2;
  if (kDown & HidNpadButton_X      || kUp & HidNpadButton_X)      data[BTN] ^= 0x4;
  if (kDown & HidNpadButton_Y      || kUp & HidNpadButton_Y)      data[BTN] ^= 0x8;
  if (kDown & HidNpadButton_Up     || kUp & HidNpadButton_Up)     data[BTN] ^= 0x10;
  if (kDown & HidNpadButton_Down   || kUp & HidNpadButton_Down)   data[BTN] ^= 0x20;
  if (kDown & HidNpadButton_Left   || kUp & HidNpadButton_Left)   data[BTN] ^= 0x40;
  if (kDown & HidNpadButton_Right  || kUp & HidNpadButton_Right)  data[BTN] ^= 0x80;
  if (kDown & HidNpadButton_L      || kUp & HidNpadButton_L)      data[BTN] ^= 0x100;
  if (kDown & HidNpadButton_R      || kUp & HidNpadButton_R)      data[BTN] ^= 0x200;
  if (kDown & HidNpadButton_ZL     || kUp & HidNpadButton_ZL)     data[BTN] ^= 0x400;
  if (kDown & HidNpadButton_ZR     || kUp & HidNpadButton_ZR)     data[BTN] ^= 0x800;
  if (kDown & HidNpadButton_StickL || kUp & HidNpadButton_StickL) data[BTN] ^= 0x1000;
  if (kDown & HidNpadButton_StickR || kUp & HidNpadButton_StickR) data[BTN] ^= 0x2000;
  if (kDown & HidNpadButton_Minus  || kUp & HidNpadButton_Minus)  data[BTN] ^= 0x4000;
  if (kDown & HidNpadButton_Plus   || kUp & HidNpadButton_Plus)   data[BTN] ^= 0x8000;
  
  /*
  showbits("Buttons:", data[BTN]);
  showbits("LX:", lx_axis[1]);
  showbits("LY:", ly_axis[1]);
  showbits("RX:", rx_axis[1]);
  showbits("RY:", ry_axis[1]);
  */
  
  if (kDown & HidNpadButton_Minus && kDown & HidNpadButton_Plus)
    return 1;
  else if (write(host_socket, &data, BUF_SIZE) <= 0)
    return 2;
  else
    return 0;
}

int main(int argc, char* argv[]) {
  //char* addr = "192.168.0.139";
  //char* port = "8090";
	PadState pad;
  int host_socket;
  bool inited = false;
  int init_code;

	consoleInit(NULL);
	padConfigureInput(1, HidNpadStyleSet_NpadStandard);
	padInitializeDefault(&pad);
  
 
  std::string port = "8090";
  std::string addr = "192.168.0.139";
  std::string status = "";
  
	//if (net::initClient(&host_socket, addr, port) == 0)
  while (appletMainLoop()) {
    //int inputStatus = handleInput(&pad, host_socket);
    padUpdate(&pad);
    u64 kDown = padGetButtonsDown(&pad);
    u64 kUp = padGetButtonsUp(&pad);

    consoleClear();
    printf("\x1b[1;1HSwitch Controller for Linux");

    printf("\x1b[3;1HWhile disconnected:");
    printf("\x1b[4;1H- Press A to connect");
    printf("\x1b[5;1H- Press B to exit the program");
    printf("\x1b[6;1H- Press Y to set the server ip address");
    printf("\x1b[7;1H- Press X to set the server port");
    
    printf("\x1b[9;1HWhile connected:");
    printf("\x1b[10;1H- Press + and - to disconnect from the server");

    printf("\x1b[12;1HStatus:");
    std::string addr_msg = "\x1b[13;1H- Current IP address "+addr+":"+port;
    printf("%s", addr_msg.c_str());
    
    if (!status.empty()) {
      std::string err_msg = "\x1b[14;1H"+status;
      printf("%s", err_msg.c_str());
    }

    if (!inited) {
      if ( kUp & HidNpadButton_B )
        break;

      if ( kUp & HidNpadButton_Y ) {
        addr = util::swkbdInput(SwkbdType_NumPad, 15, "Server IP address", "Set the server IP address", addr);
        status = "- Server IP address set";
      }

      if ( kUp & HidNpadButton_X ) {
        port = util::swkbdInput(SwkbdType_NumPad, 15, "Server port", "Set the server port", port);
        status = "- Server port set";
      }

      if ( kUp & HidNpadButton_A ) {
        init_code = net::initClient(&host_socket, addr, port);
        switch (init_code) {
          case 1:
            status = "! Error opening socket";
            socketExit();
            break;
          case 2:
            status = "! Error connecting to server";
            close(host_socket);
            socketExit();
            break;
          default:
            status = "- Connected to server";
            inited = true;
            break;
        }
        // TODO: Fix leeching for other buttons
        data[BTN] ^= 0x1;
      }
    }
    if (inited) {
      int input_status = handleInput(&pad, host_socket, kDown, kUp);
      if (input_status != 0) {
        close(host_socket);
        socketExit();
        bzero(&data, BUF_SIZE);
        inited = false;
      }

      switch (input_status) {
        case 1:
          status = "! Exit signal recieved, disconnecting...";
          break;
        case 2:
          status = "! Server stopped responding";
          break;
        default:
          break;
      }
    }
     
    consoleUpdate(NULL);
  }

  consoleExit(NULL);
  return 0;
}



