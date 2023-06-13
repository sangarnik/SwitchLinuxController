#include <util.hpp>

namespace util {
std::string swkbdInput(SwkbdType type, int len, std::string header, std::string subtext, std::string init_text) {
  SwkbdConfig config;
  char buffer[0x100];

  swkbdCreate(&config, 0);
  swkbdConfigMakePresetDefault(&config);
  swkbdConfigSetType(&config, type);
  if ( type == SwkbdType_NumPad )
    swkbdConfigSetLeftOptionalSymbolKey(&config, ".");
  swkbdConfigSetStringLenMax(&config, len);
  swkbdConfigSetHeaderText(&config, header.c_str());
  swkbdConfigSetSubText(&config, subtext.c_str());
  swkbdConfigSetInitialText(&config, init_text.c_str());
  swkbdConfigSetBlurBackground(&config, true);
  swkbdConfigSetKeySetDisableBitmask(&config, SwkbdKeyDisableBitmask_At | SwkbdKeyDisableBitmask_Percent | SwkbdKeyDisableBitmask_ForwardSlash | SwkbdKeyDisableBitmask_Backslash);

  if (R_SUCCEEDED(swkbdShow(&config, buffer, 0x100)) && strcmp(buffer, "") != 0)
    swkbdClose(&config);

  swkbdClose(&config);

  return buffer;
}

void showbits(char* mess, short x) {
	printf("%s ", mess);
	int i=0;
	for (i = (sizeof(short) * 8) - 1; i >= 0; i--)
		putchar(x & (1u << i) ? '1' : '0');
	printf("\n");
}
}

