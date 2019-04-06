/*
 * 2019 Tarpeeksi Hyvae Soft /
 * RallySportED-DOS, RGEO
 * 
 * Include this file in a unit to disable debug message output in that unit.
 * 
 */

#ifndef DBG_HUSH_
#define DBH_HUSH_

#undef DEBUG
#define DEBUG(args) ((void)0)

#endif
