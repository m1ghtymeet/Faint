#include <Analytics/GPUInfo.h>

//FORCE_DEDICATED_GPU

#ifdef FT_DEBUG
int main(int argc, char* argv[]) {
#else
#undef APIENTRY
#include "Windows.h"
INT WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow) {
#endif
	
	return EXIT_SUCCESS;
}