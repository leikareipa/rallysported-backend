;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Applies to the game's executables the common patches required by RallySportED.
;;;
;;; EXPECTS:
;;;     (- unknown)
;;; DESTROYS:
;;;     (- unknown)
;;; RETURNS:
;;;     (- unknown)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Apply_Base_Patches:
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; mark game.dta with the id of the track we want to load.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov al,[track_id]
    mov byte [file_buffer],al
    xor cx,cx
    mov dx,fn_game_dta                      ; file name.
    mov ax,3d01h                            ; set to open, write only.
    int 21h                                 ; do it.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).
    mov bx,ax
    mov dx,file_buffer                      ; write from disk to the file buffer.
    mov cx,1                                ; the file name block is 145 bytes.
    mov ah,40h
    int 21h                                 ; write.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).
    mov ah,3eh                              ; set to close.
    int 21h                                 ; do it.

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; RALLYE
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; patch ~~LLYE.EXE to tell it to look for track data in sandboxed ~~ files.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ;
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; read the exe's entire file name block into memory, for quicker editing.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov ebx,20845h                          ; the start of the file name block.
    mov dx,bx                               ; dx = lowest bits of the offset.
    shr ebx,16
    mov cx,bx                               ; cx = highest bits of the offset.
    mov bx,[fh_sb_rallye_exe]
    mov ax,4200h                            ; set to move file position, offset from the beginning.
    int 21h                                 ; move file position.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).
    mov dx,file_buffer                      ; write from disk to the file buffer.
    mov cx,145                              ; the file name block is 145 bytes.
    mov ah,3fh
    int 21h                                 ; read.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; edit the file name block.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov bx,'~~'
    mov word [file_buffer],bx
    mov word [file_buffer+19],bx
    mov word [file_buffer+29],bx
    mov word [file_buffer+42],bx
    mov word [file_buffer+53],bx
    mov word [file_buffer+65],bx
    mov word [file_buffer+135],bx

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; write the file name block back into ~~LLYE.EXE.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov bx,[fh_sb_rallye_exe]
    mov cx,-1                               ; cx = highest bits of the offset.
    mov dx,-145                             ; dx = lowest bits of the offset.
    mov ax,4201h                            ; set to move file position, offset from current position.
    int 21h                                 ; move file position.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).
    mov dx,file_buffer                      ; write from disk to the file buffer.
    mov cx,145                              ; the file name block is 145 bytes.
    mov ah,40h
    int 21h                                 ; write.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; make it so best lap times are saved for practice runs as well.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov byte [file_buffer],0ebh             ; set to enable the feature.
    mov cx,0                                ; cx = highest bits of the offset.
    mov dx,1959                             ; dx = lowest bits of the offset.
    mov ax,4200h                            ; seek from beginning.
    int 21h                                 ; move file position.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).
    mov dx,file_buffer                      ; write from disk to the file buffer.
    mov cx,1                                ; the file name block is 145 bytes.
    mov ah,40h
    int 21h                                 ; write.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; VALIKKO
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; patch ~~LIKKO.EXE to tell it to look for track data in sandboxed ~~ files.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ;
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; read the exe's entire file name block into memory, for quicker editing.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov ebx,25885h                          ; the start of the file name block.
    mov dx,bx                               ; dx = lowest bits of the offset.
    shr ebx,16
    mov cx,bx                               ; cx = highest bits of the offset.
    mov bx,[fh_sb_valikko_exe]
    mov ax,4200h                            ; set to move file position, offset from the beginning.
    int 21h                                 ; move file position.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).
    mov dx,file_buffer                      ; write from disk to the file buffer.
    mov cx,135                              ; the file name block is 135 bytes.
    mov ah,3fh
    int 21h                                 ; read.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; edit the file name block.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov bx,'~~'
    mov word [file_buffer],bx               ; text1.dta.
    mov word [file_buffer+10],bx            ; hitable.dta.
    mov word [file_buffer+22],bx            ; results.dta.
    mov word [file_buffer+43],bx            ; kierrosx.dta.
    mov word [file_buffer+56],bx            ; maasto.dta.
    mov word [file_buffer+67],bx            ; varimaa.dta.
    mov word [file_buffer+79],bx            ; palat.dta

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; write the file name block back into ~~LLYE.EXE.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov bx,[fh_sb_valikko_exe]
    mov cx,-1                               ; cx = highest bits of the offset.
    mov dx,-135                             ; dx = lowest bits of the offset.
    mov ax,4201h                            ; set to move file position, offset from current position.
    int 21h                                 ; move file position.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).
    mov dx,file_buffer                      ; write from disk to the file buffer.
    mov cx,135                              ; the file name block is 145 bytes.
    mov ah,40h
    int 21h                                 ; write.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; change the menu title of the track select option.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov ebx,18b69h                          ; the start of the file name block.
    mov dx,bx                               ; dx = lowest bits of the offset.
    shr ebx,16
    mov cx,bx                               ; cx = highest bits of the offset.
    mov bx,[fh_sb_valikko_exe]
    mov ax,4200h                            ; seek from beginning.
    int 21h                                 ; move file position.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).
    mov dx,menu_track_select
    mov cx,13
    mov ah,40h
    int 21h                                 ; write.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; patch the track's name into the menu by changing the menu title of the selected track.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov ebx,18cb2h                          ; the start of the file name block.
    mov dx,bx                               ; dx = lowest bits of the offset.
    shr ebx,16
    mov cx,bx                               ; cx = highest bits of the offset.
    mov bx,[fh_sb_valikko_exe]
    mov ax,4200h                            ; seek from beginning.
    int 21h                                 ; move file position.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).
    mov dx,menu_track_info
    mov cx,18
    mov ah,40h
    int 21h                                 ; write.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; remove track names from the track selection screen.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov ebx,18bach                          ; the start of the file name block.
    mov dx,bx                               ; dx = lowest bits of the offset.
    shr ebx,16
    mov cx,bx                               ; cx = highest bits of the offset.
    mov bx,[fh_sb_valikko_exe]
    mov ax,4200h                            ; seek from beginning.
    int 21h                                 ; move file position.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).
    mov dx,track_titles
    mov cx,127
    mov ah,40h
    int 21h                                 ; write.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; tell to look for the hitable in ~~TABLE.TXT.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov bl,'~'
    mov bh,'~'
    mov word [file_buffer],bx
    mov ebx,2588fh                          ; the start of the file name block.
    mov dx,bx                               ; dx = lowest bits of the offset.
    shr ebx,16
    mov cx,bx                               ; cx = highest bits of the offset.
    mov bx,[fh_sb_valikko_exe]
    mov ax,4200h                            ; seek from beginning.
    int 21h                                 ; move file position.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).
    mov dx,file_buffer
    mov cx,2
    mov ah,40h
    int 21h                                 ; write.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; disable scrolling through tracks (left/right arrow keys) in the track selection menu.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov bx,[fh_sb_valikko_exe]
    mov [file_buffer],0c3h
    mov [file_buffer+1],90h
    mov [file_buffer+2],90h
    mov [file_buffer+3],90h
    mov cx,0                                ; cx = highest bits of the offset.
    mov dx,3472                             ; dx = lowest bits of the offset.
    mov ax,4200h                            ; seek from beginning.
    int 21h                                 ; move file position.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).
    mov dx,file_buffer                      ; write from disk to the file buffer.
    mov cx,4                                ; the file name block is 145 bytes.
    mov ah,40h
    int 21h                                 ; write.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).
    mov cx,0                                ; cx = highest bits of the offset.
    mov dx,12                               ; dx = lowest bits of the offset.
    mov ax,4201h                            ; seek from current position.
    int 21h                                 ; move file position.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).
    mov dx,file_buffer                      ; write from disk to the file buffer.
    mov cx,4                                ; the file name block is 145 bytes.
    mov ah,40h
    int 21h                                 ; write.
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
