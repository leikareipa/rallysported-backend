;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Patches RALLYE.EXE to dump the player's ghost lap into a file on disk when the program exits
;;; (i.e. when the player exits the race).
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Patch_Rallye_Exe:
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; open ~~LLYE.EXE.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    xor cx,cx
    mov dx,fn_sb_rallye_exe                     ; file name.
    mov ah,3dh                                  ; set to open.
    mov al,0010b                                ; read/write.
    int 21h                                     ; do it.
    jc .exit_fail                               ; error-checking (the cf flag will be set by int 21h if there was an error).
    mov [fh_sb_rallye_exe],ax

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; change the player's starting position to be that of the cpu's.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    call Apply_Starting_Pos_Patch
    cmp al,1
    jne .exit_fail

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; apply the ghost lap patch.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    call Apply_Ghost_Lap_Patch
    cmp al,1
    jne .exit_fail

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; patch the file name in ~~LLYE.EXE where to dump the lap data.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov ebx,208cch                          ; the start of the name of the file to which the game will dump by default.
    mov dx,bx                               ; dx = lowest bits of the offset.
    shr ebx,16
    mov cx,bx                               ; cx = highest bits of the offset.
    mov bx,[fh_sb_rallye_exe]
    mov ax,4200h                            ; set to move file position, offset from the beginning.
    int 21h                                 ; move file position.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).
    mov dx,fn_lap_dump
    mov cx,fn_lap_dump_len                  ; set to write the entire file name, including null terminator.
    mov ah,40h
    int 21h                                 ; write.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; patch into ~~LLYE.EXE GAME.DTA's sandboxed filename.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov [file_buffer],'~'
    mov [file_buffer+1],'~'
    mov ebx,2084fh                          ; the start of the name of the file to which the game will dump by default.
    mov dx,bx                               ; dx = lowest bits of the offset.
    shr ebx,16
    mov cx,bx                               ; cx = highest bits of the offset.
    mov bx,[fh_sb_rallye_exe]
    mov ax,4200h                            ; set to move file position, offset from the beginning.
    int 21h                                 ; move file position.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).
    mov dx,file_buffer
    mov cx,2
    mov ah,40h
    int 21h                                 ; write.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; patch ~~LLYE.EXE so that it always shows a 'recording' message on screen.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    call Apply_Text_Patch
    cmp al,1
    jne .exit_fail

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; close ~~LLYE.EXE.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov ah,3eh                              ; set to close.
    mov bx,[fh_sb_rallye_exe]
    int 21h                                 ; do it.

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; open ~~ME.EXE.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    xor cx,cx
    mov dx,fn_sb_game_dta                     ; file name.
    mov ah,3dh                                  ; set to open.
    mov al,0010b                                    ; read/write.
    int 21h                                     ; do it.
    jc .exit_fail                               ; error-checking (the cf flag will be set by int 21h if there was an error).
    mov [fh_sb_game_dta],ax

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; read the contents of ~~ME.DTA.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov bx,[fh_sb_game_dta]
    mov dx,file_buffer
    mov ah,3fh
    mov cx,37                               ; game.dta should be 37 effective bytes long.
    int 21h
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; modify certain race parameters in ~~ME.DTA.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov [file_buffer+1],1                   ; number of laps.
    mov [file_buffer+30],1                  ; enable daytime race.
    mov [file_buffer+35],1                  ; race mode: practice with ghost.

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; seek back in ~~ME.DTA.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov cx,0
    mov dx,0
    mov ax,4200h                            ; set to move file position, offset from beginning.
    int 21h                                 ; move file position.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; write the modified data back into ~~ME.DTA.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov dx,file_buffer
    mov cx,37
    mov ah,40h
    int 21h                                 ; write the new maasto index.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; close ~~ME.DTA.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov ah,3eh                              ; set to close.
    mov bx,[fh_sb_rallye_exe]
    int 21h                                 ; do it.

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; create the file to dump to. this truncates the file if it already exists.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    xor cx,cx
    mov dx,fn_lap_dump                      ; file name.
    mov ah,3ch                              ; set to create/truncate.
    int 21h                                 ; do it.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).

    jmp .exit_success

    .exit_fail:
    mov al,0
    jmp .exit

    .exit_success:
    mov al,1
    jmp .exit

    .exit:
    ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Applies a patch meant for RALLYE.EXE that shows a 'Recording' message on screen in-game at all times.
;;;
;;; EXPECTS:
;;;     - fh_sb_rallye_exe to be a file handle to the RALLYE.EXE to patch.
;;;     - file_buffer to be a buffer with at least record_message_len bytes of space.
;;; DESTROYS:
;;;     (- unknown)
;;; RETURNS:
;;;     (- unknown)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Apply_Text_Patch:
    mov bx,[fh_sb_rallye_exe]

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; patch ~~LLYE.EXE to always show the message.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov byte[file_buffer],82h
    mov cx,0
    mov dx,7205
    mov ax,4200h                            ; set to move file position, offset from beginning.
    int 21h
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).
    mov dx,file_buffer
    mov cx,1
    mov ah,40h
    int 21h                                 ; write.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; patch ~~LLYE.EXE to show the message at the bottom of the screen.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov byte[file_buffer],0bah
    mov cx,0
    mov dx,137
    mov ax,4201h
    int 21h
    mov dx,file_buffer
    mov cx,1
    mov ah,40h
    int 21h
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; patch ~~LLYE.EXE to set the message color.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov byte[file_buffer],16
    mov cx,0
    mov dx,7
    mov ax,4201h
    int 21h
    mov dx,file_buffer
    mov cx,1
    mov ah,40h
    int 21h
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; patch ~~LLYE.EXE to with the message string we want to display.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov ebx,13e41h
    mov dx,bx
    shr ebx,16
    mov cx,bx
    mov bx,[fh_sb_rallye_exe]
    mov ax,4200h
    int 21h
    mov dx,record_message
    mov cx,record_message_len
    mov ah,40h
    int 21h
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).

    jmp .exit_success

    .exit_fail:
    mov al,0
    jmp .exit

    .exit_success:
    mov al,1
    jmp .exit

    .exit:
    ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Applies a patch meant for RALLYE.EXE that makes the game save the current ghost lap to disk on exit.
;;;
;;; EXPECTS:
;;;     - fh_sb_rallye_exe to be a file handle to the RALLYE.EXE to patch.
;;;     - file_buffer to be a buffer with at least 23 bytes of space.
;;; DESTROYS:
;;;     (- unknown)
;;; RETURNS:
;;;     (- unknown)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Apply_Ghost_Lap_Patch:
    mov bx,[fh_sb_rallye_exe]

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; seek to the part of code in ~~LLYE.EXE that deals with saving lap results to disk on exit.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov cx,0
    mov dx,4331
    mov ax,4200h                            ; set to move file position, offset from the beginning.
    int 21h                                 ; move file position.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; read from ~~LLYE.EXE those data.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov dx,file_buffer
    mov ah,3fh
    mov cx,23
    int 21h
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; construct a patch that makes the game save the ghost lap onto disk on exit.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov byte [file_buffer],1eh
    mov byte [file_buffer+1],08bh
    mov byte [file_buffer+2],0eh
    mov byte [file_buffer+3],0e8h
    mov byte [file_buffer+4],6bh
    mov byte [file_buffer+5],08eh
    mov byte [file_buffer+6],0d9h

    mov byte [file_buffer+9],0b9h
    mov byte [file_buffer+10],0ffh
    mov byte [file_buffer+11],0ffh
    mov byte [file_buffer+12],0bah
    mov byte [file_buffer+13],0
    mov byte [file_buffer+14],0

    mov byte [file_buffer+20],1fh
    mov byte [file_buffer+21],90h
    mov byte [file_buffer+22],90h
    mov byte [file_buffer+23],90h

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; write the ghost lap patch to ~~LLYE.EXE.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov cx,-1
    mov dx,-23
    mov ax,4201h                            ; set to move file position, offset from current position.
    int 21h                                 ; move file position.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).
    mov dx,file_buffer
    mov cx,24                               ; write 24 bytes.
    mov ah,40h
    int 21h                                 ; write the new maasto index.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).

    jmp .exit_success

    .exit_fail:
    mov al,0
    jmp .exit

    .exit_success:
    mov al,1
    jmp .exit

    .exit:
    ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Applies a patch meant for RALLYE.EXE that changes the player's starting position to better match that of the
;;; CPU opponent in an actual race.
;;;
;;; EXPECTS:
;;;     - fh_sb_rallye_exe to be a file handle to the RALLYE.EXE to patch.
;;; DESTROYS:
;;;     (- unknown)
;;; RETURNS:
;;;     (- unknown)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Apply_Starting_Pos_Patch:
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; get the track id from ~~LLYE.EXE.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov ebx,20878h                          ; the start of the file name block.
    mov dx,bx                               ; dx = lowest bits of the offset.
    shr ebx,16
    mov cx,bx                               ; cx = highest bits of the offset.
    mov bx,[fh_sb_rallye_exe]
    mov ax,4200h                            ; set to move file position, offset from the beginning.
    int 21h                                 ; move file position.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).
    mov dx,track_id
    mov cx,1                                ; read 1 byte, which is the track id in one-base ascii.
    mov ah,3fh
    int 21h                                 ; read.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).
    sub [track_id],'1'                      ; convert the track id to zero-base decimal.
    test [track_id],11111000b               ; make sure the track id is in the range 0-7.
    jnz .exit_fail

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; calculate the file offset for the car's starting position ~~LLYE.EXE.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    movzx ebx,[track_id]
    lea ebx,[starting_pos_ptr+(ebx*4)]      ; ebx = address in the block of offset addresses to the starting byte of the position's offset.
    mov ebx,dword [ebx]                     ; ebx now points to the word in the game executable where this track's starting position is defined.
    ; form cx:dx from ebx, which we can pass to int 21h as a file offset.
    mov dx,bx                               ; dx = lowest bits of the offset.
    shr ebx,16
    mov cx,bx                               ; cx = highest bits of the offset.

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; calculate the offset for the car's starting position patch in the lookup table.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov al,[track_id]
    mov bl,2
    mul bl
    mov si,ax

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; write the new starting position into ~~LLYE.EXE.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov bx,[fh_sb_rallye_exe]
    mov ax,4200h                            ; set to move file position, offset from beginning, given in cx:dx.
    int 21h                                 ; move file position.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).
    lea dx,[new_starting_pos+si]
    mov cx,2                                ; set to write 2 bytes.
    mov ah,40h
    int 21h                                 ; write the patch.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).

    jmp .exit_success

    .exit_fail:
    mov al,0
    jmp .exit

    .exit_success:
    mov al,1
    jmp .exit

    .exit:
    ret
