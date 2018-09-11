/*
 * 2018 Tarpeeksi Hyvae Soft /
 * RallySportED
 *
 */

#ifndef CMD_LINE_H
#define CMD_LINE_H

const char* kcmdl_track_name(void);

bool kcmdl_parse_command_line(const int argc, const char *const argv[]);

#endif
