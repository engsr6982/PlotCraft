#pragma once


#ifdef PLOT_EXPORTS

#define PLAPI __declspec(dllexport)

#else

#define PLAPI __declspec(dllimport)

#endif
