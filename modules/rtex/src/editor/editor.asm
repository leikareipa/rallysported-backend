;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;
;;; Subroutines for handling things to do with the editor, like mouse clicks, painting, saving, etc.
;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Deals with mouse movement.
;;;
;;; EXPECTS:
;;;     (- unknown)
;;; DESTROYS:
;;;     (- unknown)
;;; RETURNS:
;;;     (- unknown)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Handle_Editor_Mouse_Move:
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; find out which editor segment the mouse is in.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov ax,word[mouse_pos_xy+2]         ; ax = mouse y pos.
    cmp ax,90
    jl .in_palat
    cmp ax,290
    jl .in_edit
    jmp .in_palette

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; deal with clicks to the palat segment.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    .in_palat:
    jmp .exit

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; deal with clicks to the edit segment.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    .in_edit:
    jmp .exit

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; deal with clicks to the palette segment.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    .in_palette:
    jmp .exit

    .exit:
    ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Deals with mouse clicks.
;;;
;;; EXPECTS:
;;;     (- unknown)
;;; DESTROYS:
;;;     (- unknown)
;;; RETURNS:
;;;     (- unknown)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Handle_Editor_Mouse_Click:
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; see if any mouse buttons were pressed, and if not, we can exit.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ;test [mouse_buttons],01b               ; left button.
    cmp [mouse_buttons],0                   ; 0 = no buttons pressed.
    jne .handle_left_click

    mov [click_in_segment],0                ; signal that we haven't clicked in any segment.
    jmp .exit

    .handle_left_click:
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; find out which editor segment the mouse is in.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov ax,word[mouse_pos_xy+2]             ; ax = mouse y pos.
    cmp ax,90
    jl .in_palat
    cmp ax,290
    jl .in_edit
    jmp .in_palette

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; deal with clicks to the palat segment.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    .in_palat:
    cmp [click_in_segment],0                ; make sure we don't deal with clicks in this segment if the mouse is held down and was originally
    je .handle_palat                        ; ...clicked in another segment. this helps prevent cases where you're e.g. painting a pala and
    cmp [click_in_segment],1                ; ...accidentally move the mouse over the palat selector, which would then otherwise switch the pala
    je .handle_palat                        ; ...being edited.
    jmp .exit
    .handle_palat:
    call Handle_Pala_Click
    mov [click_in_segment],1
    jmp .exit

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; deal with clicks to the edit segment.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    .in_edit:
    cmp [click_in_segment],0
    je .handle_edit
    cmp [click_in_segment],2
    je .handle_edit
    jmp .exit
    .handle_edit:
    call Handle_Edit_Click
    mov [click_in_segment],2
    jmp .exit

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; deal with clicks to the palette segment.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    .in_palette:
    cmp [click_in_segment],0
    je .handle_palette
    cmp [click_in_segment],3
    je .handle_palette
    jmp .exit
    .handle_palette:
    call Translate_Mouse_Pos_To_Palette_Segment
    cmp [mouse_inside_palette],1
    jne .exit

    ; change the pen color.
    mov al,[mouse_pos_palette_y]
    cmp al,[pen_color]                      ; if the pen color is the same as before, no need to change anything.
    je .exit
    mov [pen_color],al
    mov [gfx_mouse_cursor+13],al            ; make the color of the mouse cursor's tip be the color we've got selected.

    call Draw_Palette_Selector

    mov [click_in_segment],3

    jmp .exit

    .exit:
    ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Handles a mouse click to the pala selector segment.
;;;
;;; EXPECTS:
;;;     (- unknown)
;;; DESTROYS:
;;;     (- unknown)
;;; RETURNS:
;;;     (- unknown)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Handle_Pala_Click:
    call Translate_Mouse_Pos_To_Palat_Segment

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; bail out if the mouse cursor isn't inside the palat segment at all.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    cmp [mouse_inside_palat],1
    jne .exit

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; calculate the index of the pala.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov al,byte[mouse_pos_palat_xy+1]       ; y.
    mov cl,NUM_PALA_THUMB_X
    mul cl                                  ; al = index horizontally.
    add al,byte[mouse_pos_palat_xy]         ; al += x.

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; we don't need to do anything if the pala we clicked on was already selected.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    cmp al,[selected_pala]
    je .exit

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; repaint the previously selected pala's thumbnail, to indicate any changes in it.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    push ax
    movzx ax,[selected_pala]
    mov di,[prev_pala_thumb_offs]
    add di,SCREEN_W
    call Draw_Pala_Thumbnail
    pop ax

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; change the pala we've got selected, and redraw the editor to update that we've selected
    ; a different pala.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov [selected_pala],al
    call Draw_Pala_Editor

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; erase the frame of the previously-selected pala's thumbnail.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov di,[prev_pala_thumb_offs]
    mov eax,0                               ; the frame's color.
    call Draw_Pala_Thumb_Halo

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; draw a frame around the pala we chose.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; first, calculate the x,y pixel offset at the top left corner of this thumbnail.
    movzx ax,byte[mouse_pos_palat_xy]       ; x.
    shl ax,3                                ; ax *= 8, the height of a pala thumbnail.
    mov di,ax
    add di,3                                ; offset horizontally to match the palat selector segment.
    movzx ax,byte[mouse_pos_palat_xy+1]     ; y.
    shl ax,3                                ; ax *= 8, the width of a pala thumbnail.
    add ax,8                                ; offset vertically to match the palat selector segment.
    mov cx,SCREEN_W
    mul cx                                  ; ax = absolute screen y coordinate.
    add di,ax
    mov [prev_pala_thumb_offs],di
    ; then draw it.
    mov eax,05050505h                       ; the frame's color.
    call Draw_Pala_Thumb_Halo

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; update the ui accordingly.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    call Draw_Current_Pala_ID               ; indicate the id of the pala we chose.

    .exit:
    ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Handles a mouse click to the edit segment.
;;;
;;; EXPECTS:
;;;     (- unknown)
;;; DESTROYS:
;;;     (- unknown)
;;; RETURNS:
;;;     (- unknown)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Handle_Edit_Click:
    call Translate_Mouse_Pos_To_Edit_Segment

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; bail out if the mouse cursor isn't inside the edit segment at all.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    cmp [mouse_inside_edit],1
    jne .exit

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; calculate the offset of the current pala pixel in the palat data. the offset will be placed in si.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    movzx si,byte[mouse_pos_edit_xy]        ; x.
    mov ax,(PALA_H - 1)
    movzx bx,byte[mouse_pos_edit_xy+1]      ; y.
    sub ax,bx                               ; flip the vertical coordinate.
    shl ax,4                                ; multiply by 16 (PALA_W).
    add si,ax                               ; si = offset in pala's data where we clicked.
    ; then get the offset of this pala's first pixel in the palat data buffer.
    mov ax,(PALA_W * PALA_H)
    movzx cx,byte[selected_pala]
    mul cx
    add si,ax                               ; si = offset of the pixel we edited in the palat data buffer.

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; if the user pressed anything but the right button, paint with the current pen color. otherwise,
    ; paint with this pala pixel's original color, thus undoing any changes.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov bl,[pen_color]                      ; prepare to paint with the current pen color.
    test [mouse_buttons],10b                ; but if the user was pressing the right mouse button, paint instead with the pala's original pixel color.
    jz .draw
    ; change the data palat data segment to the backup version.
    push gs
    mov ax,@BUFFER_3
    mov gs,ax
    mov bl,[gs:pala_data+si]                ; save the altered pixel into the data array.
    pop gs

    .draw:
    call Redraw_Current_Edit_Pixel

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; update the pala's data in the PALAT array
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov [gs:pala_data+si],bl                ; save the altered pixel into the data array.

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; update the ui accordingly.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    call Draw_Unsaved_Changes_Marker        ; indicate that there are unsaved changes in the data.

    .exit:
    ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Deals with keyboard input by responding properly to the program's keyboard shortcuts.
;;;
;;; EXPECTS:
;;;     (- unknown)
;;; DESTROYS:
;;;     (- unknown)
;;; RETURNS:
;;;     (- unknown)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Handle_Editor_Keyboard_Input:
    mov al,[key_pressed]

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; find out which key was pressed.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    .test_no_key:
    cmp al,KEY_NO_KEY
    je .exit

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; save the palat data into the project file.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    .test_s:
    cmp al,KEY_S
    jne .test_plus
    call Save_Palat_File
    cmp al,1
    jne .save_error
    ; clear the save marker.
    mov eax,65656565h                       ; 65h = black.
    mov di,(SCREEN_W * 2) + 4
    mov [es:di],eax
    mov [es:di+SCREEN_W],eax
    mov [es:di+SCREEN_W*2],eax
    mov [es:di+SCREEN_W*3],eax
    jmp .exit
    .save_error:
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ;;;; TODO: handle the save error.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    call Draw_Save_Error_Marker
    jmp .exit

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; set the pala editor's size larger.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    .test_plus:
    cmp al,KEY_PLUS
    jne .test_minus
    add [magnification],4
    cmp [magnification],12                  ; make sure we set the size no larger than 12.
    jng .repaint_plus
    mov [magnification],12
    jmp .exit                               ; we already had the maximum size, so don't need to redraw.
    .repaint_plus:
    call Redraw_All
    jmp .exit

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; set the pala editor's size smaller.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    .test_minus:
    cmp al,KEY_MINUS
    jne .exit
    sub [magnification],4
    jnz .repaint_minus
    mov [magnification],4                   ; if the size is smaller than 4, set it to 4.
    jmp .exit                               ; we already had the minimum size, so no need to redraw.
    .repaint_minus:
    call Redraw_All
    jmp .exit

    .exit:
    ret
