#pragma once


#ifdef PLOT_EXPORTS

#define PLOAPI __declspec(dllexport)

#else

#define PLOAPI __declspec(dllimport)

#endif
