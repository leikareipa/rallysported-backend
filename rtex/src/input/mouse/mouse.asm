;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;
;;; Subroutines for handling mouse input.
;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Initialize the mouse driver.
;;;
;;; EXPECTS:
;;;     - the video mode should be regular text mode when calling this routine, as the routine may print an error message
;;; DESTROYS:
;;;     - ax
;;; RETURNS:
;;;     - ax = whether we successfully initialized the mouse (0 = no, FFFF = yes).
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Enable_Mouse:
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; acquire the mouse via interrupt 33.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    xor ax,ax
    int 33h                                 ; if failed to acquire, will return 0 in ax, otherwise ax will be ffff.
    ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Gets the mouse's current x,y position, and whether any mouse buttons are being held.
;;;
;;; EXPECTS:
;;;     - ds to point to the data segment containing the mouse's current position, etc.
;;; DESTROYS:
;;;     - ax, bx, ecx, dx.
;;; RETURNS:
;;;     (- unknown)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Poll_Mouse_Status:
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; save the mouse's location from last frame, so we'll know by how much it has moved since then.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov ecx,dword [mouse_pos_xy]
    mov dword [prev_mouse_pos_xy],ecx

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; poll the mouse's status from the bios. this'll put x into dx, y into into cx, and mouse status into bx.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov ax,3
    int 33h
    shr cx,1                                ; divide x coordinate by 2.

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; save the mouse status into variables in memory, for later access.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    rol ecx,16                              ; move x coordinate into high bits of ecx.
    mov cx,dx                               ; put y coordinate into low bits of ecx.
    mov dword [mouse_pos_xy],ecx            ; save the mouse position for later use.
    mov word [mouse_buttons],bx             ; save mouse buttons' status for later use.

    ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Translates the mouse's current coordinates to the palette segment.
;;;
;;; Note that this function should be called AFTER you've obtained the mouse cursor's current x,y coordinates
;;; and stored them in mouse_pos_xy.
;;;
;;; EXPECTS:
;;;     - ds to point to the general data segment.
;;; DESTROYS:
;;;     - eax, cl
;;; RETURNS:
;;;     (- nothing)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Translate_Mouse_Pos_To_Palette_Segment:
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; get the mouse cursor position. ax = mouse x, eax upper 16 bits = mouse y coordinate.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov eax,[mouse_pos_xy]

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; make the mouse position relative to the 0,0 corner of the palette segment, and make sure that
    ; position is inside the screen.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    sub ax,4                                ; y.
    js .not_in_segment                      ; jump if below 0.
    rol eax,16                              ; move x coordinate into ax.
    sub ax,295                              ; x.
    js .not_in_segment                      ; jump if below 0.

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; translate the mouse cursor to a palette index. we really only need to translate the y coordinate.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    shr eax,16                              ; move y coordinate into ax.
    mov cl,6
    div cl                                  ; mouse_y / swatch_h.

    cmp al,31                               ; there's only 32 colors, so any indices past that are outside of the palette selector.
    jg .exit

    mov [mouse_pos_palette_y],al            ; store the palette index over which the mouse hovers.
    mov [mouse_inside_palette],1

    jmp .exit

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; if the cursor isn't inside the edit segment, set the 16th bit in eax to 0 to signal this.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    .not_in_segment:
    mov [mouse_inside_palette],0

    .exit:
    ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Translates the mouse's current coordinates to the palat segment.
;;;
;;; Note that this function should be called AFTER you've obtained the mouse cursor's current x,y coordinates
;;; and stored them in mouse_pos_xy.
;;;
;;; EXPECTS:
;;;     (- unknown)
;;; DESTROYS:
;;;     (- unknown)
;;; RETURNS:
;;;     (- nothing)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Translate_Mouse_Pos_To_Palat_Segment:
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; get the mouse cursor position. ax = mouse x, eax upper 16 bits = mouse y coordinate.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov ebx,[mouse_pos_xy]

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; make the mouse position relative to the 0,0 corner of the edit segment, and make sure that
    ; position is inside the screen.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    sub bx,7                                ; y.
    js .not_in_segment                      ; jump if below 0.
    rol ebx,16                              ; move x coordinate into ax.
    sub bx,3                                ; x
    js .not_in_segment                      ; jump if below 0.

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; translate the mouse cursor to a pala thumbnail index.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    shr bx,3                                ; bx /= 8, which assumes that each pala thumbnail is 8x8.
    cmp bl,NUM_PALA_THUMB_X                 ; test x.
    jge .not_in_segment                     ; if the index is >= 16, we know we're outside the pala.
    mov dl,bl                               ; set translated mouse x.
    rol ebx,16
    shr bx,3                                ; bx /= 8, which assumes that each pala thumbnail is 8x8.
    cmp bl,NUM_PALA_THUMB_Y                 ; test y.
    jge .not_in_segment                     ; if the index is >= 16, we know we're outside the pala.
    mov dh,bl                               ; set translated mouse y.

    mov [mouse_pos_palat_xy],dx
    mov [mouse_inside_palat],1

    jmp .exit

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; if the cursor isn't inside the edit segment, set the 16th bit in eax to 0 to signal this.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    .not_in_segment:
    mov [mouse_inside_palat],0

    .exit:
    ret

    ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Translates the mouse's current coordinates to the edit segment.
;;;
;;; Note that this function should be called AFTER you've obtained the mouse cursor's current x,y coordinates
;;; and stored them in mouse_pos_xy.
;;;
;;; EXPECTS:
;;;     - ds to point to the general data segment.
;;; DESTROYS:
;;;     - ax, ebx, cx, dx
;;; RETURNS:
;;;     (- nothing)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Translate_Mouse_Pos_To_Edit_Segment:
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; get the mouse cursor position. ax = mouse x, eax upper 16 bits = mouse y coordinate.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov ebx,[mouse_pos_xy]

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; make the mouse position relative to the 0,0 corner of the edit segment, and make sure that
    ; position is inside the screen.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    sub bx,4
    js .not_in_segment                      ; jump if below 0.
    rol ebx,16                              ; move x coordinate into ax.
    sub bx,99
    js .not_in_segment                      ; jump if below 0.

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; translate the mouse cursor to a pala pixel index (i.e. to a 16x16 grid).
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov cl,[magnification]
    mov ax,bx
    div cl
    cmp al,PALA_W
    jge .not_in_segment                     ; if the index is >= 16, we know we're outside the pala.
    mov dl,al                               ; set translated mouse x.
    rol ebx,16
    mov ax,bx
    div cl
    cmp al,PALA_H
    jge .not_in_segment                     ; if the index is >= 16, we know we're outside the pala.
    mov dh,al                               ; set translated mouse y.

    mov [mouse_pos_edit_xy],dx
    mov [mouse_inside_edit],1

    jmp .exit

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; if the cursor isn't inside the edit segment, set the 16th bit in eax to 0 to signal this.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    .not_in_segment:
    mov [mouse_inside_edit],0

    .exit:
    ret
