;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;
;;; Subroutines that have to do with rendering or handling text (strings).
;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Draws a null-terminated string (must have at least 1 character + null) onto the screen in VGA mode 13h
;;; using the custom character set in the 'font' array. Note that the string must not contain characters that
;;; are not represented in the character set, with the exception of lowercase color control characters.
;;;
;;; EXPECTS:
;;;     - es:di = location in the video memory buffer to draw to
;;;     - ds:si = location of the string's first character
;;; DESTROYS:
;;;     - ax, bx, cx, si
;;; RETURNS:
;;;     (- nothing)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Draw_String:
    push di                                 ; preserve the location on screen where the text is to be drawn.
    mov al,1dh                              ; set default text color.

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; find the string's length.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    xor bx,bx
    .get_str_len:
        inc bx                              ; assumes the string is at least 1 character long, i.e. that the first character isn't a null.
        cmp [ds:si+bx],byte 0
        jne .get_str_len                    ; loop until we encounter a null character.

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; draw the string, character by character.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov cx,bx                               ; loop over each character, except the null terminator.
    .draw_str:
        movzx bx,byte [ds:si]               ; fetch the next character into bx.

        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        ; see whether the current character is a control character for color.
        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        cmp bl,'a'                          ; for lowercase ascii, we take the value to refer to a palette entry, i.e. it's a color code.
        jb .l1                              ; jump if not lowercase.
        mov al,bl                           ; if the character is a color code, set the text color accordingly and move on.
        jmp .l2                             ; jump to the end of the loop. don't draw this character.

        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        ; locate the character's bitmap in the character set, and draw it to the screen buffer.
        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        .l1:
        sub bx,' '                          ; the font's character set begins at space, so make bx point to a relative index within the character set, where space = 0.
        shl bx,5                            ; multiply by 32, as each character block is 16 words. bx now points to this character's bitmap.
        push di
        .draw_char:
            add di,[font+bx]                ; get the next pixel offset. if there are no more pixels, the offset is ffffh,
            jc .next_char                   ; which will set the carry flag. if that happened, we can jump to the next character.
            add bx,2
            mov [es:di],byte al             ; draw a pixel into the video memory buffer.
            jmp .draw_char

        .next_char:
        pop di                              ; restore the screen pointer to the top left corner of the character
        add di,4                            ; move the screen pointer forward by one character's width.
        .l2:
        inc si                              ; move to the next character in the string.
        loop .draw_str

    pop di
    ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Draws an unsigned integer (valid range: 0-99) into a video memory buffer in VGA mode 13h.
;;; The routine first converts the integer into an ASCII string, then calls the string drawer
;;; routine to draw it.
;;;
;;; EXPECTS:
;;;     - es:di = location in the video memory buffer to draw to
;;;     - cl = the color to draw with
;;;     - bx = the value to draw
;;; DESTROYS:
;;;     - eax, cl
;;; RETURNS:
;;;     (- nothing)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Draw_Unsigned_Integer:
    push bx

    mov byte [tmp_int_str],cl               ; save the color code at the beginning of the string.
    mov eax,0a0064h                         ; low 16 bits = 100, high 16 bits = 10.

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; extract the most significant digit from the value. that is, keep subtracting 100 from the value
    ; until the value is below 100, and the number of times we needed to subtract is the digit.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ;mov cl,'0'
    ;.100:
    ;    cmp bx,ax                           ; ax < 100?
    ;    jb .100_done
    ;    sub bx,ax
    ;    inc cl
    ;    jmp .100
    ;.100_done:
    ;mov byte [tmp_int_str+1],cl

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; extract the second-most significant digit from the value.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    shr eax,16                              ; ax = 10.
    mov ch,'0'
    .10:
        cmp bx,ax                           ; ax < 10?
        jb .10_done
        sub bx,ax
        inc ch
        jmp .10
    .10_done:
    mov word [tmp_int_str+1],cx             ; cl == most significant digit, ch == second-most significant digit.

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; extract the third-most significant digit from the value.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    .1:
    add bl,'0'
    mov byte [tmp_int_str+3],bl             ; bl == third-most significant digit.

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; see whether the value is below 10, and if it is, remove the leading zero character.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    cmp ch,'0'                              ; is the value < 10?
    jne .no_leading_adjust
    mov bh,0
    mov word [tmp_int_str+1],bx             ; store the first digit at the start of the string, followed by a null-terminating 0.

    .no_leading_adjust:

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; print the finished string to the screen and return.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov si,tmp_int_str
    call Draw_String

    pop bx

    ret

Draw_Unsigned_Integer_Long:
    mov byte [tmp_int_str],cl               ; save the color code at the beginning of the string.
    mov eax,0a0064h                         ; low 16 bits = 100d, high 16 bits = 10d.

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; extract the most significant digit from the value. that is, keep subtracting 100 from the value
    ; until the value is below 100, and the number of times we needed to subtract is the digit.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov cl,'0'
    .100:
        cmp bx,ax                           ; ax < 100?
        jb .100_done
        sub bx,ax
        inc cl
        jmp .100
    .100_done:
    ;mov byte [tmp_int_str+1],cl

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; extract the second-most significant digit from the value.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    shr eax,16                              ; ax = 10.
    mov ch,'0'
    .10:
        cmp bx,ax                           ; ax < 10?
        jb .10_done
        sub bx,ax
        inc ch
        jmp .10
    .10_done:
    mov word [tmp_int_str+1],cx             ; cl == most significant digit, ch == second-most significant digit.

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; extract the third-most significant digit from the value.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    .1:
    add bl,'0'
    mov byte [tmp_int_str+3],bl             ; bl == third-most significant digit.

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; print the finished string to the screen and return.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov si,tmp_int_str
    call Draw_String

    ret
