#pragma once

namespace Faint {

	using KeyCode = uint16_t;

	namespace Key {
		enum : KeyCode {
			Space =      32,
			Apostrophe = 39,
			Comma =      44,
			Minus =      45,
			Period =     46,
			Slash =      47,

			Number_0 =   48,
			Number_1 =   49,
			Number_2 =   50,
			Number_3 =   51,
			Number_4 =   52,
			Number_5 =   53,
			Number_6 =   54,
			Number_7 =   55,
			Number_8 =   56,
			Number_9 =   57,

			Semicolom     = 59,  /* ; */
			Equal         = 61,  /* = */
			A             = 65,
			B             = 66,
			C             = 67,
			D             = 68,
			E             = 69,
			F             = 70,
			G             = 71,
			H             = 72,
			I             = 73,
			J             = 74,
			K             = 75,
			L             = 76,
			M             = 77,
			N             = 78,
			O             = 79,
			P             = 80,
			Q             = 81,
			R             = 82,
			S             = 83,
			T             = 84,
			U             = 85,
			V             = 86,
			W             = 87,
			X             = 88,
			Y             = 89,
			Z             = 90,
			LeftBracket  = 91,  /* [ */
			Backslash     = 92,  /* \ */
			RightBracket = 93,  /* ] */
			GraveAccent  = 96,  /* ` */
			World_1       = 161, /* non-US #1 */
			World_2       = 162, /* non-US #2 */

			/* Function keys */
			Escape = 256,
			Enter = 257,
			Tab = 258,
			Backspace = 259,
			Insert = 260,
			Delete = 261,
			Right = 262,
			Left = 263,
			Down = 264,
			Up = 265,
			PageUp = 266,
			PageDown = 267,
			Home = 268,
			End = 269,
			CapsLock = 280,
			ScrollLock = 281,
			NumLock = 282,
			PrintScreen = 283,
			Pause = 284,
			F1 = 290,
			F2 = 291,
			F3 = 292,
			F4 = 293,
			F5 = 294,
			F6 = 295,
			F7 = 296,
			F8 = 297,
			F9 = 298,
			F10 = 299,
			F11 = 300,
			F12 = 301,
			F13 = 302,
			F14 = 303,
			F15 = 304,
			F16 = 305,
			F17 = 306,
			F18 = 307,
			F19 = 308,
			F20 = 309,
			F21 = 310,
			F22 = 311,
			F23 = 312,
			F24 = 313,
			F25 = 314,

			LeftShift    = 340, /* non-US #2 */
			LeftControl  = 341,
			LeftAlt      = 342,
			LeftSuper    = 343,
			RightShift   = 344, /* non-US #2 */
			RightControl = 345,
			RightAlt     = 346,
			RightSuper   = 347,
			Menu         = 348,
		};
	}
}

// From glfw3.h
#define HZ_KEY_SPACE              32
#define HZ_KEY_APOSTROPHE         39  /* ' */
#define HZ_KEY_COMMA              44  /* , */
#define HZ_KEY_MINUS              45  /* - */
#define HZ_KEY_PERIOD             46  /* . */
#define HZ_KEY_SLASH              47  /* / */
#define HZ_KEY_0                  48
#define HZ_KEY_1                  49
#define HZ_KEY_2                  50
#define HZ_KEY_3                  51
#define HZ_KEY_4                  52
#define HZ_KEY_5                  53
#define HZ_KEY_6                  54
#define HZ_KEY_7                  55
#define HZ_KEY_8                  56
#define HZ_KEY_9                  57
#define HZ_KEY_SEMICOLON          59  /* ; */
#define HZ_KEY_EQUAL              61  /* = */
#define HZ_KEY_A                  65
#define HZ_KEY_B                  66
#define HZ_KEY_C                  67
#define HZ_KEY_D                  68
#define HZ_KEY_E                  69
#define HZ_KEY_F                  70
#define HZ_KEY_G                  71
#define HZ_KEY_H                  72
#define HZ_KEY_I                  73
#define HZ_KEY_J                  74
#define HZ_KEY_K                  75
#define HZ_KEY_L                  76
#define HZ_KEY_M                  77
#define HZ_KEY_N                  78
#define HZ_KEY_O                  79
#define HZ_KEY_P                  80
#define HZ_KEY_Q                  81
#define HZ_KEY_R                  82
#define HZ_KEY_S                  83
#define HZ_KEY_T                  84
#define HZ_KEY_U                  85
#define HZ_KEY_V                  86
#define HZ_KEY_W                  87
#define HZ_KEY_X                  88
#define HZ_KEY_Y                  89
#define HZ_KEY_Z                  90
#define HZ_KEY_LEFT_BRACKET       91  /* [ */
#define HZ_KEY_BACKSLASH          92  /* \ */
#define HZ_KEY_RIGHT_BRACKET      93  /* ] */
#define HZ_KEY_GRAVE_ACCENT       96  /* ` */
#define HZ_KEY_WORLD_1            161 /* non-US #1 */
#define HZ_KEY_WORLD_2            162 /* non-US #2 */

/* Function keys */
#define HZ_KEY_ESCAPE             256
#define HZ_KEY_ENTER              257
#define HZ_KEY_TAB                258
#define HZ_KEY_BACKSPACE          259
#define HZ_KEY_INSERT             260
#define HZ_KEY_DELETE             261
#define HZ_KEY_RIGHT              262
#define HZ_KEY_LEFT               263
#define HZ_KEY_DOWN               264
#define HZ_KEY_UP                 265
#define HZ_KEY_PAGE_UP            266
#define HZ_KEY_PAGE_DOWN          267
#define HZ_KEY_HOME               268
#define HZ_KEY_END                269
#define HZ_KEY_CAPS_LOCK          280
#define HZ_KEY_SCROLL_LOCK        281
#define HZ_KEY_NUM_LOCK           282
#define HZ_KEY_PRINT_SCREEN       283
#define HZ_KEY_PAUSE              284
#define HZ_KEY_F1                 290
#define HZ_KEY_F2                 291
#define HZ_KEY_F3                 292
#define HZ_KEY_F4                 293
#define HZ_KEY_F5                 294
#define HZ_KEY_F6                 295
#define HZ_KEY_F7                 296
#define HZ_KEY_F8                 297
#define HZ_KEY_F9                 298
#define HZ_KEY_F10                299
#define HZ_KEY_F11                300
#define HZ_KEY_F12                301
#define HZ_KEY_F13                302
#define HZ_KEY_F14                303
#define HZ_KEY_F15                304
#define HZ_KEY_F16                305
#define HZ_KEY_F17                306
#define HZ_KEY_F18                307
#define HZ_KEY_F19                308
#define HZ_KEY_F20                309
#define HZ_KEY_F21                310
#define HZ_KEY_F22                311
#define HZ_KEY_F23                312
#define HZ_KEY_F24                313
#define HZ_KEY_F25                314
#define HZ_KEY_KP_0               320
#define HZ_KEY_KP_1               321
#define HZ_KEY_KP_2               322
#define HZ_KEY_KP_3               323
#define HZ_KEY_KP_4               324
#define HZ_KEY_KP_5               325
#define HZ_KEY_KP_6               326
#define HZ_KEY_KP_7               327
#define HZ_KEY_KP_8               328
#define HZ_KEY_KP_9               329
#define HZ_KEY_KP_DECIMAL         330
#define HZ_KEY_KP_DIVIDE          331
#define HZ_KEY_KP_MULTIPLY        332
#define HZ_KEY_KP_SUBTRACT        333
#define HZ_KEY_KP_ADD             334
#define HZ_KEY_KP_ENTER           335
#define HZ_KEY_KP_EQUAL           336
#define HZ_KEY_LEFT_SHIFT         340
#define HZ_KEY_LEFT_CONTROL       341
#define HZ_KEY_LEFT_ALT           342
#define HZ_KEY_LEFT_SUPER         343
#define HZ_KEY_RIGHT_SHIFT        344
#define HZ_KEY_RIGHT_CONTROL      345
#define HZ_KEY_RIGHT_ALT          346
#define HZ_KEY_RIGHT_SUPER        347
#define HZ_KEY_MENU               348

#define HZ_MOUSE_BUTTON_1         0
#define HZ_MOUSE_BUTTON_2         1
#define HZ_MOUSE_BUTTON_3         2
#define HZ_MOUSE_BUTTON_4         3
#define HZ_MOUSE_BUTTON_5         4
#define HZ_MOUSE_BUTTON_6         5
#define HZ_MOUSE_BUTTON_7         6
#define HZ_MOUSE_BUTTON_8         7
#define HZ_MOUSE_BUTTON_LAST      GLFW_MOUSE_BUTTON_8
#define HZ_MOUSE_BUTTON_LEFT      GLFW_MOUSE_BUTTON_1
#define HZ_MOUSE_BUTTON_RIGHT     GLFW_MOUSE_BUTTON_2
#define HZ_MOUSE_BUTTON_MIDDLE    GLFW_MOUSE_BUTTON_3