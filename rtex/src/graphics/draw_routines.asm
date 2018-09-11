;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;
;;; Subroutines for drawing things to screen in VGA mode 13h.
;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Copies the video buffer pointed to by ds:si to video memory pointed to by es:di.
;;;
;;; EXPECTS:
;;;		- si = point to start of video memory buffer
;;; DESTROYS:
;;;		- di, ax
;;; RETURNS:
;;;		(- nothing)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Flip_Video_Buffer:
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; initialize.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    push es
    push ds

    mov ax,es                               ; set up the source (video memory buffer).
    mov ds,ax                               ;
    mov ax,VRAM_SEG                         ; set up the destination (video memory).
    mov es,ax                               ;
    xor di,di                               ; start at offset 0 in video memory.
    xor si,si

    call Wait_For_VSync

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; copy the vga buffer into video memory.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov cx,3e80h                            ; how many double words to copy (320*200 = 64000/4 = 16000).
    rep movsd

    pop ds
    pop es

    ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Draws the currently selected pala as a large, paintable version in the middle of the screen.
;;;
;;; EXPECTS:
;;;     - ds to be the general data segment.
;;; DESTROYS:
;;;     - ax, si
;;; RETURNS:
;;;     (- nothing)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Draw_Pala_Editor:
    mov di,(SCREEN_W * 4) + 99              ; positioning of the editor on screen.

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; draw the currently selected pala.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    movzx ax,[selected_pala]
    .12x:
    cmp [magnification],12
    jne .8x
    call Draw_Pala_Enlarged_12X
    jmp .exit
    .8x:
    cmp [magnification],8
    jne .4x
    call Draw_Pala_Enlarged_8X
    jmp .exit
    .4x:
    call Draw_Pala_Enlarged_4X

    .exit:
    ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Resets the background of the pala editor to black.
;;;
;;; EXPECTS:
;;;     (- unknown)
;;; DESTROYS:
;;;     (- unknown)
;;; RETURNS:
;;;     (- unknown)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Reset_Pala_Editor_Background:
    mov di,(SCREEN_W * 4) + 99              ; start from the top left corner of the pala edit field.
    mov eax,65656565h                       ; 65h = black.
    mov cx,192                              ; 192 pixel rows.
    .reset:
        mov bx,48                           ; there are a maximum of 192 pixels horizontally, and we fill in 4 at a time.
        .horizontal:
            mov [es:di],eax                 ; fill 4 pixels with black.
            add di,4
            sub bx,1
            jnz .horizontal

        add di,SCREEN_W-192                 ; move to the start of the next horizontal row.
        loop .reset

    ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Draws a single pala texture as a custom-sized thumbnail.
;;;
;;; EXPECTS:
;;;     - es:di to point to the first pixel in the screen buffer to draw to
;;;     - ax to give the id of the pala texture to be drawn.
;;; DESTROYS:
;;;     (- unknown)
;;; RETURNS:
;;;     (- unknown)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Draw_Pala_Enlarged_4X:
    .MAGNIF = 4

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; calculate the starting offset of the given pala in the pala data array.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov si,(PALA_W * PALA_H)
    mul si
    mov si,ax                               ; si stores the offset.
    add si,(PALA_W * PALA_H) - PALA_W       ; flip the pala vertically.

    mov cx,PALA_H
    .column:
        push cx
        mov cx,PALA_W
        .row:
            mov ebx,dword [gs:pala_data+si]         ; get the next pixel from the pala.
            add si,1

            ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
            ; duplicate the pixel 4x into eax.
            ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
            mov bh,bl
            mov ax,bx
            rol eax,16
            mov ax,bx

            ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
            ; draw an enlarged pixel.
            ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
            push di
            stosd
            add di,(SCREEN_W - .MAGNIF)
            stosd
            add di,(SCREEN_W - .MAGNIF)
            stosd
            add di,(SCREEN_W - .MAGNIF)
            stosd
            pop di

            add di,.MAGNIF
            loop .row
        add di,((SCREEN_W * .MAGNIF) - (PALA_W * .MAGNIF))          ; move down to the next scanline.
        sub si,(PALA_W * 2)
        pop cx
        loop .column

    ret
Draw_Pala_Enlarged_8X:
    .MAGNIF = 8

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; calculate the starting offset of the given pala in the pala data array.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov si,(PALA_W * PALA_H)
    mul si
    mov si,ax                               ; si stores the offset.
    add si,(PALA_W * PALA_H) - PALA_W       ; flip the pala vertically.

    mov cx,PALA_H
    .column:
        push cx
        mov cx,PALA_W
        .row:
            mov ebx,dword [gs:pala_data+si]         ; get the next pixel from the pala.
            add si,1

            ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
            ; duplicate the pixel 4x into eax.
            ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
            mov bh,bl
            mov ax,bx
            rol eax,16
            mov ax,bx

            ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
            ; draw an enlarged pixel.
            ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
            push di
            stosd
            stosd
            add di,(SCREEN_W - .MAGNIF)
            stosd
            stosd
            add di,(SCREEN_W - .MAGNIF)
            stosd
            stosd
            add di,(SCREEN_W - .MAGNIF)
            stosd
            stosd

            add di,(SCREEN_W - .MAGNIF)
            stosd
            stosd
            add di,(SCREEN_W - .MAGNIF)
            stosd
            stosd
            add di,(SCREEN_W - .MAGNIF)
            stosd
            stosd
            add di,(SCREEN_W - .MAGNIF)
            stosd
            stosd
            pop di

            add di,.MAGNIF
            loop .row
        add di,((SCREEN_W * .MAGNIF) - (PALA_W * .MAGNIF))          ; move down to the next scanline.
        sub si,(PALA_W * 2)
        pop cx
        loop .column

    ret
Draw_Pala_Enlarged_12X:
    .MAGNIF = 12

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; calculate the starting offset of the given pala in the pala data array.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov si,(PALA_W * PALA_H)
    mul si
    mov si,ax                               ; si stores the offset.
    add si,(PALA_W * PALA_H) - PALA_W       ; flip the pala vertically.

    mov cx,PALA_H
    .column:
        push cx
        mov cx,PALA_W
        .row:
            mov bl,[gs:pala_data+si]        ; get the next pixel from the pala.
            call Draw_Edit_Pixel_12X        ; paint the current pixel.

            add si,1
            add di,.MAGNIF

            loop .row
        add di,((SCREEN_W * .MAGNIF) - (PALA_W * .MAGNIF)) ; move down to the next scanline.
        sub si,(PALA_W * 2)
        pop cx
        loop .column

    ret
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; a helper function for Draw_Pala_Enlarged_12X.
; expects:
;   - bl to contain the palette index of the pixel's color.
;   - di to point to the top left corner in the video buffer to draw the pixel to.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Draw_Edit_Pixel_12X:
    .MAGNIF = 12

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; duplicate the pixel 4x into eax.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov bh,bl
    mov ax,bx
    rol eax,16
    mov ax,bx

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; draw the enlarged pixel.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    push di
    mov dx,3                                ; the number of times we loop.
    .do:
        stosd
        stosd
        stosd
        add di,(SCREEN_W - .MAGNIF)
        stosd
        stosd
        stosd
        add di,(SCREEN_W - .MAGNIF)
        stosd
        stosd
        stosd
        add di,(SCREEN_W - .MAGNIF)
        stosd
        stosd
        stosd
        add di,(SCREEN_W - .MAGNIF)
        sub dx,1
        cmp dx,0
        jne .do
    pop di

    ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; a helper function for Draw_Pala_Enlarged_8X.
; expects:
;   - bl to contain the palette index of the pixel's color.
;   - di to point to the top left corner in the video buffer to draw the pixel to.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Draw_Edit_Pixel_8X:
    .MAGNIF = 8

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; duplicate the pixel 4x into eax.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov bh,bl
    mov ax,bx
    rol eax,16
    mov ax,bx

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; draw the enlarged pixel.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    push di
    stosd
    stosd
    add di,(SCREEN_W - .MAGNIF)
    stosd
    stosd
    add di,(SCREEN_W - .MAGNIF)
    stosd
    stosd
    add di,(SCREEN_W - .MAGNIF)
    stosd
    stosd

    add di,(SCREEN_W - .MAGNIF)
    stosd
    stosd
    add di,(SCREEN_W - .MAGNIF)
    stosd
    stosd
    add di,(SCREEN_W - .MAGNIF)
    stosd
    stosd
    add di,(SCREEN_W - .MAGNIF)
    stosd
    stosd
    pop di

    ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; a helper function for Draw_Pala_Enlarged_4X.
; expects:
;   - bl to contain the palette index of the pixel's color.
;   - di to point to the top left corner in the video buffer to draw the pixel to.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Draw_Edit_Pixel_4X:
    .MAGNIF = 4

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; duplicate the pixel 4x into eax.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov bh,bl
    mov ax,bx
    rol eax,16
    mov ax,bx

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; draw an enlarged pixel.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    push di
    stosd
    add di,(SCREEN_W - .MAGNIF)
    stosd
    add di,(SCREEN_W - .MAGNIF)
    stosd
    add di,(SCREEN_W - .MAGNIF)
    stosd
    pop di

    ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Prints the name of the project and palat file in the top right corner of the selector.
;;;
;;; EXPECTS:
;;;     (- nothing)
;;; DESTROYS:
;;;     (- unknown)
;;; RETURNS:
;;;     (- nothing)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Draw_Project_Title:
    mov di,(SCREEN_W * 2) + 9
    mov si,project_name_str
    call Draw_String

    mov di,(SCREEN_W * 2) + 57
    mov si,pala_file_str
    call Draw_String

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; draw the message that appears on the bottom left corner of the screen.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov di,(SCREEN_W * (SCREEN_H - FONT_HEIGHT - 0))+1
    mov si,message_str
    call Draw_String

    ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Prints a red '*' in the top left corner of the screen to indicate that there are unsaved changes present.
;;;
;;; EXPECTS:
;;;     (- nothing)
;;; DESTROYS:
;;;     - di, si.
;;; RETURNS:
;;;     (- nothing)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Draw_Unsaved_Changes_Marker:
    mov di,(SCREEN_W * 2) + 4
    mov si,str_unsaved_changes
    call Draw_String
    ret
Draw_Save_Error_Marker:
    mov di,(SCREEN_W * 2) + 4
    mov si,str_unsaved_changes_err
    call Draw_String
    ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Repaints the edit pixel under the mouse cursor.
;;;
;;; EXPECTS:
;;;     - bl to give the color to paint with.
;;; DESTROYS:
;;;     - ax, bh, cx, dx, di.
;;; RETURNS:
;;;     (- nothing)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Redraw_Current_Edit_Pixel:
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; update the edit pixel under the mouse cursor.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov bh,[magnification]
    mov al,byte [mouse_pos_edit_xy]         ; x.
    mul bh                                  ; convert from relative edit segment coordinates to absolute screen coordinates.
    mov di,ax
    mov al,byte [mouse_pos_edit_xy+1]       ; y.
    mul bh                                  ; convert from relative edit segment coordinates to absolute screen coordinates.
    add ax,4                                ; y offset on screen of the edit segment.
    mov cx,SCREEN_W
    mul cx
    add di,ax
    add di,99                               ; x offset on screen of the edit segment.

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; draw the pixel with the set color at the current level of magnification.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    cmp bh,12
    jne .8x
    call Draw_Edit_Pixel_12X
    jmp .exit
    .8x:
    cmp bh,8
    jne .4x
    call Draw_Edit_Pixel_8X
    jmp .exit
    .4x:
    call Draw_Edit_Pixel_4X

    .exit:
    ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Prints the ID of the currently selected pala in the bottom left corner of the screen.
;;;
;;; EXPECTS:
;;;     (- unknown)
;;; DESTROYS:
;;;     (- unknown)
;;; RETURNS:
;;;     (- unknown)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Draw_Current_Pala_ID:
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; clear the pala id's background to black.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov eax,65656565h                   ; color to clear with. 65h = black.
    mov di,(SCREEN_W * (SCREEN_H - FONT_HEIGHT))+56
    mov cx,FONT_HEIGHT
    .clear:
        mov [es:di],eax
        mov [es:di+4],eax
        mov [es:di+8],eax
        add di,SCREEN_W
        loop .clear

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; print the id on the screen.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    movzx bx,[selected_pala]            ; pala id.
    mov cl,'b'                          ; text color.
    mov di,(SCREEN_W * (SCREEN_H - FONT_HEIGHT))+56 ; text position.
    call Draw_Unsigned_Integer_Long

    ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Draws all the editable pala textures as small thumbnails.
;;;
;;; EXPECTS:
;;;     (- nothing)
;;; DESTROYS:
;;;     - ax, bx, cx, si, di
;;; RETURNS:
;;;     (- nothing)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Draw_Palat_Selector:
    .NUM_THUMBNAIL_ROWS     = 23
    .NUM_THUMBNAIL_COLUMNS  = 11
    .THUMBNAIL_SIZE         = 8

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; draw the thumbnails.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov di,(SCREEN_W * (FONT_HEIGHT + 3)) + 3
    xor ax,ax
    mov cx,.NUM_THUMBNAIL_ROWS
    .column:
        push cx
        mov cx,.NUM_THUMBNAIL_COLUMNS
        .row:
            call Draw_Pala_Thumbnail
            add ax,1
            add di,.THUMBNAIL_SIZE
            loop .row
        pop cx
        add di,2560 - (.NUM_THUMBNAIL_COLUMNS * .THUMBNAIL_SIZE) ; move to the start of the next thumbnail row on the screen.
        loop .column

    ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Draws a single pala texture as a 8x8 thumbnail.
;;;
;;; EXPECTS:
;;;     - es:di to point to the first pixel in the screen buffer to draw to
;;;     - ax to give the id of the pala texture to be drawn.
;;; DESTROYS:
;;;     - bx
;;; RETURNS:
;;;     (- nothing)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Draw_Pala_Thumbnail:
    .THUMBNAIL_SIZE = 8

    push di
    push cx
    push ax

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; calculate the starting offset of the given pala in the pala data array.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov bx,(PALA_W * PALA_H)
    mul bx
    mov bx,ax                               ; bx stores the offset.
    add bx,(PALA_W * PALA_H) - 1            ; we trace the pala's data backwards to flip it on the screen.

    add di,.THUMBNAIL_SIZE                  ; we draw from right to left, so flip the pala horizontally.

    mov cx,(.THUMBNAIL_SIZE - 1)            ; we subtract 1 to get a horizontal outline effect.
    .row:
        push cx
        mov cx,(.THUMBNAIL_SIZE - 1)        ; we subtract 1 to get a vertical outline effect.
        sub bx,2                            ; pre-add to these to get that outline effect.
        sub di,1                            ;
        .column:
            mov al,[gs:pala_data+bx]        ; get the next pixel from the pala.
            mov byte [es:di],al             ; draw.
            sub di,1
            sub bx,2
            loop .column
        add di,(SCREEN_W + .THUMBNAIL_SIZE) ; move down to the next scanline.
        sub bx,PALA_W                       ; skip a line in the pala, since we're doing 8x8 instead of 16x16.
        pop cx
        loop .row

    pop ax
    pop cx
    pop di

    ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Draws a frame around the given pala thumbnail in the palat selector.
;;;
;;; EXPECTS:
;;;     - es:di to point to the first pixel in the screen buffer to draw to, i.e. the top left corner of the thumbnail.
;;;     - eax to be the color to fill with, where the color is 1 byte, repeated 4 times.
;;; DESTROYS:
;;;     (- unknown)
;;; RETURNS:
;;;     (- unknown)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Draw_Pala_Thumb_Halo:
    .THUMB_W    = 8                     ; the width of a thumbnail.
    .Y_SKIP     = (.THUMB_W * SCREEN_W) ; how many pixels we need to skip to get from the top row of the thumbnail to its bottom row.

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; fill horizontally.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov [es:di],eax                     ; upper bar.
    mov [es:di+4],eax
    mov [es:di+.Y_SKIP],eax             ; lower bar.
    mov [es:di+.Y_SKIP+4],eax

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; fill vertically.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov cx,9
    .left_vert:
        mov [es:di],al
        mov [es:di+.THUMB_W],al
        ror eax,8
        add di,SCREEN_W
        loop .left_vert

    ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Draws a strip of palette color swatches from which the user can select the color to draw with.
;;; Each swatch is 8 px wide and 6 px tall.
;;;
;;; EXPECTS:
;;;     (- nothing)
;;; DESTROYS:
;;;     - di, ax, bx
;;; RETURNS:
;;;     (- nothing)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Draw_Palette_Selector:
    .SIDE_OFFSET    = 13
    .SWATCH_WIDTH   = 8
    .SWATCH_HEIGHT  = 6

    sti                                     ; make sure the direction flag points forward.

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; we want to start drawing on the 5th scanline, in the right-hand corner of the screen.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov di,(SCREEN_W * 4) + (SCREEN_W - .SIDE_OFFSET) - .SWATCH_WIDTH

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; draw the swatches.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov al,0
    .draw_swatches:
        call Draw_Color_Swatch
        add al,1
        cmp al,32
        jne .draw_swatches

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; draw swatch labels, which tell you the color index of the given swatch.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov di,(SCREEN_W * 4) + (SCREEN_W - .SIDE_OFFSET) + 2
    mov bx,0                                ; index number to print to screen.
    .draw_labels:
        mov cl,'d'                          ; text color.
        call Draw_Unsigned_Integer
        add bx,1
        add di,SCREEN_W * FONT_HEIGHT
        cmp bx,32
        jne .draw_labels

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; draw the currently selected color's label with a brighter color.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov ax, (FONT_HEIGHT * SCREEN_W)
    movzx bx, [pen_color]
    mul bx                                  ; ax = y offset of the start of the swatch's label.
    mov di,(SCREEN_W * 4) + (SCREEN_W - .SIDE_OFFSET) + 2   ; x,y coordinate of the first color swatch.
    add di,ax                               ; move to the x,y of the selected color swatch.
    mov cl,'b'                              ; select a light color to print the label.
    call Draw_Unsigned_Integer

    ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Draws a single color swatch, i.e. a block of a given color.
;;;
;;; EXPECTS:
;;;     - al to hold the palette index to be drawn.
;;;     - es:di to point to the first pixel in the screen buffer to start drawing to.
;;; DESTROYS:
;;;     - high 16 bits of eax
;;; RETURNS:
;;;     (- nothing)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Draw_Color_Swatch:
    push cx

    .SWATCH_WIDTH   = 8
    .SWATCH_HEIGHT  = 6

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; duplicate the palette index into all 4 bytes of eax, so we can write the 4 bytes to screen at once.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    shl eax,8
    mov al,ah
    shl eax,8
    mov al,ah
    shl eax,8
    mov al,ah

    mov cx,.SWATCH_HEIGHT
    .draw_slice:
        stosd
        stosd
        add di,SCREEN_W - .SWATCH_WIDTH     ; move to the next scanline.
        loop .draw_slice

    pop cx

    ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Erases the mouse cursor from the screen by redrawing its background over it.
;;;
;;; EXPECTS:
;;;     - ds to point to the data segment containing the mouse's current position, the cursor graphic, etc.
;;; DESTROYS:
;;;     - ecx.
;;; RETURNS:
;;;     (- unknown)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Erase_Mouse_Cursor:
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; we only want to erase the cursor if it has moved since the last frame, or if the user had clicked on something.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    cmp [mouse_buttons],0                   ; see if a mouse button was pressed.
    jne .erase_cursor                       ; if it was, erase the cursor.
    cmp [key_pressed],KEY_NO_KEY
    jne .erase_cursor
    mov ecx,[mouse_pos_xy]
    cmp ecx,[prev_mouse_pos_xy]             ; see whether the mouse has moved.
    je .dont_erase_cursor                   ; if it hasn't, don't erase the cursor.

    .erase_cursor:
    call Redraw_Mouse_Cursor_Background     ; repaint the cursor's background at its position last frame.

    .dont_erase_cursor:
    ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Draws the mouse cursor on screen.
;;;
;;; EXPECTS:
;;;     - ds to point to the data segment containing the mouse's current position, the cursor graphic, etc.
;;; DESTROYS:
;;;     - ecx.
;;; RETURNS:
;;;     (- unknown)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Draw_Mouse_Cursor:
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; earlier, we only redrew the cursor's background, thus erasing the cursor, if the cursor had moved since
    ; the last frame, or if the user had clicked on something. if we didn't erase the cursor, we don't want to
    ; redraw it again now, so skip that if the cursor hasn't moved and the user hasn't pressed any mouse buttons.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    cmp [mouse_buttons],0                   ; see if a mouse button was pressed.
    jne .redraw_cursor                      ; if it was, redraw the cursor.
    cmp [key_pressed],KEY_NO_KEY
    jne .redraw_cursor
    mov ecx,[mouse_pos_xy]
    cmp ecx,[prev_mouse_pos_xy]             ; see whether the mouse has moved.
    je .skip_redraw                         ; if it hasn't, don't redraw the cursor.

    .redraw_cursor:
    call Save_Mouse_Cursor_Background       ; save the cursor's background this frame, so we can use to it erase the cursor next frame.
    call Draw_Mouse_Cursor_Graphic

    .skip_redraw:
    ret

ECX_To_VGA_Mem_Offset:
    ; convert the y coordinate into a row index on the screen.
    mov ax,cx										; y coordinate.
    mov bx,140h										; width of the screen in vga mode 13h.
    mul bx

    ; add the x coordinate to the row index.
    ror ecx,16										; move the high bits (x coordinate) in ecx to cx.
    mov bx,cx
    add ax,bx										; ax now contains a byte offset to the location on screen (video memory) where the mouse cursor should be drawn.
    mov di,ax

    ror ecx,10h										; restore the y coordinate to cx.

    ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Draws the mouse cursor's graphic on screen, with alpha. Checks to make sure no out-of-screen drawing is done.
;;;
;;; EXPECTS:
;;;     - ds to point to the data segment containing the mouse's current position, the cursor graphic, etc.
;;; DESTROYS:
;;;     - ax, bx, cx, dx, di, si.
;;; RETURNS:
;;;     (- nothing)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Draw_Mouse_Cursor_Graphic:
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; map the mouse's screen x,y coordinates into an offset in the video memory buffer.
    ; the offset will be stored in di.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov ecx,dword [mouse_pos_xy]
    call ECX_To_VGA_Mem_Offset
    add di,vga_buffer                       ; offset the video memory buffer index to start where the buffer starts in its segment.

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; load si with the pointer to the mouse cursor graphic.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov si,gfx_mouse_cursor
    add si,2                                ; skip the cursor image's header.

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; get the mouse cursor's x,y coordinates. x will be placed in edx, y in ecx.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov ecx,dword [mouse_pos_xy]
    mov edx,ecx
    shr edx,16

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; see whether the cursor is fully inside the screen, and if not, alter the size of its rectangle so we don't
    ; draw outside of the screen borders.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    .check_x:
    cmp dx,(SCREEN_W - CURSOR_W)
    jl .keep_x                              ; if mouse_x < (screen_w - cursor_w).
    mov ax,SCREEN_W                         ; otherwise, set the cursor image's width to be the remainder.
    sub ax,dx
    mov dl,al
    jmp .check_y
    .keep_x:
    mov dl,CURSOR_W

    .check_y:
    cmp cx,(SCREEN_H - CURSOR_H)
    jl .keep_y                              ; if mouse_y < (screen_h - cursor_h).
    mov ax,SCREEN_H                         ; otherwise, set the cursor image's height to be the remainder.
    sub ax,cx
    jmp .assign_adjusted_size
    .keep_y:
    mov al,CURSOR_H

    .assign_adjusted_size:                  ; set the background's new size.
    mov bl,dl
    mov bh,al

    mov dx,(CURSOR_W + 1)
    movzx ax,bl
    sub dx,ax                               ; dx == the number of pixels we skip on each horizontal row.

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; draw the image.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    .draw_image:
    mov ch,bh                               ;  loop over each row in the image.
    .draw_img_y:
        mov cl,bl                           ; loop over each column on this row.
        ;mov dh,CURSOR_W                     ; we keep track of
        .draw_img_x:
            mov al,[ds:si]                  ; load the next pixel from the cursor image buffer (ds:si).
            cmp al,TRANSPARENT_COLOR
            je .skip_pixel                  ; don't write transparent pixels.
            mov [es:di],al                  ; write the pixel into the video buffer (es:di).
            .skip_pixel:
            sub cl,1                        ; keep track of how far into the adjusted image width we've drawn,
            jz .next_row                    ; and if we've fully drawn up to the adjusted width, stop and move to the next row.
            add di,1
            add si,1
            ;sub dh,1
            jmp .draw_img_x
        .next_row:
        ;movzx ax,dh
        add di,dx                           ; in case we skipped pixels due to the image being outside the screen borders,
        add si,dx                           ; adjust the indices to account for that skipping.
        add di,SCREEN_W                     ; move to the next row on the screen.
        sub di,CURSOR_W                     ; move back to the start of the image's next row on the screen.
        sub ch,1
        jnz .draw_img_y

    .done:
    ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Stores the mouse's current background into a buffer.
;;;
;;; EXPECTS:
;;;     - gs to point to the pixel buffer's segment.
;;;	- es to point to the video memory buffer's segment.
;;; DESTROYS:
;;;     - al, ecx, di, si
;;; RETURNS:
;;;     (- nothing)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Save_Mouse_Cursor_Background:
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; map the mouse's x,y position into an offset in the video memory buffer.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov ecx,dword [mouse_pos_xy]
    call ECX_To_VGA_Mem_Offset
    add di,vga_buffer                       ; offset the video memory buffer index to start where the buffer starts in its segment.
    mov si,cursor_background                ; we'll save the pixels into gs:si.

    mov edx,ecx
    shr edx,16                              ; edx is now the mouse's x coordinate, and ecx its y coordinate.

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; see whether the cursor is fully inside the screen, and if not, alter the size of its rectangle so we don't
    ; draw outside of the screen borders.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    .check_x:
    cmp dx,(SCREEN_W - CURSOR_W)
    jl .keep_x                              ; if mouse_x < (screen_w - cursor_w).
    mov ax,SCREEN_W                         ; otherwise, set the cursor image's width to be the remainder.
    sub ax,dx
    mov dl,al
    jmp .check_y
    .keep_x:
    mov dl,CURSOR_W

    .check_y:
    cmp cx,(SCREEN_H - CURSOR_H)
    jl .keep_y                              ; if mouse_y < (screen_h - cursor_h).
    mov ax,SCREEN_H                         ; otherwise, set the cursor image's height to be the remainder.
    sub ax,cx
    jmp .assign_adjusted_size
    .keep_y:
    mov al,CURSOR_H

    .assign_adjusted_size:                  ; set the background's new size.
    mov bl,dl
    mov bh,al
    movzx dx,dl

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; if the cursor was fully inside the screen, we can use reckless filling methods. otherwise, we fill pixel by pixel.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    cmp bl,CURSOR_W
    jl .careful
    cmp bh,CURSOR_H
    jl .careful
    jmp .reckless

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; save the background, either carefully or recklessly.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    .careful:
    mov cl,bh
    .column:
        mov ch,bl
        .row:
            mov al,[es:di]                  ; load a pixel from the screen.
            mov [gs:si],al                  ; save the pixel into the pixel buffer.
            add di,1
            add si,1
            sub ch,1
            jnz .row
        add di,(SCREEN_W)                   ; move to the next scanline.
        sub di,dx                           ; move to the start of the cursor's image position on that scanline.
        sub cl,1
        jnz .column
    jmp .exit

    .reckless:
    mov cl,CURSOR_H
    .row2:
        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        ; move 10 bytes per row - assumes the cursor is 10 px wide.
        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        mov eax,[es:di]
        mov [gs:si],eax
        add di,4
        add si,4

        mov eax,[es:di]
        mov [gs:si],eax
        add di,4
        add si,4

        mov ax,[es:di]
        mov [gs:si],ax
        add di,2
        add si,2

        add di,(SCREEN_W - CURSOR_W)        ; move to the next scanline.
        sub cl,1

        jnz .row2

    .exit:
    ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Draws an image that represents the mouse cursor's background over the mouse cursor, thereby erasing the
;;; cursor from the screen while minimizing the screen area that is redrawn.
;;;
;;; EXPECTS:
;;;     - gs to point to the pixel buffer's segment.
;;;	- es to point to the video buffer's segment.
;;; DESTROYS:
;;;     - al, ecx, di, si
;;; RETURNS:
;;;     (- nothing)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Redraw_Mouse_Cursor_Background:
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; map the mouse's x,y position into an offset in the video memory buffer.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov ecx,dword [prev_mouse_pos_xy]
    call ECX_To_VGA_Mem_Offset
    add di,vga_buffer                       ; offset the video memory buffer index to start where the buffer starts in its segment.
    mov si,cursor_background                ; the background pixel buffer.

    mov edx,ecx
    shr edx,16                              ; edx is now the mouse's x coordinate, and ecx its y coordinate.

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; see whether the cursor is fully inside the screen, and if not, alter the size of its rectangle so we don't
    ; draw outside of the screen borders.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    .check_x:
    cmp dx,(SCREEN_W - CURSOR_W)
    jl .keep_x                              ; if mouse_x < (screen_w - cursor_w).
    mov ax,SCREEN_W                         ; otherwise, set the cursor image's width to be the remainder.
    sub ax,dx
    mov dl,al
    jmp .check_y
    .keep_x:
    mov dl,CURSOR_W

    .check_y:
    cmp cx,(SCREEN_H - CURSOR_H)
    jl .keep_y                              ; if mouse_y < (screen_h - cursor_h).
    mov ax,SCREEN_H                         ; otherwise, set the cursor image's height to be the remainder.
    sub ax,cx
    jmp .assign_adjusted_size
    .keep_y:
    mov al,CURSOR_H

    .assign_adjusted_size:                  ; set the background's new size.
    mov bl,dl
    mov bh,al
    movzx dx,dl

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; if the cursor was fully inside the screen, we can use reckless filling methods. otherwise, we fill pixel by pixel.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    cmp bl,CURSOR_W
    jl .careful
    cmp bh,CURSOR_H
    jl .careful
    jmp .reckless

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; redraw the cursor. this version fills in pixel by pixel, and only to the adjusted size
    ; of the cursor's rectangle (see above).
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    .careful:
    mov cl,bh
    .column:
        mov ch,bl
        .row:
            mov al,[gs:si]                  ; load a pixel from the buffer.
            mov [es:di],al                  ; save the pixel into the video buffer.
            add di,1
            add si,1
            sub ch,1
            jnz .row
        add di,(SCREEN_W)                   ; move to the next scanline.
        sub di,dx
        sub cl,1
        jnz .column
    jmp .exit

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; redraw the cursor. this version fills in 10 pixels per row. note that it assumes the cursor to
    ; be 10 px wide.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    .reckless:
    mov cl,CURSOR_H
    .row2:
        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        ; move 10 bytes per row - assumes the cursor is 10 px wide.
        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        mov eax,[gs:si]
        mov [es:di],eax
        add di,4
        add si,4

        mov eax,[gs:si]
        mov [es:di],eax
        add di,4
        add si,4

        mov ax,[gs:si]
        mov [es:di],ax
        add di,2
        add si,2

        add di,(SCREEN_W - CURSOR_W)        ; move to the next scanline.
        sub cl,1
        jnz .row2

    .exit:
    ret
