;
; 2017 Tarpeeksi Hyvae Soft /
; RLOAD
;
; Extracts RallySportED map assets from an RSED project and sets up a sandbox for Rally-Sport to run them.
;
; Expects to be assembled with FASM.
;

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; constants.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DEBUG_ENABLED               = 0             ; set to 1 to enable certain debug features.
LOADER_MAJOR_VERSION        = 4             ; the loader's major version number. this will be matched against the version required by the given track.
BASE_MEM_REQUIRED           = 80            ; how much base memory (KB) the program needs. if the user has less, the program exits.
MAX_MANIFESTO_PARAMS        = 10            ; how many parameters (counting the command itself) a manifesto command line can have.
FILE_BUFFER_SIZE            = 53248         ; the size of the file buffer (in bytes), used when relocating disk data.

format MZ

entry @CODE:start
; default stack size = 4096 bytes.

segment @CODE use16

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; includes.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
include "file/file.asm"
include "cmd_line/cmd_line.asm"
include "manifesto/manifesto.asm"
include "manifesto/manifesto_cmd.asm"
include "base_patches.asm"

start:
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; delete the temporary flag file, if it exists.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
mov ah,41h
mov dx,fn_flag
int 21h

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; get the amount of free conventional memory, by looking at the dos psp (program segment prefix).
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
mov ax,@BASE_DATA
mov gs,ax
mov bx,ds                                   ; get the address of the psp.
mov ax,[ds:2]                               ; get from the psp the last paragraph allocated to the program.
sub ax,bx
shr ax,6                                    ; convert to kilobytes.
mov [gs:free_conventional_memory],ax

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; skip parsing the command line if we have debugging enabled. if we don't do this, the dosbox debugger wigs out.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
mov ax,DEBUG_ENABLED
cmp ax,1
je .skip_command_line

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; parse the command line.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
call Parse_Command_Line
cmp al,1
je .assign_segments

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; if we failed to parse the command line, display an error message and exit.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
mov dx,str_cmd_argument_info
mov ah,9h
int 21h
mov dx,err_bad_cmd_line
mov ah,9h
int 21h
jmp .exit                                   ; this needs to be commented out if using the dosbox debugger.

.skip_command_line:

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; assign segments.
; cs = code, ds = data.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
.assign_segments:
mov ax,@BASE_DATA
mov ds,ax

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; exit if we don't have enough conventional memory.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
cmp [free_conventional_memory],BASE_MEM_REQUIRED
jge .sandbox_rallye_exe
mov dx,err_low_memory
mov ah,9h
int 21h
jmp .exit

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; put the game's executable assets into a sandbox environment, where we can path them without
; messing with the user's real files.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
.sandbox_rallye_exe:
call Sandbox_RALLYE_EXE
cmp al,1
je .sandbox_valikko_exe
mov dx,fn_rallye_exe
mov ah,9h
int 21h
mov dx,err_file_fail
mov ah,9h
int 21h
jmp .exit

.sandbox_valikko_exe:
call Sandbox_VALIKKO_EXE
cmp al,1
je .sandbox_results_dta
mov dx,fn_valikko_exe
mov ah,9h
int 21h
mov dx,err_file_fail
mov ah,9h
int 21h
jmp .exit

.sandbox_results_dta:
call Sandbox_RESULTS_DTA
cmp al,1
je .apply_manifesto
mov dx,fn_sb_results_dta
mov ah,9h
int 21h
mov dx,err_file_fail
mov ah,9h
int 21h
jmp .exit

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; apply the manifesto file.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
.apply_manifesto:
mov bx,[project_file_ext_offset]
mov byte [fn_project_file+bx+0],'$'         ; apply the correct file suffix.
mov byte [fn_project_file+bx+1],'F'
mov byte [fn_project_file+bx+2],'T'
call Apply_Manifesto
cmp al,1
je .apply_base_patches
mov dx,[err_manifesto_additional]           ; pointer to an additional error message.
mov ah,9h
int 21h
mov dx,err_manifesto_fail
mov ah,9h
int 21h
jmp .exit

.apply_base_patches:
call Apply_Base_Patches
cmp al,1
je .extract_project_files
mov dx,err_base_patch_fail
mov ah,9h
int 21h
jmp .exit

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; update the track's file's names based on which track and palat ids we're using.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
.extract_project_files:
mov al,[track_id]
mov ah,[palat_id]
add al,'1'                                  ; convert to ascii, 1-indexed.
add ah,'1'
mov byte [fn_sb_maasto_dta+9],al
mov byte [fn_sb_varimaa_dta+10],al
mov byte [fn_sb_kierros_dta+7],al
mov byte [fn_sb_palat_dta+8],ah

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; extract the project's files from the project's .dta file.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
mov bx,[project_file_ext_offset]
mov byte [fn_project_file+bx+0],'D'         ; apply the correct file suffix.
mov byte [fn_project_file+bx+1],'T'
mov byte [fn_project_file+bx+2],'A'
call Extract_Project_Files
cmp al,1
je .manifesto_done
mov dx,fn_project_file
mov ah,9h
int 21h
mov dx,err_file_fail
mov ah,9h
int 21h
jmp .exit

.manifesto_done:
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; close the sandboxed executable files.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
mov ah,3eh                              ; set to close.
mov bx,[fh_sb_rallye_exe]
int 21h
mov ah,3eh                              ; set to close.
mov bx,[fh_sb_valikko_exe]
int 21h

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; create a flag file to tell that we loaded the editor successfully.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
xor cx,cx
mov dx,fn_flag
mov ah,3ch                              ; set to create/truncate.
int 21h                                 ; do it.
mov bx,ax
mov ah,3eh                              ; set to close.
int 21h                                 ; do it.
pop ax

.exit:
mov ah,4ch
mov al,0
int 21h

; end of program



segment @BASE_DATA
    free_conventional_memory dw 0               ; the amount of free conventional memory (in KB) available on program startup.

    ; file names. sb = sandboxed version.
    fn_project_file db "HERBMARE\HERBMARE.DTA",0,0,0,0,0ah,0dh,'$' ; the name and path to the project file. this is changed later by the program to adjust to the project we want to open.

    fn_sb_rallye_exe db "~~LLYE.EXE",0,0ah,0dh,'$'
    fn_rallye_exe db "RALLYE.EXE",0

    fn_sb_valikko_exe db "~~LIKKO.EXE",0,0ah,0dh,'$'
    fn_valikko_exe db "VALIKKO.EXE",0

    fn_sb_results_dta db "~~SULTS.DTA",0,0ah,0dh,'$'

    fn_game_dta db "GAME.DTA",0

    fn_flag db "~~~.---",0

    fn_sb_maasto_dta db "~~ASTO.00x",0
    fn_sb_varimaa_dta db "~~RIMAA.00x",0
    fn_sb_palat_dta db "~~LAT.00x",0
    fn_sb_anims_dta db "~~IMS.DTA",0
    fn_sb_text1_dta db "~~XT1.DTA",0
    fn_sb_kierros_dta db "~~ERROSx.DTA",0

    ; file handles.
    fh_temp dw 0
    fh_sb_rallye_exe dw 0
    fh_rallye_exe dw 0
    fh_sb_valikko_exe dw 0
    fh_valikko_exe dw 0
    fh_project_file dw 0

    ; file info.
    project_name db "HERBMARE",0,"$"            ; the name of the project we're loading data from.
    project_name_len db 8                       ; the number of characters in the project name, excluding the null terminator.
    project_file_ext_offset dw 18               ; the offset in project_file_name where the file's 3-character extension begins.

    ; track info.
    track_id db 0                               ; which track we have (0-7).
    palat_id db 0                               ; which palat file we use (0-1).
    num_object_types db 16                      ; how many different objects (3d track props) there are in the game.
    max_num_objects_on_track db 14
    num_objects_on_track db 1                   ; how many objects there are on the current track. always at least 1 (the finish line).
    track_palette_id db 0,0,0,0,1,2,0,3         ; which of the game's four palettes the given track uses.
    palette_offset dd 202d6h,20336h,20396h,203f6h            ; the byte offset at which the xth palette begins in RALLYE.EXE.
    palette_offset_valikko dd 23e37h,23e97h,23ef7h,23f57h    ; the byte offset at which the xth palette begins in VALIKKO.EXE.
    object_block_offs dd 1510bh,15191h,1519fh,151d1h         ; the starting offset of the object block on each track in RALLYE.EXE.
                      dd 1527bh,15295h,152a3h,15341h
    object_block_offs_valikko dd 1a48ah,1a510h,1a51eh,1a550h ; the starting offset of the object block on each track in VALIKKO.EXE.
                              dd 1a5fah,1a614h,1a622h,1a6c0h
    object_header db 2ch,0d0h,88h,0d0h,46h,42h  ; tree.
                  db 0e2h,47h,98h,4ch,26h,42h   ; fence (chicken wire).
                  db 0e2h,47h,98h,4dh,26h,42h   ; fence (horse).
                  db 66h,47h,20h,4ch,0deh,42h   ; traffic sign (80).
                  db 66h,47h,5ch,4ch,0deh,42h   ; traffic sign (!).
                  db 0f2h,4fh,0e0h,51h,84h,42h  ; stone arch.
                  db 0bah,4ah,0ech,4ah,0feh,42h ; stone post.
                  db 32h,49h,0e4h,49h,0eh,43h   ; rock (large).
                  db 8eh,49h,0e4h,49h,0eh,43h   ; rock (small).
                  db 66h,44h,60h,45h,0a2h,42h   ; billboard (large).
                  db 0e6h,44h,60h,46h,0c0h,42h  ; billboard (small).
                  db 0eeh,48h,7ch,4bh,16h,42h   ; house.
                  db 24h,43h,4ah,43h,0eeh,42h   ; utility pole 1.
                  db 24h,43h,9ch,43h,0eeh,42h   ; utility pole 2.
                  db 88h,54h,02h,55h,66h,42h    ; starting line.
                  db 0d2h,50h,0c4h,51h,84h,42h  ; starting line (stone arch).
    object_header_valikko db 4dh,0dch,0a9h,0dch,55h,4eh ; tree.
                          db 0edh,59h,6bh,5bh,35h,4eh   ; fence (chicken wire).
                          db 0edh,59h,6bh,5ch,35h,4eh   ; fence (horse).
                          db 0c7h,4eh,0f3h,5ah,0b1h,4eh ; traffic sign (80).
                          db 0c7h,4eh,2fh,5bh,0b1h,4eh  ; traffic sign (!).
                          db 0bbh,51h,0a9h,53h,93h,4eh  ; stone arch.
                          db 0d3h,50h,05h,51h,95h,51h   ; stone post.
                          db 4bh,4fh,0fdh,4fh,0a5h,51h  ; rock (large).
                          db 0a7h,4fh,0fdh,4fh,0a5h,51h ; rock (small).
                          db 51h,56h,4bh,57h,09h,4fh    ; billboard (large).
                          db 0d1h,56h,4bh,57h,27h,4fh   ; billboard (small).
                          db 05h,59h,43h,59h,25h,4eh    ; house.
                          db 51h,58h,77h,58h,0f9h,4eh   ; utility pole 1.
                          db 51h,58h,0c9h,58h,0f9h,4eh  ; utility pole 2.
                          db 0c5h,5eh,3fh,5fh,75h,4eh   ; starting line.
                          db 9bh,52h,8dh,53h,93h,4eh    ; starting line (stone arch).
    track_titles db "Live race feed",0,0        ; the title of each track shown in the track info menu.
                 db "Live race feed",0,0
                 db "Live race feed",0,0
                 db "Live race feed",0,0
                 db "Live race feed",0,0
                 db "Live race feed",0,0
                 db "Live race feed",0,0
                 db "Live race feed",0,0

    ; error messages.
    err_low_memory db "ERROR: Not enough free conventional memory to run the program. Exiting.",0ah,0dh
                   db "   Try to have at least 80 KB of free memory.",0ah,0dh,"$"
    err_file_fail db "ERROR: Failed to access one or more of the project's files. Exiting.",0ah,0dh,'$'
    err_file_size db "Only the demo version of Rally-Sport is supported.",0ah,0dh,'$'
    err_base_patch_fail db "ERROR: Failed to apply base patches. Exiting.",0ah,0dh,'$'
    err_manifesto_general db "Encountered an unknown error while parsing the manifesto file.",0ah,0dh,'$'
    err_manifesto_additional dw err_manifesto_general               ; a pointer to an additional error message string relating to manifesto parsing errors.
    err_manifesto_fail db "ERROR: Failed to apply the manifesto. Exiting.",0ah,0dh,'$'
    err_manifesto_fail_version db "The track is incompatible with this version of the loader. You may need to",0ah,0dh
                               db "update the loader to a newer version.",0ah,0dh,'$'
    err_manifesto_parameters db "Encountered an unexpected number of parameters for a command in the manifesto",0ah,0dh
                             db "file. Check the file for typos, and if necessary, refer to RallySportED's",0ah,0dh
                             db "readme.txt for more info on the manifesto's format.",0ah,0dh,'$'
    err_manifesto_parameter_range db "Encountered an out-of-range value in a parameter in the manifesto file. Check",0ah,0dh
                                  db "the file for typos, and if necessary, refer to RallySportED's readme.txt for",0ah,0dh
                                  db "more info on the manifesto's format.",0ah,0dh,'$'
    err_manifesto_unknown_cmd db "Encountered an unknown command in the manifesto file. Either there is a typo",0ah,0dh
                              db "in the file or you may need an older version of the loader to play this track.",0ah,0dh,'$'
    err_bad_cmd_line db "ERROR: Malformed command line argument. Exiting.",0ah,0dh,"$"

    ; info messages.
    str_cmd_argument_info db "RallySportED Loader v4 by Tarpeeksi Hyvae Soft.",0ah,0dh
                          db "Expected command-line parameters: <track_name>",0ah,0dh
                          db "The track name can be of up to eight ASCII characters from A-Z.",0ah,0dh,"$"

    ; custom game menu texts.
    menu_track_select db "Track info",0,0,0     ; shown on the main menu screen.
    menu_track_info db "Track: ",0,0,0,0,0,0,0,0,0,0,0    ; shown on the track selection screen.

    ; for storing manifesto parameters.
    manifesto_cmd rb MAX_MANIFESTO_PARAMS       ; the command and its parameters on the current line of the manifesto.
    manifesto_cmd_len db 0                      ; how many parameters (counting the command itself) there are on the current line of the manifesto.

    ; misc.
    decimal_lut db 100,10,1                     ; a lookup table used to convert asciz to decimal.
    tmp_str db 0,0,0,0,"$"                      ; used for converting asciz to decimal.
    tmp dd 0                                    ; temporary storage.

    file_buffer rb FILE_BUFFER_SIZE             ; we load data from disk into this buffer.
