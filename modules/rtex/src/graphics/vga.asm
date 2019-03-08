;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;
;;; Subroutines that have to do with handling video modes.
;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Waits for vertical refresh (vsync).
;;;
;;; EXPECTS:
;;;		(- nothing)
;;; DESTROYS:
;;;		- dx
;;; RETURNS:
;;;		(- nothing)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Wait_For_VSync:
	mov dx,3dah
	.wait_1:
	    in al,dx
	    and al,00001000b
	    jnz .wait_1
	.wait_2:
	    in al,dx
	    and al,00001000b
	    jz .wait_2
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Assigns the palette colors in VGA mode 13h from the 'palette' array (defined externally).
;;;
;;; EXPECTS:
;;;     - existance of a 'palette' array, which is 256 * 3 db and gives the red, green, and blue components
;;;       of each palette entry (value range 0-63 for each channel)
;;;     - the current video mode must be set to mode 13h before calling this subroutine
;;;     - ds:si to point to the beginning of the palette array
;;; DESTROYS:
;;;     - al, cl, dx, si, ebx
;;; RETURNS:
;;;     (- nothing)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Set_Palette_13H:
    xor cl,cl
    call Wait_For_VSync
    index:
        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        ; load the next color into a register. we load all three 8-bit components (r,g,b) at once,
        ; plus one exra that we ignore (as we don't have a 24-bit register to get just the three components).
        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        mov ebx,dword [ds:si]               ; bl = red, bh = green, low 8 bits above bh = blue.
        add si,3                            ; move the array pointer to the beginning of the next palette entry.

        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        ; set which palette index to modify.
        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        mov al,cl
        mov dx,3c8h
        out dx,al

        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        ; set the color (r, g, b) at that index.
        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        mov dx,3c9h
        mov al,bl
        out dx,al
        mov al,bh
        out dx,al
        shr ebx,8                           ; shift right to move the 8 bits above bh into bh.
        mov al,bh
        out dx,al

        inc cl
        jnz index                           ; stop looping when cl==0, i.e. when the counter loops from ..., 253, 254, 255 to 0.
    ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Redraws the entire screen.
;;;
;;; EXPECTS:
;;;     (- unknown)
;;; DESTROYS:
;;;     (- unknown)
;;; RETURNS:
;;;     (- unknown)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Redraw_All:
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; if there are unsaved changes, we want to make sure to redraw the marker as well. so check to see
    ; if the marker is on screen, and save that info.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov al,[es:(SCREEN_W * 3) + 5]          ; save the color of the screen where the unsaved changes marker is.
    push ax

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; set to draw everything into the video buffer, so we can use double buffering to eliminate flicker.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    push es
    mov ax,@VGA_BUFFER
    mov es,ax

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; clear the video buffer.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov di,vga_buffer
    mov eax,65656565h                       ; color to clear with (65h = black).
    mov cx,3e80h                            ; how many bytes to clear (320*200 = 64000/4 = 16000).
    rep stosd                               ; clear the screen in four-byte steps.

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; fill the video buffer with the ui's controls.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    call Draw_Palette_Selector
    call Draw_Palat_Selector
    call Draw_Project_Title
    call Draw_Current_Pala_ID
    call Draw_Pala_Editor
    call Save_Mouse_Cursor_Background       ; prevent a black box in the upper left corner of the screen on startup.

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; draw a halo around the currently selected pala's thumbnail in the palat selector.
    ; on startup, its position is marked in prev_pala_thumb_offs.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov di,[prev_pala_thumb_offs]
    mov eax,05050505h                       ; the frame's color.
    call Draw_Pala_Thumb_Halo

    call Draw_Mouse_Cursor

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; wait for vsync, then copy the video buffer into video memory and so onto the display.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    call Flip_Video_Buffer

    pop es

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; see whether we need to redraw the unsaved changes marker.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    pop ax
    .test_unsaved:
    cmp al,[str_unsaved_changes]            ; compare to the color of the unsaved changes marker.
    jne .test_save_error
    call Draw_Unsaved_Changes_Marker
    jmp .exit
    .test_save_error:
    cmp al,[str_unsaved_changes_err]        ; compare to the color of the save error marker.
    jne .exit
    call Draw_Save_Error_Marker
    jmp .exit

    .exit:
    ret
