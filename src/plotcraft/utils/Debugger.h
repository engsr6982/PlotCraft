#pragma once

#ifdef DEBUG
#define debugger(...) std::cout << "[Debug] " << __VA_ARGS__ << std::endl;
#else
#define debugger(...) ((void)0)
#endif
