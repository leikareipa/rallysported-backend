Parse_Lap_Dump:
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; open the lap dump.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    xor cx,cx
    mov dx,fn_lap_dump                      ; file name.
    mov ah,3dh                              ; set to open.
    mov al,0010b                            ; read/write.
    int 21h                                 ; do it.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).
    mov [fh_lap_dump],ax

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; read the lap dump.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov bx,[fh_lap_dump]
    mov cx,FILE_BUFFER_SIZE
    mov dx,file_buffer
    mov ah,3fh
    int 21h
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; close the lap dump file.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov ah,3eh
    int 21h
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; parse the lap dump.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    xor dx,dx                               ; the car's current speed.
    ;mov dx,60
    xor si,si                               ; si is our index into the lap dump.
    xor di,di                               ; di is the index into the lap dump where we re-write the parsed data.
    xor cx,cx                               ; used to keep track of when the checkpoints move over to a new tile.
    mov [tmp],1
    mov word [tmp+2],0
    mov word [tmp+4],0
    .parse:
        cmp si,(FILE_BUFFER_SIZE+9)         ; make sure we don't access the file buffer out of bounds.
        jae .exit_fail

        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        ; test for the end code, and if we found it, we know we've finished (presumably) successfully.
        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        cmp word [file_buffer+si],0fefeh
        je .done

        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        ; determine the car's speed based on the distance it moved since the previous checkpoint.
        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        push cx
        mov ax,word[file_buffer+si]             ; current x position.
        mov bx,word[file_buffer+si+2]           ; current y position.
        mov cx,word[tmp+2]                      ; previous x position.
        mov dx,word[tmp+4]                      ; previous y position.
        mov word[tmp+2],ax                      ; update current position for next frame.
        mov word[tmp+4],bx

        cmp cx,ax                               ; get the relative change in the car's horizontal speed.
        jae .sub_x
        xchg cx,ax
        .sub_x:
        sub cx,ax

        cmp dx,bx                               ; get the relative change in the car's vertical speed.
        jae .sub_y
        xchg dx,bx
        .sub_y:
        sub dx,bx

        add dx,cx                               ; the car's speed is the combined speed vertically and horizontally.
        pop cx

        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        ; see whether the checkpoints have progressed onto a new tile since the checkpoint
        ; we last wrote to disk. if they haven't keep looping until we find a checkpoint that's
        ; on a new tile.
        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        cmp cl,[file_buffer+si+1]
        jne .go_on
        cmp ch,[file_buffer+si+3]
        jne .go_on
        jmp .next_block

        .go_on:
        mov al,[file_buffer+si]                 ; x least significant.
        mov [file_buffer+di],al

        mov al,[file_buffer+si+1]               ; x most significant.
        mov cl,al
        mov [file_buffer+di+1],al

        mov al,[file_buffer+si+2]               ; y least significant.
        mov [file_buffer+di+2],al

        mov al,[file_buffer+si+3]               ; y most significant.
        mov ch,al
        mov [file_buffer+di+3],al

        mov ax,word [file_buffer+si+4]          ; direction
        shr ax,7
        mov word [file_buffer+di+4],ax

        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        ; speed.
        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        cmp dl,63                               ; don't let speed go over 63.
        jbe .no_overspeed
        mov dl,63
        .no_overspeed:
        mov [file_buffer+di+6],dl

        mov al,0                                ; just 0, no idea what it's used for.
        mov [file_buffer+di+7],al

        add di,8
        .next_block:
        add si,9

        jmp .parse

    .done:
    mov dword [file_buffer+di-8],0ffffffffh   ; mark the end of the lap.
    mov dword [file_buffer+di-4],0ffffffffh

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; set to skip the first few checkpoints, since these would have 0 speed.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov al,8                                ; 8 bytes in each checkpoint block.
    mov cl,3                                ; how many checkpoints to skip.
    mul cl
    sub di,ax                               ; di = number of bytes of converted data to write.
    mov si,ax                               ; si = offset in the converted data to start writing from.

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; write the lap dump into the project's .dta file.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; open the project's .dta file.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    xor cx,cx
    mov dx,fn_project_file                  ; file name.
    mov ah,3dh                              ; set to open.
    mov al,0010b                            ; read/write.
    int 21h                                 ; do it.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).
    mov [fh_project_file],ax

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; seek to where the kierros file starts.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    push si
    mov bx,[fh_project_file]
    mov si,5                                ; the kierros file is the 6th file in the project's .dta file, so skip the first 5.
    .seek_to_kierros:
        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        ; get the length of the next file.
        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        mov dx,file_buffer
        mov ah,3fh
        mov cx,4                                ; 4 bytes == long int.
        int 21h
        jc .seek_failed                         ; error-checking (the cf flag will be set by int 21h if there was an error).

        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        ; seek to the end of that file.
        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        mov cx,word [file_buffer+2]
        mov dx,word [file_buffer]
        mov ax,4201h                            ; set to move file position, offset from current position.
        int 21h                                 ; move file position.
        jc .seek_failed                         ; error-checking (the cf flag will be set by int 21h if there was an error).

        sub si,1
        jnz .seek_to_kierros
    pop si
    jmp .write
    .seek_failed:
    pop si
    jmp .exit_fail

    .write:
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; write the length of the new kierros data into the project's .dta file.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    movzx eax,di
    mov dword [tmp],eax
    mov cx,4
    mov dx,tmp
    mov ah,40h
    int 21h
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; write the lap dump into the project's .dta file.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov cx,di
    lea dx,[file_buffer+si]                 ; while writing, we take care to skip the first few checkpoints.
    mov ah,40h
    int 21h
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; truncate the project's .dta file to the end of the kierros data.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    xor cx,cx
    mov ah,40h
    int 21h

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; close the project's .dta file.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov bx,[fh_project_file]
    mov ah,3eh                              ; set to close.
    int 21h                                 ; do it.

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; delete the lap dump file.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ;mov ah,41h
    ;mov dx,fn_lap_dump
    ;int 21h

    jmp .exit_success

    .exit_fail:
    mov al,0
    jmp .exit

    .exit_success:
    mov al,1
    jmp .exit

    .exit:
    ret
