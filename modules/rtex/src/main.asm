;
; 2017 Tarpeeksi Hyvae Soft /
; RTEX
;
; A texture editor for Rally-Sport (the DOS game from 1996).
;
; Expects to be assembled with FASM.
;

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; constants.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DEBUG_MODE          = 0                     ; set to 1 if the debug mode is enabled.
BASE_MEM_REQUIRED   = 260                   ; how much base (conventional) memory the program needs. if the user has less, the program exits.
VGA_BUFFER_SIZE     = 0fa00h
PALA_BUFFER_SIZE    = 65024                 ; how many bytes of data from the PALA file we'll load and handle.
VRAM_SEG            = 0a000h                ; address of the video ram segment in vga mode 13h.
TRANSPARENT_COLOR   = 0                     ; transparent color index.
SCREEN_W            = 320                   ; screen resolution (vga mode 13h).
SCREEN_H            = 200                   ;
TIMER_RES           = 5                     ; timer resolution, in milliseconds.
TIMER_TICKS_PER_SEC = 200                   ; how many times per second the timer interrupt is fired.
FONT_HEIGHT         = 6                     ; how many px tall the font is.
PALA_W              = 16                    ; dimensions of a pala texture.
PALA_H              = 16
CURSOR_W            = 10                    ; dimensions of the mouse cursor. NOTE: neither the width nor height should be more than 16 px.
CURSOR_H            = 13                    ; NOTE: if you change the cursor width, you need to change the cursor drawing routines as well.
NUM_PALA_THUMB_X    = 11                    ; the number of pala thumbnails horizontally in the palat selector.
NUM_PALA_THUMB_Y    = 23                    ; the number of pala thumbnails vertically in the palat selector.

format MZ

entry @CODE:start
; default stack size = 4096 bytes.

segment @CODE use16

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; includes.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
include "text/text.asm"
include "graphics/vga.asm"
include "graphics/draw_routines.asm"
include "input/mouse/mouse.asm"
include "input/keyboard/keyboard.asm"
include "file/file.asm"
include "cmd_line/cmd_line.asm"
include "editor/editor.asm"

start:
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
mov ax,DEBUG_MODE
cmp ax,1
je .assign_segments

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
jmp .exit

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; assign segments.
; cs = code, ds = data, es = video memory buffer, fs = copy of ds, gs = palat file data as a flat array.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
.assign_segments:
mov ax,@BASE_DATA
mov ds,ax
mov fs,ax
mov ax,VRAM_SEG;@BUFFER_1
mov es,ax
mov ax,@BUFFER_2
mov gs,ax

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; exit if we don't have enough conventional memory.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
cmp [free_conventional_memory],BASE_MEM_REQUIRED
jge .load_sandbox
mov dx,err_low_memory
mov ah,9h
int 21h
jmp .exit

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; load the track's specific data from the sandboxed ~~LLYE.EXE. this assumes that rsed_ldr has first been run
; to sandbox the RALLYE.EXE file and apply the track's manifesto to it.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
.load_sandbox:
call Load_Sandbox_Data
cmp al,0
jne .load_palat
mov ah,9h
mov dx,err_sandbox
int 21h
jmp .exit

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; load the palat file. also check to see if there was an error loading it, and if so, exit.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
.load_palat:
call Load_Palat_File
cmp al,0
jne .enable_mouse
mov ah,9h
mov dx,err_palat_load
int 21h
jmp .exit

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; enable the mouse. the routine will return 0 in ax if it failed.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
.enable_mouse:
call Enable_Mouse
cmp ax,0
jne .got_mouse
mov dx,err_mouse_init                       ; if we failed to acquire the mouse, display an error message and exit.
mov ah,9h
int 21h
jmp .exit
.got_mouse:

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; set the keyboard key repeat delay rate low.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
mov ah,3
mov al,4
int 16h

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; initialize vga mode to 13h for graphics.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
mov ah,0                                    ; set to change the video mode.
mov al,13h                                  ; the video mode we want.
int 10h                                     ; change the video mode.

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; set the game's palette.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
mov si,palette
call Set_Palette_13H

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; clear the video buffer.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
mov di,vga_buffer
mov eax,65656565h                   ; color to clear with (65h = black).
mov cx,3e80h                        ; how many bytes to clear (320*200 = 64000/4 = 16000).
rep stosd                           ; clear the screen in four-byte steps.

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; enable our own timer interrupt.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;call Set_Timer_Interrupt_Handler

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; clear the screen and fill the video buffer with the ui's controls.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
call Redraw_All

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; loop until the user presses the right mouse button.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
.main_loop:
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; wait until the monitor's refresh period is over, then start processing the next frame
    ; while the monitor isn't drawing the contents of the video memory onto the screen.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    call Wait_For_VSync

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; get info on which keyboard button is being held down, and deal with any
    ; keyboard input (like saving the data).
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    call Poll_Keyboard_Status
    call Handle_Editor_Keyboard_Input
    cmp [key_pressed],KEY_ZERO
    je .init_exit
    mov [key_pressed],KEY_NO_KEY            ; any keys pressed have been handled, so discard them.

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; get the current mouse position and its button status.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    call Poll_Mouse_Status

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; remove the mouse cursor from screen by redrawing its background over the cursor graphic.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    call Erase_Mouse_Cursor

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; process any mouse clicks the user may have made.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    call Handle_Editor_Mouse_Click          ; process the mouse click in some way.

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; draw the mouse cursor on screen.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    call Draw_Mouse_Cursor

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; copy the vga buffer to video memory.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ;mov si,vga_buffer
    ;call Flip_Video_Buffer

    jmp .main_loop
; END OF MAIN LOOP.
;;;;;;;;;;;;;;;;;;;;

.init_exit:
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; make sure dos gets its own timer handler back.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;call Restore_Timer_Interrupt_Handler

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; initialize the video mode back to text.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
mov ah,0                                    ; set to change the video mode.
mov al,3                                    ; the video mode we want.
int 10h                                     ; change the video mode.

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; close the project's .dta file.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
mov ah,3eh                              ; set to close.
mov bx,[fh_project_file]
int 21h

.exit:
mov ah,4ch
mov al,0
int 21h
; END OF PROGRAM.
;;;;;;;;;;;;;;;;;;



segment @BASE_DATA
    free_conventional_memory dw 0           ; the amount of free conventional memory (in kb) at program startup.

    tmp_int_str db "m999",0                 ; a temporary buffer used when printing integers to the screen.

    ; mouse.
    mouse_pos_xy dd 0                       ; the x,y coordinates of the mouse cursor.
    prev_mouse_pos_xy dd 0                  ; the mouse's x,y coordinates in the previous frame.
    mouse_buttons dw 0                      ; mouse button status.

    mouse_inside_edit db 0                  ; set to 1 if the mouse is within the pala texture in the edit field.
    mouse_pos_edit_xy dw 0                  ; the position of the mouse cursor relative to the edit segment.

    mouse_inside_palette db 0               ; set to 1 if the mouse is within the color swatches in the color selector.
    mouse_pos_palette_y db 0                ; the position of the mouse cursor relative to the palette segment.

    mouse_inside_palat db 0                 ; set to 1 if the mouse is within the palat selector.
    mouse_pos_palat_xy dw 0                 ; the position of the mouse cursor relative to the palat segment.
    prev_pala_thumb_offs dw 2587            ; the pixel offset on the screen of the top left corner of the thumbnail of the previously selected pala.
                                            ; it's set by default to the 4th thumbnail.

    ; editing.
    magnification db 12                     ; by how much the current pala should be magnified.
    selected_pala db 3                      ; the index in the PALAT file of the pala we've selected for editing.
    selected_pala_offset dw 16*16*3         ; pre-computed offset from the start of the PALA file data for the currently selected pala.
    hovering_pala db 0                      ; the pala over which the mouse is hovering in the palat selector.
    pen_color db 5                          ; which palette index the pen is painting with.
    click_in_segment db 0                   ; which segment (palat picker, editor, palette) the mouse has been clicked in. if 0, none.

    ; file handles.
    fh_project_file dw 0
    fh_sb_rallye_exe dw 0

    file_pala_data_start dd 0               ; the byte offset in the project's file where the palat file's data starts.

    ; track info.
    track_id db 0                               ; which track we have (0-7).
    palat_id db 0                               ; which palat file we use (0-1).
    track_palette_id db 0,0,0,0,1,2,0,3         ; which of the game's four palettes the given track uses.
    palette_offset dd 202d6h,20336h,20396h,203f6h ; the byte offset at which the xth palette begins in RALLYE.EXE.

    ; STRINGS
    ; error messages.
    err_sandbox db "ERROR: Failed to access sandboxed information. Exiting.",0ah,0dh
                db "   Make sure you've run rsed_ldr first.",0ah,0dh,"$"
    err_mouse_init db "ERROR: Failed to initialize the mouse. Exiting.",0ah,0dh
                   db "   Make sure your mouse is installed and that its driver is active.",0ah,0dh,"$"
    err_palat_load db "ERROR: Failed to load data from the project file. Exiting.",0ah,0dh,"$"
    err_low_memory db "ERROR: Not enough free conventional memory to run the program. Exiting.",0ah,0dh
                   db "   Try to have at least 260 KB of free memory.",0ah,0dh,"$"
    err_bad_cmd_line db "ERROR: Malformed command line argument. Exiting.",0ah,0dh,"$"

    ; ui messages.
    message_str db "cCURRENT PALA:    .",0
    project_name_str db "cDEBUG",0,0,0,0
    pala_file_str db "cPALAT.001",0         ; the name of the palat file we're editing. for cosmetic purposes.
    str_unsaved_changes db "f*",0           ; a little indicator shown next to the project name when there are unsaved changes.
    str_unsaved_changes_err db "g*",0       ; a little indicator shown when saving changes fails.

    ; info messages.
    str_cmd_argument_info db "RallySportED Texture Editor v.7 / October 2017.",0ah,0dh
                          db "Expected command line usage: rsed_tex <track name>",0ah,0dh
                          db "The track name can be of up to eight ASCII characters from A-Z.",0ah,0dh,"$"

    ; file info.
    fn_project_file db "KLOROFYL\KLOROFYL.DTA",0,0,0,0 ; the name and path to the project file. this is changed later by the program to adjust to the project we want to open.
    fn_sb_rallye_exe db "~~LLYE.EXE",0      ; the name of the game's main executable, RALLYE.EXE, sandboxed to ~~LLYE.EXE.
    project_name db 0,0,0,0,0,0,0,0,0,"$"   ; the name of the project we're loading data from.
    project_name_len db 0                   ; the number of characters in the project name, excluding the null terminator.
    palat_file_name db "PALAT.001",0,0ah,0dh,"$" ; the name of the actual file we'll load the palat data from.
    project_file_name rb 22                 ; the name and path to the project file, which is 'proj_name\proj_name.xxx'.
    project_file_ext_offset dw 0            ; the offset in project_file_name where the file's 3-character extension begins.

    ; keyboard stuff.
    key_pressed db 0                        ; which key on the keyboard is currently pressed down.
    key_lock db 0                           ; if set to 1, we don't accept new input from the keyboard.

    include "text/font.inc"                 ; the character set for the text renderer.
    include "graphics/palette.inc"          ; the graphics palette.
    include "input/mouse/mouse_cursor.inc"  ; the mouse cursor image.

segment @VGA_BUFFER
    vga_buffer rb VGA_BUFFER_SIZE           ; we draw into this buffer, then flip it onto the screen at the end of the frame.
                                            ; NOTE: this buffer needs to start at offset 0 in its segment. this is because we might have decided to
                                            ; disable double buffering, in which case we'll be writing directly into vga video memory, which'll be
                                            ; at offset 0 of its segment, and this way we don't need to change code that aligns the offset to
                                            ; the vga_buffer buffer.

segment @BUFFER_2
    pala_data rb PALA_BUFFER_SIZE           ; the texture pixel data loaded from the palat file is stored here.

    cursor_background rb (CURSOR_W * CURSOR_H) ; used to store the background of the mouse cursor, so we can erase the cursor without redrawing the whole screen.

segment @BUFFER_3
    pala_data_backup rb PALA_BUFFER_SIZE    ; we store a copy of the palat data here, for undo.

