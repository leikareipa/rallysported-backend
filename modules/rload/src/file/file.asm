;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;
;;; Subroutines for dealing with files.
;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Sandboxes RALLYE.EXE into ~~LLYE.EXE, i.e. duplicates the data from the former to the latter.
;;;
;;; EXPECTS:
;;;     (- unknown)
;;; DESTROYS:
;;;     - ax, bx, cx, dx, si, di
;;; RETURNS:
;;;     - al set to 1 if we succeeded in duplicating the file, 0 otherwise.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Sandbox_RALLYE_EXE:
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; create the placeholder ~~LLYE.EXE file. this truncates the file if it already exists.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    xor cx,cx
    mov dx,fn_sb_rallye_exe                 ; file name.
    mov ah,3ch                              ; set to create/truncate.
    int 21h                                 ; do it.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; open the sandboxed ~~LLYE.EXE.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    xor cx,cx
    mov dx,fn_sb_rallye_exe                 ; file name.
    mov ah,3dh                              ; set to open.
    mov al,0010b                            ; read/write.
    int 21h                                 ; do it.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).
    mov [fh_sb_rallye_exe],ax               ; save the file handle.
    mov di,ax

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; open the real RALLYE.EXE.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    xor cx,cx
    mov dx,fn_rallye_exe                    ; file name.
    mov ah,3dh                              ; set to open.
    mov al,0                                ; read only.
    int 21h                                 ; do it.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).
    mov [fh_rallye_exe],ax                  ; save the file handle.
    mov si,ax

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; test the file size of RALLYE.EXE to make sure it's the demo version.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov dx,0                                ; dx = lowest bits of the offset.
    mov cx,0                                ; cx = highest bits of the offset.
    mov bx,si
    mov ax,4202h                            ; seek from end.
    int 21h                                 ; move file position.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).

    mov bx,dx
    shl ebx,16
    mov bx,ax
    cmp ebx,133452                          ; the demo RALLYE.EXE is supposed to be 133452 bytes.
    je .seek_to_beg
    mov dx,err_file_size
    mov ah,9h
    int 21h
    jmp .exit_fail

    .seek_to_beg:
    mov bx,si
    mov dx,0                                ; dx = lowest bits of the offset.
    mov cx,0                                ; cx = highest bits of the offset.
    mov ax,4200h                            ; seek from beginning.
    int 21h                                 ; return to the start of the file.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; copy the data over to the duplicate file.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    call Duplicate_File_Data

    push ax
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; close the RALLYE.EXE file.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov ah,3eh                              ; set to close.
    mov bx,[fh_rallye_exe]
    int 21h                                 ; do it.
    pop ax

    cmp al,1                                ; see if everything went well with duplicating the data.
    je .exit

    .exit_fail:
    mov al,0
    jmp .exit

    .exit:
    ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Sandboxes VALIKKO.EXE into ~~LIKKO.EXE, i.e. duplicates the data from the former to the latter.
;;;
;;; EXPECTS:
;;;     (- unknown)
;;; DESTROYS:
;;;     - ax, bx, cx, dx, si, di
;;; RETURNS:
;;;     - al set to 1 if we succeeded in duplicating the file, 0 otherwise.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Sandbox_VALIKKO_EXE:
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; create the placeholder ~~LIKKO.EXE file. this truncates the file if it already exists.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    xor cx,cx
    mov dx,fn_sb_valikko_exe                ; file name.
    mov ah,3ch                              ; set to create/truncate.
    int 21h                                 ; do it.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; open the sandboxed ~~LIKKO.EXE.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    xor cx,cx
    mov dx,fn_sb_valikko_exe                ; file name.
    mov ah,3dh                              ; set to open.
    mov al,0010b                            ; read/write.
    int 21h                                 ; do it.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).
    mov [fh_sb_valikko_exe],ax              ; save the file handle.
    mov di,ax

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; open the real VALIKKO.EXE.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    xor cx,cx
    mov dx,fn_valikko_exe                   ; file name.
    mov ah,3dh                              ; set to open.
    mov al,0                                ; read only.
    int 21h                                 ; do it.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).
    mov [fh_valikko_exe],ax                 ; save the file handle.
    mov si,ax

    call Duplicate_File_Data

    push ax
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; close the VALIKKO.EXE file.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov ah,3eh                              ; set to close.
    mov bx,[fh_valikko_exe]
    int 21h                                 ; do it.
    pop ax

    cmp al,1                                ; see if everything went well with duplicating the data.
    je .exit

    .exit_fail:
    mov al,0
    jmp .exit

    .exit:
    ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Sandboxes RESULTS.DTA into ~~SULTS.DTA, i.e. duplicates the data from the former to the latter.
;;;
;;; EXPECTS:
;;;     (- unknown)
;;; DESTROYS:
;;;     (- unknown)
;;; RETURNS:
;;;     (- unknown)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Sandbox_RESULTS_DTA:
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; create the placeholder ~~SULTS.DTA file. this truncates the file if it already exists.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    xor cx,cx
    mov dx,fn_sb_results_dta                ; file name.
    mov ah,3ch                              ; set to create/truncate.
    int 21h                                 ; do it.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; open the sandboxed ~~SULTS.DTA.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    xor cx,cx
    mov dx,fn_sb_results_dta                ; file name.
    mov ah,3dh                              ; set to open.
    mov al,1                                ; write only.
    int 21h                                 ; do it.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; fill in the first 2 bytes as ff ff to indicate to rally-sport not to load in these results, and save them
    ; into the file.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov bx,ax
    mov ax,0ffffh
    mov word [file_buffer],ax
    mov dx,file_buffer
    mov cx,2                                ; the file name block is 145 bytes.
    mov ah,40h
    int 21h                                 ; write.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; close the ~~SULTS.DTA file.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    push ax                                 ; preserve al, since it's our return value.
    mov ah,3eh                              ; set to close.
    int 21h                                 ; do it.
    pop ax

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
;;; Duplicates the source file's data into the destination file.
;;;
;;; EXPECTS:
;;;     - si to give the file handle of the source file.
;;;     - di to give the file handle of the target file.
;;;     - ds:file:buffer to be a buffer that can take 64 kb of data.
;;; DESTROYS:
;;;     - ax, bx, cx, dx
;;; RETURNS:
;;;     - al set to 1 if we succeeded in duplicating the file, 0 otherwise.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Duplicate_File_Data:
    mov dx,file_buffer

    .copy_data:
        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        ; read the data.
        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        mov bx,si
        mov ah,3fh
        mov cx,FILE_BUFFER_SIZE             ; read the maximum number of bytes. the number of bytes that were actually read will be set in ax after the call to int 21h.
        int 21h
        jc .exit_fail                       ; error-checking (the cf flag will be set by int 21h if there was an error).

        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        ; test for end of file.
        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        cmp ax,0
        je .exit_success

        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        ; write the data from RALLYE.EXE into the sandboxed ~~LLYE.EXE.
        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        mov cx,ax                           ; copy as many bytes as we read.
        mov bx,di
        mov ah,40h
        int 21h
        jc .exit_fail                       ; error-checking (the cf flag will be set by int 21h if there was an error).

        jmp .copy_data

    .exit_fail:
    mov al,0
    jmp .exit

    .exit_success:
    mov al,1
    jmp .exit

    .exit:
    ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Extracts the project files from the project's .dta file.
;;;
;;; EXPECTS:
;;;     (- unknown)
;;; DESTROYS:
;;;     (- unknown)
;;; RETURNS:
;;;     - al set to 1 if we succeeded, 0 otherwise.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Extract_Project_Files:
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; open the project's .dta file.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    xor cx,cx
    mov dx,fn_project_file                  ; file name.
    mov ah,3dh                              ; set to open.
    mov al,0                                ; read only.
    int 21h                                 ; do it.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).
    mov [fh_project_file],ax                ; save the file handle.

    ; extract a file.
    mov dx,fn_sb_maasto_dta
    call Extract_Project_File
    cmp al,1
    jne .exit_fail

    mov dx,fn_sb_varimaa_dta
    call Extract_Project_File
    cmp al,1
    jne .exit_fail

    mov dx,fn_sb_palat_dta
    call Extract_Project_File
    cmp al,1
    jne .exit_fail

    mov dx,fn_sb_anims_dta
    call Extract_Project_File
    cmp al,1
    jne .exit_fail

    mov dx,fn_sb_text1_dta
    call Extract_Project_File
    cmp al,1
    jne .exit_fail

    mov dx,fn_sb_kierros_dta
    call Extract_Project_File
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
    push ax                                 ; preserve al, since it's our return value.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; close the project file.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov bx,[fh_project_file]
    mov ah,3eh                              ; set to close.
    int 21h                                 ; do it.
    pop ax

    ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Extracts the next file from the project's .dta file.
;;;
;;; EXPECTS:
;;;     - si to contain the file handle to the project's .dta file.
;;;     - dx to be a pointer to the name of the file to extract the data to.
;;; DESTROYS:
;;;     (- unknown)
;;; RETURNS:
;;;     - al set to 1 if we succeeded, 0 otherwise.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Extract_Project_File:
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; create the placeholder target file. this truncates the file if it already exists.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    xor cx,cx
    mov ah,3ch                              ; set to create/truncate.
    int 21h                                 ; do it.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; open the target file.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    xor cx,cx
    mov ah,3dh                              ; set to open.
    mov al,1                                ; write only.
    int 21h                                 ; do it.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).
    mov [fh_temp],ax                        ; store the file handle.

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; read the length of the data from the project's .dta file.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov bx,[fh_project_file]
    mov dx,file_buffer
    mov ah,3fh
    mov cx,4                                ; 4 bytes == long int.
    int 21h
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; the file can theoretically have a data length of > 64k, but we can only read 64k of
    ; it at most. we'll make a note if the data overflows the 64k boundary, so we'll read
    ; up to 64k and seek forward past the rest.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    .numOverflowBytes dw ?
    mov [.numOverflowBytes],0
    mov ecx,dword [file_buffer]
    sub ecx,0ffffh
    js .extract                              ; if the result is negative, the data don't overflow 64k.
    mov [.numOverflowBytes],cx
    mov dword [file_buffer],0ffffh           ; limit data length to 64k max, so we won't attempt to read past it.

    .extract:

    mov si,word [file_buffer]
    mov di,FILE_BUFFER_SIZE
    .read_write:
        cmp di,si
        ja .swap
        jmp .no_swap

        .swap:
        xchg di,si

        .no_swap:

        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        ; read the data from the project's .dta file into memory.
        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        mov bx,[fh_project_file]            ; file handle.
        mov ah,3fh
        mov cx,di                           ; read at most as many bytes as can fit into our memory buffer, but no more than the remaining length of data we want.
        int 21h
        jc .exit_fail                       ; error-checking (the cf flag will be set by int 21h if there was an error).

        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        ; write the data into the sandboxed track file.
        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        mov bx,[fh_temp]                    ; file handle.
        mov cx,ax                           ; write the number of bytes that we read.
        mov ah,40h
        int 21h
        jc .exit_fail                       ; error-checking (the cf flag will be set by int 21h if there was an error).

        sub si,ax

        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        ; continue reading and writing until the data size we're moving is less than the size of our file buffer, which
        ; would mean we've read to the end of the file.
        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        cmp ax,FILE_BUFFER_SIZE
        je .read_write                      ; read until we've reached the end of the file.

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; seek forward past any data that overflows the 64k boundary.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov bx,[fh_project_file]
    mov cx,0                                ; cx = highest bits of the offset.
    mov dx,[.numOverflowBytes]              ; dx = lowest bits of the offset.
    mov ax,4201h                            ; seek from current position.
    int 21h                                 ; seek.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).

    jmp .exit_success

    .exit_fail:
    mov al,0
    jmp .exit

    .exit_success:
    mov al,1
    jmp .exit

    .exit:
    push ax                                 ; the call below to close the file destroys al, our return value, so preserve it.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; close the target file.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov bx,[fh_temp]
    mov ah,3eh                              ; set to close.
    int 21h                                 ; do it.
    pop ax

    ret
