#ifndef _COMMON_H_
#define _COMMON_H_

// byte and uint32_t are not defined in header files so we define them here
// define task struct allowing inter core communication with queue
struct LedState
{
	bool on;
	bool blend;
	char mode;
	char brightness;
	char palette;
	unsigned int color;
};

#endif
