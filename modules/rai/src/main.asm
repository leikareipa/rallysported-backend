;
; 2017 Tarpeeksi Hyvae Soft /
; RAI
;
; Lets you create a new CPU racer for your custom track in Rally-Sport.
; It does this by patching RALLYE.EXE so that on exit, it dumps the ghost driver's lap checkpoints onto disk.
;
; NOTE: This program assumes that you've already run RLOAD, which would've created
;       sandboxed versions of the game's files (e.g. RALLYE.EXE would be ~~LLYE.EXE or the like).
;
; Expects to be assembled with FASM.
;

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; constants.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DEBUG_ENABLED               = 0             ; set to 1 if you want to enable debug.
FILE_BUFFER_SIZE            = 53248         ; the size of the file buffer, in bytes.

format MZ

entry @CODE:start
; default stack size = 4096 bytes.

segment @CODE use16

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; includes.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
include "cmd_line/cmd_line.asm"
include "parse.asm"
include "patch.asm"

start:
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; skip parsing the command line if we have debugging enabled. if we don't do this, the dosbox debugger wigs out.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
mov ax,DEBUG_ENABLED
cmp ax,1
je .parse

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; parse the command line.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
call Parse_Command_Line
cmp al,2                                    ; al == 2 means the command line was empty, so we should patch the game executable.
je .patch
cmp al,1                                    ; al == 1 means there was a project name in the command line, so we should parse and save the ghost lap dump.
je .parse
mov dx,err_generic                          ; otherwise, an error occurred.
mov ah,9h
int 21h
jmp .exit_with_error

.patch:
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; patch ~~LLYE.EXE to make it so the game saves the ghost lap onto disk.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
mov ax,@BASE_DATA
mov ds,ax
call Patch_Rallye_Exe
cmp al,1
je .exit
mov dx,err_generic
mov ah,9h
int 21h
jmp .exit_with_error

.parse:
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; parse the extracted ghost checkpoint track.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
mov ax,@BASE_DATA
mov ds,ax
call Parse_Lap_Dump
cmp al,1
je .exit
mov dx,err_parse_fail
mov ah,9h
int 21h
jmp .exit_with_error

.exit:
mov ah,4ch
mov al,0
int 21h

.exit_with_error:
mov ah,4ch
mov al,1
int 21h

; end of program



segment @BASE_DATA
    ; track data.
    track_id db 0                               ; which track we have (0-7).
    starting_pos_ptr dd 1500dh,1500dh,1500dh,15015h ; byte offsets in RALLYE.EXE to player starting positions.
                     dd 1501eh,15025h,1502dh,15035h
    new_starting_pos db 40h,0bh                 ; we write the particular word to the starting_pos_ptr's location to move the player's
                     db 40h,0bh                 ; car to where the cpu starts from, since the cpu's car is somewhat offset from the player's.
                     db 40h,0bh
                     db 42h,0eh
                     db 10h,00h
                     db 80h,13h
                     db 00h,12h
                     db 00h,09h

    ; file names. sb = sandboxed version.
    fn_project_file db "HERBMARE\HERBMARE.DTA",0,0,0,0 ; the name and path to the project file. this is changed later by the program to adjust to the project we want to open.
    fn_sb_rallye_exe db "~~LLYE.EXE",0
    fn_sb_game_dta db "~~ME.DTA",0
    fn_lap_dump db "~~PONENT.DTA",0
    fn_lap_dump_len = $ - fn_lap_dump           ; how many bytes the lap dump file name is, including the 0 terminator.
    fn_lap_dump_converted db "KIERROS1.DTA",0

    ; file handles.
    fh_sb_rallye_exe dw 0
    fh_sb_game_dta dw 0
    fh_project_file dw 0
    fh_lap_dump dw 0

    ; file info.
    project_name db "HERBMARE",0,"$"            ; the name of the project we're loading data from.
    project_name_len db 8                       ; the number of characters in the project name, excluding the null terminator.
    project_file_ext_offset dw 18               ; the offset in project_file_name where the file's 3-character extension begins.

    ; error strings.
    err_generic db "RSED_CPU: Oops, something went wrong. Exiting.",0ah,0dh,'$'
    err_parse_fail db "ERROR: Failed to parse the lap dump. Exiting.",0ah,0dh,'$'

    ; misc.
    tmp db 0,0,0,0,0,0
    record_message db "Recording...",0          ; message show on screen when recording the cpu lap.
    record_message_len = $ - record_message

    file_buffer rb FILE_BUFFER_SIZE             ; we load data from disk into this buffer.
