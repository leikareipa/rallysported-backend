;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;
;;; Handles loading from and saving to files on disk.
;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Loads 65024 bytes from the project's .dta file into a buffer in memory.
;;;
;;; EXPECTS:
;;;     - fn_project_file to give a zero-terminated file name to load from.
;;;     - pala_data to be a buffer we can read PALA_BUFFER_SIZE bytes into.
;;;     - gs to point to the segment holding the pala_data buffer.
;;;     - fs to be a copy of ds, the general data segment.
;;; DESTROYS:
;;;     (- unknown)
;;; RETURNS:
;;;     (- unknown)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Load_Palat_File:
    push ds                                 ; prepare to temporarily switch segments.
    push es

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; open the project's .dta file.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    xor cx,cx
    mov dx,fn_project_file                  ; file name.
    mov ah,3dh                              ; set to open.
    mov al,0010b                            ; read/write.
    int 21h                                 ; do it.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).
    mov [fh_project_file],ax                ; save the file handle.

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; switch to the segment that holds the palat data buffer we want to read into.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov ax,gs
    mov ds,ax

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; seek to where the palat file starts.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov bx,[fs:fh_project_file]
    mov si,2                                ; the palat file is the 3rd file in the project's .dta file, so skip the first 2.
    .seek_to_kierros:
        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        ; get the length of the next file.
        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        mov dx,pala_data                        ; we use the palat data buffer as temporary storage here.
        mov ah,3fh
        mov cx,4                                ; 4 bytes == long int.
        int 21h
        jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).
        test dword [pala_data],0ffff0000h       ; make sure the data length isn't >ffffh.
        jnz .exit_fail

        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        ; seek to the end of that file.
        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        mov cx,0
        mov dx,word [pala_data]
        mov ax,4201h                            ; set to move file position, offset from current position.
        int 21h                                 ; move file position.
        jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).

        sub si,1
        jnz .seek_to_kierros

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; save the file position where the palat data starts.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov word[fs:file_pala_data_start],ax
    mov word[fs:file_pala_data_start+2],dx

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; read in the palat file's data.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    .read:
    ; get the length of the palat file.
    mov dx,pala_data                        ; we use the palat data buffer as temporary storage here.
    mov ah,3fh
    mov cx,4                                ; 4 bytes == long int.
    int 21h                                 ; read in the length of the palat data.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).
    test dword [pala_data],0ffff0000h       ; make sure the data length isn't >ffffh.
    jnz .exit_fail
    ; read in the palat data.
    mov dx,pala_data
    mov ah,3fh
    mov cx,PALA_BUFFER_SIZE                 ; how many bytes to read.
    int 21h
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).
    cmp ax,PALA_BUFFER_SIZE                 ; make sure we read just the right number of bytes.
    jne .exit_fail

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; copy the palat data to a backup buffer, for undo.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov bx,@BUFFER_3
    mov es,bx
    mov si,pala_data
    mov di,pala_data_backup
    mov cx,ax
    shr cx,2                                ; div by 4, as we'll copy 4 bytes at a time.
    rep movsd

    jmp .exit_success

    .exit_fail:
    mov al,0
    jmp .exit

    .exit_success:
    mov al,1
    jmp .exit

    .exit:
    pop es
    pop ds
    ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Saves 65024 bytes of palat data to the project's .dta file.
;;;
;;; EXPECTS:
;;;     - fn_project_file to give a zero-terminated file name to load from.
;;;     - pala_data to be a buffer we can read PALA_BUFFER_SIZE bytes into.
;;;     - gs to point to the segment holding the pala_data buffer.
;;;     - fs to be a copy of ds, the general data segment.
;;; DESTROYS:
;;;     (- unknown)
;;; RETURNS:
;;;     (- unknown)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Save_Palat_File:
    push ds                                 ; prepare to temporarily switch segments.

    mov bx,[fh_project_file]                ; get the project's file handle. we assume the handle is already open.

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; switch to the segment that holds the palat data buffer we want to read into.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov ax,gs
    mov ds,ax

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; write the palat data into the project's .dta file.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; seek to the start of the data.
    mov eax,[fs:file_pala_data_start]       ; get the byte offset in the .dta file where the palat data begins.
    add eax,4                               ; skip the long int describing the following data's length.
    mov dx,ax                               ; dx = the least significant bit of the offset.
    shr eax,16                              ; move the most significant byte to ax.
    mov cx,ax                               ; cx = the most significant bit of the offset.
    mov ax,4200h                            ; set to move file position, offset from beginning.
    int 21h                                 ; seek.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).
    ; write the data.
    mov cx,PALA_BUFFER_SIZE                 ; set to write the entire palat buffer.
    mov dx,pala_data
    mov ah,40h
    int 21h                                 ; write
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).

    jmp .exit_success

    .exit_fail:
    mov al,0
    jmp .exit

    .exit_success:
    mov al,1
    jmp .exit

    .exit:
    pop ds
    ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Loads the track's specific data from the game's sandboxed executables.
;;;
;;; EXPECTS:
;;;     (- unknown)
;;; DESTROYS:
;;;     (- unknown)
;;; RETURNS:
;;;     (- unknown)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Load_Sandbox_Data:
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; open the sandboxed ~~LLYE.EXE.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    xor cx,cx
    mov dx,fn_sb_rallye_exe                 ; file name.
    mov ah,3dh                              ; set to open.
    mov al,0                                ; read only.
    int 21h                                 ; do it.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).
    mov [fh_sb_rallye_exe],ax               ; save the file handle.

    call Load_Track_IDs
    cmp al,1
    jne .exit_fail

    call Load_Palette
    cmp al,1
    jne .exit_fail

    jmp .exit_success

    .exit_fail:
    mov al,0
    jmp .exit

    .exit_success:
    mov al,1
    jmp .exit

    .exit:
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; close the sandboxed executable files.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov ah,3eh                              ; set to close.
    mov bx,[fh_sb_rallye_exe]
    int 21h

    ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Loads the map maasto and palat IDs from ~~LLYE.EXE.
;;;
;;; EXPECTS:
;;;     (- unknown)
;;; DESTROYS:
;;;     (- unknown)
;;; RETURNS:
;;;     (- unknown)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Load_Track_IDs:
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; read maasto id.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; seek to the correct byte offset.
    mov ebx,20878h                          ; maasto.00_x_.
    mov dx,bx                               ; dx = lowest bits of the offset.
    shr ebx,16
    mov cx,bx                               ; cx = highest bits of the offset.
    mov bx,[fh_sb_rallye_exe]
    mov ax,4200h                            ; set to move file position, offset from the beginning.
    int 21h                                 ; move file position.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).
    ; read in the maasto id.
    mov dx,track_id
    mov cx,1
    mov ah,3fh
    int 21h                                 ; read.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).
    sub [track_id],'1'                      ; convert the track id to zero-based decimal.
    test [track_id],11111000b                ; make sure the track is is in the range 0-7.
    jnz .exit_fail

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; read palat id.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; seek to the correct byte offset.
    mov cx,0
    mov dx,21
    mov ax,4201h                            ; set to move file position, offset from current position.
    int 21h                                 ; move file position.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).
    ; read in the palat id.
    mov dx,palat_id
    mov cx,1
    mov ah,3fh
    int 21h                                 ; read.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).
    mov al,[palat_id]
    mov [pala_file_str+9],al

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
;;; Loads the game's palette from the RALLYE.EXE executable.
;;;
;;; EXPECTS:
;;;     (- unknown)
;;; DESTROYS:
;;;     (- unknown)
;;; RETURNS:
;;;     (- unknown)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Load_Palette:
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; calculate the file offset for the given color in this track's palette in ~~LLYE.EXE.
    ; the index will be in cx:dx, from where int 21h, ah = 42h will read it.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    movzx bx,byte [track_id]
    movzx ebx,byte [track_palette_id+bx]    ; get which palette the track uses.
    lea ebx,dword [palette_offset+(ebx*4)]  ;
    mov ebx,[ebx]                           ; ebx = byte offset in RALLYE.EXE where this track's palette begins.
    mov dx,bx                               ; dx = lowest bits of the offset.
    shr ebx,16
    mov cx,bx                               ; cx = highest bits of the offset.

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; read in the palette.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; seek to the palette's byte offset in ~~LLYE.EXE.
    mov bx,[fh_sb_rallye_exe]               ; file handle to the sandboxed ~~LLYE.EXE file.
    mov ax,4200h                            ; set to move file position, offset from beginning.
    int 21h                                 ; move file position.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).
    ; read in the palette.
    mov dx,palette
    mov ah,3fh
    mov cx,96                               ; 32 colors, each 3 bytes.
    int 21h
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).
    cmp ax,96                               ; make sure we read just the right number of bytes.
    jne .exit_fail

    jmp .exit_success

    .exit_fail:
    mov al,0
    jmp .exit

    .exit_success:
    mov al,1
    jmp .exit

    .exit:
    ret
