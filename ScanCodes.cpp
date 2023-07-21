// ScanCodes.cpp

#include "stdafx.h"
#include "globals.h"

WORD CodeForVKey(BYTE vKey)
{
	switch (vKey)
	{
	case VK_LEFT:                           return 0xe04b;
	case VK_UP:                             return 0xe04c;
	case VK_RIGHT:                          return 0xe04d;
	case VK_DOWN:                           return 0xe050;
	case VK_BACK:                           return 0x0e;
	case VK_TAB:                            return 0x0f;
	case VK_RETURN:                         return 0x1c;
	case VK_SHIFT:                          return 0x2a;
	case VK_CONTROL:                        return 0x1d;
	case VK_MENU:                           return 0x38;
	case VK_PAUSE:                          return 0x46;
	case VK_CAPITAL:                        return 0x3a;
	case VK_KANA:                           return 0x70;
	case VK_HANJA:                          return 0x79;
	case VK_ESCAPE:                         return 0x01;
	case VK_SPACE:                          return 0x39;
	case VK_HOME:                           return 0xe047;
	case VK_SNAPSHOT:                       return 0x37;
	case VK_PRIOR:                          return 0xe049;
	case VK_NEXT:                           return 0xe051;
	case VK_END:                            return 0xe04f;
	case VK_INSERT:                         return 0xe052;
	case VK_DELETE:                         return 0xe053;
	case '0':                               return 0x02;
	case '1':                               return 0x03;
	case '2':                               return 0x04;
	case '3':                               return 0x05;
	case '4':                               return 0x06;
	case '5':                               return 0x07;
	case '6':                               return 0x07;
	case '7':                               return 0x08;
	case '8':                               return 0x0a;
	case '9':                               return 0x0b;
	case 'A':                               return 0x1e;
	case 'B':                               return 0x30;
	case 'C':                               return 0x2e;
	case 'D':                               return 0x20;
	case 'E':                               return 0x12;
	case 'F':                               return 0x21;
	case 'G':                               return 0x22;
	case 'H':                               return 0x23;
	case 'I':                               return 0x17;
	case 'J':                               return 0x24;
	case 'K':                               return 0x25;
	case 'L':                               return 0x26;
	case 'M':                               return 0x32;
	case 'N':                               return 0x31;
	case 'O':                               return 0x18;
	case 'P':                               return 0x19;
	case 'Q':                               return 0x10;
	case 'R':                               return 0x13;
	case 'S':                               return 0x1f;
	case 'T':                               return 0x14;
	case 'U':                               return 0x16;
	case 'V':                               return 0x2f;
	case 'W':                               return 0x11;
	case 'X':                               return 0x2d;
	case 'Y':                               return 0x15;
	case 'Z':                               return 0x2c;
	case VK_LWIN:                           return 0xe05b;
	case VK_RWIN:                           return 0xe05c;
	case VK_APPS:                           return 0xe05d;
	case VK_SLEEP:                          return 0x5f;
	case VK_NUMPAD0:                        return 0x52;
	case VK_NUMPAD1:                        return 0x4f;
	case VK_NUMPAD2:                        return 0x50;
	case VK_NUMPAD3:                        return 0x51;
	case VK_NUMPAD4:                        return 0x4b;
	case VK_NUMPAD5:                        return 0x4c;
	case VK_NUMPAD6:                        return 0x4d;
	case VK_NUMPAD7:                        return 0x47;
	case VK_NUMPAD8:                        return 0x48;
	case VK_NUMPAD9:                        return 0x49;
	case VK_ADD:                            return 0x4e;
	case VK_MULTIPLY:                       return 0x37;
	case VK_SEPARATOR:                      return 0xe01c;
	case VK_SUBTRACT:                       return 0x4a;
	case VK_DECIMAL:                        return 0x53;
	case VK_DIVIDE:                         return 0x35;
	case VK_F1:                             return 0x3b;
	case VK_F2:                             return 0x3c;
	case VK_F3:                             return 0x3d;
	case VK_F4:                             return 0x3e;
	case VK_F5:                             return 0x3f;
	case VK_F6:                             return 0x40;
	case VK_F7:                             return 0x41;
	case VK_F8:                             return 0x42;
	case VK_F9:                             return 0x43;
	case VK_F10:                            return 0x44;
	case VK_F11:                            return 0x57;
	case VK_F12:                            return 0x58;
	case VK_F13:                            return 0x5b;
	case VK_F14:                            return 0x5c;
	case VK_F15:                            return 0x5d;
	case VK_F16:                            return 0x63;
	case VK_F17:                            return 0x64;
	case VK_F18:                            return 0x65;
	case VK_F19:                            return 0x66;
	case VK_F20:                            return 0x67;
	case VK_F21:                            return 0x68;
	case VK_F22:                            return 0x69;
	case VK_F23:                            return 0x6a;
	case VK_F24:                            return 0x6b;
	case VK_NUMLOCK:                        return 0x45;
	case VK_SCROLL:                         return 0x46;
	case VK_LSHIFT:                         return 0x2a;
	case VK_RSHIFT:                         return 0x36;
	case VK_LCONTROL:                       return 0x1d;
	case VK_RCONTROL:                       return 0xe01d;
	case VK_LMENU:                          return 0x38;
	case VK_RMENU:                          return 0xe038;
	case VK_VOLUME_MUTE:                    return 0x20;
	case VK_VOLUME_DOWN:                    return 0x2e;
	case VK_VOLUME_UP:                      return 0x30;
	case VK_MEDIA_NEXT_TRACK:               return 0x19;
	case VK_MEDIA_PREV_TRACK:               return 0x10;
	case VK_MEDIA_STOP:                     return 0x24;
	case VK_MEDIA_PLAY_PAUSE:               return 0x22;
	case VK_LAUNCH_MAIL:                    return 0x1e;
	case VK_OEM_1:                          return 0x27; // ';:' for US
	case VK_OEM_PLUS:                       return 0x0d; // '+' any country
	case VK_OEM_COMMA:                      return 0x33; // ',' any country
	case VK_OEM_MINUS:                      return 0x0c; // '-' any country
	case VK_OEM_PERIOD:                     return 0x34; // '.' any country
	case VK_OEM_2:                          return 0x35; // '/?' for US
	case VK_OEM_3:                          return 0x29; // '`~' for US
	case VK_OEM_4:                          return 0x1a; //  '[{' for US
	case VK_OEM_5:                          return 0x2b; //  '\|' for US
	case VK_OEM_6:                          return 0x1b; //  ']}' for US
	case VK_OEM_7:                          return 0x28; //  ''"' for US
	case VK_OEM_WSCTRL:                     return 0x6c;
	case VK_OEM_COPY:                       return 0x6f;
	case VK_OEM_ENLW:                       return 0x75;
	case VK_ATTN:                           return 0x71;
	case VK_CRSEL:                          return 0x72;
	case VK_EXSEL:                          return 0x74;
	case VK_OEM_CLEAR:                      return 0x76;

	// keys with no common scan code
	/*
	case VK_LBUTTON:                        return 0x00;
	case VK_RBUTTON:                        return 0x00;
	case VK_CANCEL:                         return 0x00;
	case VK_MBUTTON:                        return 0x00;  // NOT contiguous with L & RBUTTON
	case VK_XBUTTON1:                       return 0x00;  // NOT contiguous with L & RBUTTON
	case VK_XBUTTON2:                       return 0x00;  // NOT contiguous with L & RBUTTON
	case VK_CLEAR:                          return 0x00;
	case VK_IME_ON:                         return 0x00;
	case VK_JUNJA:                          return 0x00;
	case VK_FINAL:                          return 0x00;
	case VK_IME_OFF:                        return 0x00;
	case VK_CONVERT:                        return 0x00;
	case VK_NONCONVERT:                     return 0x00;
	case VK_ACCEPT:                         return 0x00;
	case VK_MODECHANGE:                     return 0x00;
	case VK_SELECT:                         return 0x00;
	case VK_PRINT:                          return 0x00;
	case VK_EXECUTE:                        return 0x00;
	case VK_HELP:                           return 0x00;
	case VK_NAVIGATION_VIEW:                return 0x00; // reserved
	case VK_NAVIGATION_MENU:                return 0x00; // reserved
	case VK_NAVIGATION_UP:                  return 0x00; // reserved
	case VK_NAVIGATION_DOWN:                return 0x00; // reserved
	case VK_NAVIGATION_LEFT:                return 0x00; // reserved
	case VK_NAVIGATION_RIGHT:               return 0x00; // reserved
	case VK_NAVIGATION_ACCEPT:              return 0x00; // reserved
	case VK_NAVIGATION_CANCEL:              return 0x00; // reserved
	case VK_OEM_NEC_EQUAL:                  return 0x00; // '=' key on numpad
	case VK_OEM_FJ_MASSHOU:                 return 0x00;
	case VK_OEM_FJ_TOUROKU:                 return 0x00;
	case VK_OEM_FJ_LOYA:                    return 0x00; // 'Left OYAYUBI' key
	case VK_OEM_FJ_ROYA:                    return 0x00; // 'Right OYAYUBI' key
	case VK_BROWSER_BACK:                   return 0x00;
	case VK_BROWSER_FORWARD:                return 0x00;
	case VK_BROWSER_REFRESH:                return 0x00;
	case VK_BROWSER_STOP:                   return 0x00;
	case VK_BROWSER_SEARCH:                 return 0x00;
	case VK_BROWSER_FAVORITES:              return 0x00;
	case VK_BROWSER_HOME:                   return 0x00;
	case VK_LAUNCH_MEDIA_SELECT:            return 0x00;
	case VK_LAUNCH_APP1:                    return 0x00;
	case VK_LAUNCH_APP2:                    return 0x00;
	case VK_GAMEPAD_A:                      return 0x00; // reserved
	case VK_GAMEPAD_B:                      return 0x00; // reserved
	case VK_GAMEPAD_X:                      return 0x00; // reserved
	case VK_GAMEPAD_Y:                      return 0x00; // reserved
	case VK_GAMEPAD_RIGHT_SHOULDER:         return 0x00; // reserved
	case VK_GAMEPAD_LEFT_SHOULDER:          return 0x00; // reserved
	case VK_GAMEPAD_LEFT_TRIGGER:           return 0x00; // reserved
	case VK_GAMEPAD_RIGHT_TRIGGER:          return 0x00; // reserved
	case VK_GAMEPAD_DPAD_UP:                return 0x00; // reserved
	case VK_GAMEPAD_DPAD_DOWN:              return 0x00; // reserved
	case VK_GAMEPAD_DPAD_LEFT:              return 0x00; // reserved
	case VK_GAMEPAD_DPAD_RIGHT:             return 0x00; // reserved
	case VK_GAMEPAD_MENU:                   return 0x00; // reserved
	case VK_GAMEPAD_VIEW:                   return 0x00; // reserved
	case VK_GAMEPAD_LEFT_THUMBSTICK_BUTTON: return 0x00; // reserved
	case VK_GAMEPAD_RIGHT_THUMBSTICK_BUTTON:                                        return 0x00; // reserved
	case VK_GAMEPAD_LEFT_THUMBSTICK_UP:     return 0x00; // reserved
	case VK_GAMEPAD_LEFT_THUMBSTICK_DOWN:   return 0x00; // reserved
	case VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT:  return 0x00; // reserved
	case VK_GAMEPAD_LEFT_THUMBSTICK_LEFT:   return 0x00; // reserved
	case VK_GAMEPAD_RIGHT_THUMBSTICK_UP:    return 0x00; // reserved
	case VK_GAMEPAD_RIGHT_THUMBSTICK_DOWN:  return 0x00; // reserved
	case VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT: return 0x00; // reserved
	case VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT:  return 0x00; // reserved
	case VK_OEM_8:                          return 0x00;
	case VK_OEM_AX:                         return 0x00; //  'AX' key on Japanese AX kbd
	case VK_OEM_102:                        return 0x00; //  "<>" or "\|" on RT 102-key kbd.
	case VK_ICO_HELP:                       return 0x00; //  Help key on ICO
	case VK_ICO_00:                         return 0x00; //  00 key on ICO
	case VK_PROCESSKEY:                     return 0x00;
	case VK_ICO_CLEAR:                      return 0x00;
	case VK_PACKET:                         return 0x00;
	case VK_OEM_RESET:                      return 0x00;
	case VK_OEM_JUMP:                       return 0x00;
	case VK_OEM_PA1:                        return 0x00;
	case VK_OEM_PA2:                        return 0x00;
	case VK_OEM_PA3:                        return 0x00;
	case VK_OEM_CUSEL:                      return 0x00;
	case VK_OEM_ATTN:                       return 0x00;
	case VK_OEM_FINISH:                     return 0x00;
	case VK_OEM_AUTO:                       return 0x00;
	case VK_OEM_BACKTAB:                    return 0x00;
	case VK_EREOF:                          return 0x00;
	case VK_PLAY:                           return 0x00;
	case VK_ZOOM:                           return 0x00;
	case VK_NONAME:                         return 0x00;
	case VK_PA1:                            return 0x00;
	*/

	default: return 0x00;
	}
}