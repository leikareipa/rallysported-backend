;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;
;;; Subroutines for applying all the possible manifesto commands.
;;;
;;; The id of the command is given in the first byte of ds:manifesto_cmd, and the subsequent bytes give
;;; the command's parameters. The total number of parameters is given in ds:manifesto_cmd_len.
;;;
;;; The routines return al = 1 if successful, otherwise al is set to 0.
;;;
;;; NOTE WELL: The routines must preserve si.
;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Sets the maasto and palat ids for the map, and the minimum rsed-loader version number.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Command_0_REQUIRE:
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; we expect 3 parameters + the command itself.
    ; p1 = maasto id.
    ; p2 = palat id.
    ; p3 = rsed loader version required.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    cmp [manifesto_cmd_len],4
    je .param_num_is_good
    mov [err_manifesto_additional],err_manifesto_parameters
    jmp .exit_fail
    .param_num_is_good:

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; make sure the track doesn't require a loader version newer than what we have.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    cmp byte [manifesto_cmd+3],LOADER_MAJOR_VERSION
    jle .version_is_good
    mov [err_manifesto_additional],err_manifesto_fail_version
    jmp .exit_fail
    .version_is_good:                       ; we've got a loader version compatible with this track.

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; make sure the maasto and palat ids are within valid ranges.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    sub [manifesto_cmd+1],1                 ; convert to 0-indexed.
    sub [manifesto_cmd+2],1                 ; convert to 0-indexed.
    test [manifesto_cmd+1],11111000b        ; see that the maasto index is in the range 0-7.
    jnz .param_range_is_bad
    test [manifesto_cmd+2],11111110b        ; see that the palat index is in the range 0-1.
    jnz .param_range_is_bad
    jmp .param_range_is_good

    .param_range_is_bad:
    mov [err_manifesto_additional],err_manifesto_parameter_range
    jmp .exit_fail

    .param_range_is_good:

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; store the maasto and palat ids for later use.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov ax,word [manifesto_cmd+1]           ; al = maasto, ah = palat.
    mov [track_id],al
    mov [palat_id],ah

    add [manifesto_cmd+1],'1'               ; convert to ascii and back to 1-indexed.
    add [manifesto_cmd+2],'1'               ; convert to ascii and convert back to 1-indexed.

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; store the new data in the file buffer from which we can write them to disk.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov ax,word [manifesto_cmd+1]
    mov word [file_buffer],ax

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; patch into ~~LLYE.EXE.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; patch the kierros index.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov ebx,20869h                          ; the start of 'kierrosx.dta' in the file name block.
    mov dx,bx                               ; dx = lowest bits of the offset.
    shr ebx,16
    mov cx,bx                               ; cx = highest bits of the offset.
    mov bx,[fh_sb_rallye_exe]
    mov ax,4200h                            ; set to move file position, offset from the beginning.
    int 21h                                 ; move file position.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).
    mov dx,file_buffer
    mov cx,1                                ; write 1 byte.
    mov ah,40h
    int 21h                                 ; write the new maasto index.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; patch the maasto index.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov cx,0                                ; cx:dx = new file offset.
    mov dx,14                              ; set to move to where we have 'maasto.001' in the exe.
    mov ax,4201h                            ; set to move file position, offset from the current.
    int 21h                                 ; move file position.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).
    mov dx,file_buffer
    mov cx,1                                ; write 1 byte.
    mov ah,40h
    int 21h                                 ; write the new maasto index.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; patch the varimaa index.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov cx,0                               ; cx:dx = new file offset.
    mov dx,11                              ; set to move to where we have 'varimaa.001' in the exe.
    mov ax,4201h                            ; set to move file position, offset from the current.
    int 21h                                 ; move file position.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).
    mov dx,file_buffer
    mov cx,1                                ; write 1 byte.
    mov ah,40h
    int 21h                                 ; write the new maasto index.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; patch the palat index.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov cx,0                                ; cx:dx = new file offset.
    mov dx,9                               ; set to move to where we have 'palat.001' in the exe.
    mov ax,4201h                            ; set to move file position, offset from current position.
    int 21h                                 ; move file position.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).
    mov dx,file_buffer+1
    mov cx,1                                ; write 1 byte.
    mov ah,40h
    int 21h                                 ; write the new maasto index.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).
    ;; TODO: patch the track header too.

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; patch into ~~LIKKO.EXE.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; patch the kierros index.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov ebx,258b7h                          ; the start of 'kierrosx.dta' in the file name block.
    mov dx,bx                               ; dx = lowest bits of the offset.
    shr ebx,16
    mov cx,bx                               ; cx = highest bits of the offset.
    mov bx,[fh_sb_valikko_exe]
    mov ax,4200h                            ; set to move file position, offset from the beginning.
    int 21h                                 ; move file position.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).
    mov dx,file_buffer
    mov cx,1                                ; write 1 byte.
    mov ah,40h
    int 21h                                 ; write the new maasto index.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; patch the maasto index.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov cx,0                                ; cx:dx = new file offset.
    mov dx,14                              ; set to move to where we have 'maasto.001' in the exe.
    mov ax,4201h                            ; set to move file position, offset from the current.
    int 21h                                 ; move file position.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).
    mov dx,file_buffer
    mov cx,1                                ; write 1 byte.
    mov ah,40h
    int 21h                                 ; write the new maasto index.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; patch the varimaa index.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov cx,0                               ; cx:dx = new file offset.
    mov dx,11                              ; set to move to where we have 'varimaa.001' in the exe.
    mov ax,4201h                            ; set to move file position, offset from the current.
    int 21h                                 ; move file position.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).
    mov dx,file_buffer
    mov cx,1                                ; write 1 byte.
    mov ah,40h
    int 21h                                 ; write the new maasto index.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; patch the palat index.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov cx,0                                ; cx:dx = new file offset.
    mov dx,9                               ; set to move to where we have 'palat.001' in the exe.
    mov ax,4201h                            ; set to move file position, offset from current position.
    int 21h                                 ; move file position.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).
    mov dx,file_buffer+1
    mov cx,1                                ; write 1 byte.
    mov ah,40h
    int 21h                                 ; write the new maasto index.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).
    ;; TODO: patch the track header too.

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
;;; Sets the maasto and palat ids for the map, and the minimum rsed-loader version number.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Command_2_SET_NUM_OBJECTS:
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; we expect 1 parameter + the command itself.
    ; p1 = the new number of objects.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    cmp [manifesto_cmd_len],2
    je .param_num_is_good
    mov [err_manifesto_additional],err_manifesto_parameters
    jmp .exit_fail
    .param_num_is_good:

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; make sure the number of objects to be set is within a valid range (1 to n, where n is the number of objects on this track by default).
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov al,[max_num_objects_on_track]
    cmp [manifesto_cmd+1],al
    ja .param_range_is_bad
    cmp [manifesto_cmd+1],1
    jl .param_range_is_bad
    jmp .param_range_is_good

    .param_range_is_bad:
    mov [err_manifesto_additional],err_manifesto_parameter_range
    jmp .exit_fail

    .param_range_is_good:

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; write the new number of objects to ~~LLYE.EXE.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; seek to the byte in ~~LLYE.EXE that sets the number of objects on this track.
    movzx ebx,[track_id]
    lea di,[object_block_offs+(ebx*4)]      ; di = address in the block of offset addresses to the starting byte of this track's offset.
    mov ebx,dword [di]                      ; ebx now points to the first byte in the game executable that defines the track's objects.
    sub ebx,2                               ; ebx now points to the byte which sets the number of objects on this track.
    mov dx,bx                               ; dx = lowest bits of the offset.
    shr ebx,16
    mov cx,bx                               ; cx = highest bits of the offset.
    mov bx,[fh_sb_rallye_exe]
    mov ax,4200h                            ; set to move file position, offset from the beginning.
    int 21h                                 ; move file position.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).
    ; write the new number of objects in.
    mov dx,manifesto_cmd+1                  ; write the manifesto command directly.
    mov cx,1                                ; write 1 byte.
    mov ah,40h
    int 21h                                 ; write.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; write the new number of objects to ~~LIKKO.EXE.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; seek to the byte in ~~LLYE.EXE that sets the number of objects on this track.
    movzx ebx,[track_id]
    lea di,[object_block_offs_valikko+(ebx*4)]; di = address in the block of offset addresses to the starting byte of this track's offset.
    mov ebx,dword [di]                      ; ebx now points to the first byte in the game executable that defines the track's objects.
    sub ebx,2                               ; ebx now points to the byte which sets the number of objects on this track.
    mov dx,bx                               ; dx = lowest bits of the offset.
    shr ebx,16
    mov cx,bx                               ; cx = highest bits of the offset.
    mov bx,[fh_sb_valikko_exe]
    mov ax,4200h                            ; set to move file position, offset from the beginning.
    int 21h                                 ; move file position.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).
    ; write the new number of objects in.
    mov dx,manifesto_cmd+1                  ; write the manifesto command directly.
    mov cx,1                                ; write 1 byte.
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

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Adds a new object onto the track.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Command_3_ADD_OBJECT:
    push si

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; we expect 5 parameters + the command itself.
    ; p1 = object type.
    ; p2 = tile x position.
    ; p3 = tile y position.
    ; p4 = local tile x position.
    ; p5 = local tile y position.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    cmp [manifesto_cmd_len],6
    je .param_num_is_good
    mov [err_manifesto_additional],err_manifesto_parameters
    jmp .exit_fail
    .param_num_is_good:

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; verify that the parameters are correct.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    sub [manifesto_cmd+1],1                 ; convert object idx to 0-indexed.
    js .param_range_is_bad
    mov ah,byte [num_object_types]
    cmp [manifesto_cmd+1],ah                ; make sure the object id doesn't point to an object that doesn't exist on this track.
    jge .param_range_is_bad
    test [manifesto_cmd+2],10000000b        ; see that the global x coordinate is within 0-127.
    jnz .param_range_is_bad
    test [manifesto_cmd+3],10000000b        ; see that the global y coordinate is within 0-127.
    jnz .param_range_is_bad
    jmp .param_range_is_good

    .param_range_is_bad:
    mov [err_manifesto_additional],err_manifesto_parameter_range
    jmp .exit_fail

    .param_range_is_good:

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; write the new object's header into the executable, at the end of the existing object block.
    ; take note that we overwrite any existing data there, with the assumption that it's not program-critical
    ; code but rather an object block for the next track (which we won't play while the loader runs, so its
    ; data being overwritten shouldn't matter).
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; calculate the position of the given object type's header in the loader's lookup table.
    mov al,[manifesto_cmd+1]
    mov cl,6                                ; the number of bytes per object header.
    mul cl
    mov si,ax                               ; si is now the starting position for our object header.

    ; get the desired object type's id string.
    xor bx,bx
    mov ecx,6
    .type_string:
        mov al,[object_header+si+bx]
        mov [file_buffer+bx],al
        inc bx
        loop .type_string

    ; set the object's position to the user-supplied coordinates.
    mov al,[manifesto_cmd+4]                ; local x.
    mov [file_buffer+6],al
    mov al,[manifesto_cmd+2]                ; global x.
    mov [file_buffer+7],al
    mov al,[manifesto_cmd+5]                ; local y.
    mov [file_buffer+8],al
    mov al,[manifesto_cmd+3]                ; global y.
    mov [file_buffer+9],al
    mov word [file_buffer+10],word 0xffff   ; height.

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; write the new data into ~~LLYE.EXE.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; find the last byte in this track's object block.
    movzx ebx,[track_id]
    lea di,[object_block_offs+(ebx*4)]      ; di = address in the block of offset addresses to the starting byte of this track's offset.
    mov ebx,dword [di]                      ; ebx now points to the first byte in the game executable that defines the track's objects.
    movzx eax,[num_objects_on_track]
    mov cl,12                               ; the number of bytes in the object header.
    mul cl
    add ebx,eax                             ; ebx now points to the last byte in this track's object block.
    mov dx,bx                               ; dx = lowest bits of the offset.
    shr ebx,16
    mov cx,bx                               ; cx = highest bits of the offset.
    ; patch the object's new header into the executable.
    mov bx,[fh_sb_rallye_exe]               ; file handle.
    mov ax,4200h                            ; set to move file position, offset from the beginning.
    int 21h                                 ; move file position.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).
    lea dx,[file_buffer]                    ; get the address to the target object type's header.
    mov cx,12                               ; write 12 bytes, i.e. the entire object header.
    mov ah,40h
    int 21h                                 ; write the new maasto index.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; write the new data into ~~LIKKO.EXE.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; find the last byte in this track's object block.
    movzx ebx,[track_id]
    lea di,[object_block_offs_valikko+(ebx*4)] ; di = address in the block of offset addresses to the starting byte of this track's offset.
    mov ebx,dword [di]                      ; ebx now points to the first byte in the game executable that defines the track's objects.
    movzx eax,[num_objects_on_track]
    mov cl,12                               ; the number of bytes in the object header.
    mul cl
    add ebx,eax                             ; ebx now points to the last byte in this track's object block.
    mov dx,bx                               ; dx = lowest bits of the offset.
    shr ebx,16
    mov cx,bx                               ; cx = highest bits of the offset.
    ; patch the object's new header into the executable.
    mov bx,[fh_sb_valikko_exe]              ; file handle.
    mov ax,4200h                            ; set to move file position, offset from the beginning.
    int 21h                                 ; move file position.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).
    lea dx,[file_buffer]                    ; get the address to the target object type's header.
    mov cx,12                               ; write 12 bytes, i.e. the entire object header.
    mov ah,40h
    ;int 21h                                 ; write the new maasto index.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; increase the track's object count by one.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov al,[num_objects_on_track]
    inc al
    mov [num_objects_on_track],al
    cmp al,[max_num_objects_on_track]       ; make sure we're not adding too many new objects, seeing as how their data overwrites subsequent bytes in the executable.
                                            ; FIXME: for the last track (#8) there may be room in the executable for only about six new objects.
    jg .param_range_is_bad

    ; patch the count into ~~LLYE.EXE.
    mov [file_buffer], byte al
    movzx ebx,[track_id]
    lea di,[object_block_offs+(ebx*4)]      ; di = address in the block of offset addresses to the starting byte of this track's offset.
    mov ebx,dword [di]                      ; ebx now points to the first byte in the game executable that defines the track's objects.
    sub ebx,2                               ; move to the byte which gives the total count of objects.
    mov dx,bx                               ; dx = lowest bits of the offset.
    shr ebx,16
    mov cx,bx                               ; cx = highest bits of the offset.
    mov bx,[fh_sb_rallye_exe]
    mov ax,4200h                            ; set to move file position, offset from the beginning.
    int 21h                                 ; move file position.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).
    mov dx,file_buffer
    mov cx,1                                ; write 1 byte.
    mov ah,40h
    int 21h                                 ; write the new maasto index.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).

    ; patch the count into ~~LIKKO.EXE.
    ;;; FIXME: this doesn't work at the moment, so it's been disabled.
    movzx ebx,[track_id]
    lea di,[object_block_offs_valikko+(ebx*4)] ; di = address in the block of offset addresses to the starting byte of this track's offset.
    mov ebx,dword [di]                      ; ebx now points to the first byte in the game executable that defines the track's objects.
    sub ebx,2                               ; move to the byte which gives the total count of objects.
    mov dx,bx                               ; dx = lowest bits of the offset.
    shr ebx,16
    mov cx,bx                               ; cx = highest bits of the offset.
    mov bx,[fh_sb_valikko_exe]
    mov ax,4200h                            ; set to move file position, offset from the beginning.
    int 21h                                 ; move file position.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).
    mov dx,file_buffer
    mov cx,1                                ; write 1 byte.
    mov ah,40h
    ;int 21h                                 ; write the new maasto index.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).

    jmp .exit_success

    .exit_fail:
    mov al,0
    jmp .exit

    .exit_success:
    mov al,1
    jmp .exit

    .exit:
    pop si
    ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Changes the type of the given object.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Command_4_CHANGE_OBJECT_TYPE:
    push si

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; we expect 2 parameters + the command itself.
    ; p1 = target object id.
    ; p2 = new object type to assign to the target object.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    cmp [manifesto_cmd_len],3
    je .param_num_is_good
    mov [err_manifesto_additional],err_manifesto_parameters
    jmp .exit_fail
    .param_num_is_good:

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; make sure the target object id and the new object type are within valid ranges.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov ah,byte [num_objects_on_track]
    cmp [manifesto_cmd+1],ah                ; make sure the object id doesn't point to an object that doesn't exist on this track.
    jg .param_range_is_bad
    sub [manifesto_cmd+1],1                 ; convert target id to 0-indexed.
    js .param_range_is_bad
    sub [manifesto_cmd+2],1                 ; convert object type to 0-indexed.
    test [manifesto_cmd+2],11110000b        ; see that the object type is in the range 0-15, as there are 16 possible object types.
    jnz .param_range_is_bad
    jmp .param_range_is_good

    .param_range_is_bad:
    mov [err_manifesto_additional],err_manifesto_parameter_range
    jmp .exit_fail

    .param_range_is_good:

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; calculate the position of the given object type's header in the loader's lookup table.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov al,[manifesto_cmd+2]
    mov cl,6                                ; the number of bytes per object header.
    mul cl
    mov si,ax                               ; si is now the starting position for our object header.

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; calculate the file offset for this object's header in ~~LLYE.EXE.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    movzx ebx,[track_id]
    lea di,[object_block_offs+(ebx*4)]      ; di = address in the block of offset addresses to the starting byte of this track's offset.
    mov ebx,dword [di]                      ; ebx now points to the first byte in the game executable that defines the track's objects.
    movzx eax,byte [manifesto_cmd+1]        ; eax = the target object.
    mov ecx,12
    mul ecx                                 ; we multiply the target object id by 12, the number of bytes per object header, to get the relative offset of that object's header.
    add ebx,eax
    mov dx,bx                               ; dx = lowest bits of the offset.
    shr ebx,16
    mov cx,bx                               ; cx = highest bits of the offset.

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; patch the object's new header into ~~LLYE.EXE.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov bx,[fh_sb_rallye_exe]               ; file handle.
    mov ax,4200h                            ; set to move file position, offset from the beginning.
    int 21h                                 ; move file position.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).
    lea dx,[object_header+si]               ; get the address to the target object type's header.
    mov cx,6                                ; write 6 bytes, i.e. the entire object header.
    mov ah,40h
    int 21h                                 ; write the new maasto index.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; calculate the file offset for this object's header in ~~LIKKO.EXE.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    movzx ebx,[track_id]
    lea di,[object_block_offs_valikko+(ebx*4)] ; di = address in the block of offset addresses to the starting byte of this track's offset.
    mov ebx,dword [di]                      ; ebx now points to the first byte in the game executable that defines the track's objects.
    movzx eax,byte [manifesto_cmd+1]        ; eax = the target object.
    mov ecx,12
    mul ecx                                 ; we multiply the target object id by 12, the number of bytes per object header, to get the relative offset of that object's header.
    add ebx,eax
    mov dx,bx                               ; dx = lowest bits of the offset.
    shr ebx,16
    mov cx,bx                               ; cx = highest bits of the offset.

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; patch the object's new header into ~~LIKKO.EXE.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov bx,[fh_sb_valikko_exe]               ; file handle.
    mov ax,4200h                            ; set to move file position, offset from the beginning.
    int 21h                                 ; move file position.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).
    lea dx,[object_header_valikko+si]       ; get the address to the target object type's header.
    mov cx,6                                ; write 6 bytes, i.e. the entire object header.
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
    pop si
    ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Moves the given object.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Command_5_MOVE_OBJECT:
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; we expect 5 parameters + the command itself.
    ; p1 = the id of the object to be moved.
    ; p2 = the global x position of the object (note that this is the actual tile x coordinate divided by 2).
    ; p3 = the global y position of the object (note that this is the actual tile x coordinate divided by 2).
    ; p4 = the local x position of the object (value range: 0-255).
    ; p5 = the local y position of the object (value range: 0-255).
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    cmp [manifesto_cmd_len],6
    je .param_num_is_good
    mov [err_manifesto_additional],err_manifesto_parameters
    jmp .exit_fail
    .param_num_is_good:

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; make sure the target object id and the new object type are within valid ranges.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov ah,byte [num_objects_on_track]
    cmp [manifesto_cmd+1],ah                ; make sure the object id doesn't point to an object that doesn't exist on this track.
    jg .param_range_is_bad
    sub [manifesto_cmd+1],1                 ; convert target id to 0-indexed.
    js .param_range_is_bad
    test [manifesto_cmd+2],10000000b        ; see that the global x coordinate is within 0-127.
    jnz .param_range_is_bad
    test [manifesto_cmd+3],10000000b        ; see that the global y coordinate is within 0-127.
    jnz .param_range_is_bad
    jmp .param_range_is_good

    .param_range_is_bad:
    mov [err_manifesto_additional],err_manifesto_parameter_range
    jmp .exit_fail

    .param_range_is_good:

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; calculate the file offset for this object's header in ~~LLYE.EXE.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    movzx ebx,[track_id]
    lea di,[object_block_offs+(ebx*4)]      ; di = address in the block of offset addresses to the starting byte of this track's offset.
    mov ebx,dword [di]                      ; ebx now points to the first byte in the game executable that defines the track's objects.
    movzx eax,byte [manifesto_cmd+1]        ; eax = the target object.
    mov ecx,12
    mul ecx                                 ; we multiply the target object id by 12, the number of bytes per object header, to get the relative offset of that object's header.
    add ebx,eax
    mov dx,bx                               ; dx = lowest bits of the offset.
    shr ebx,16
    mov cx,bx                               ; cx = highest bits of the offset.

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; load the object's header from disk.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov bx,[fh_sb_rallye_exe]               ; file handle to the sandboxed ~~LLYE.EXE file.
    mov ax,4200h                            ; set to move file position, offset from the beginning.
    int 21h                                 ; move file position.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).
    mov dx,file_buffer                      ; load from disk to the file buffer.
    mov cx,12                               ; the object header is 12 bytes.
    mov ah,3fh
    int 21h
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; modify the object's header with the new data.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov al,byte [manifesto_cmd+2]
    mov [file_buffer+7],al               ; global x position.
    mov al,byte [manifesto_cmd+3]
    mov [file_buffer+9],al               ; global y position.
    mov al,byte [manifesto_cmd+4]
    mov [file_buffer+6],al               ; local x position.
    mov al,byte [manifesto_cmd+5]
    mov [file_buffer+8],al               ; local y position.

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; write the modified data back into ~~LLYE.EXE.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov cx,-1                               ; cx:dx = new file offset.
    mov dx,-12                              ; go back 12 bytes to the start of the header block.
    mov ax,4201h                            ; set to move file position, current position.
    int 21h                                 ; move file position.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).
    mov dx,file_buffer
    mov cx,12                               ; the object header is 12 bytes.
    mov ah,40h
    int 21h                                 ; write the new maasto index.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; calculate the file offset for this object's header in ~~LIKKO.EXE.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    movzx ebx,[track_id]
    lea di,[object_block_offs_valikko+(ebx*4)] ; di = address in the block of offset addresses to the starting byte of this track's offset.
    mov ebx,dword[di]                      ; ebx now points to the first byte in the game executable that defines the track's objects.
    movzx eax,byte[manifesto_cmd+1]        ; eax = the target object.
    mov ecx,12
    mul ecx                                 ; we multiply the target object id by 12, the number of bytes per object header, to get the relative offset of that object's header.
    add ebx,eax
    add ebx,6                               ; skip the first 6 bytes of the header.
    mov dx,bx                               ; dx = lowest bits of the offset.
    shr ebx,16
    mov cx,bx                               ; cx = highest bits of the offset.

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; write the modified data back into ~~LIKKO.EXE.
    ; note that we only write the last 4 bytes of the offset, thus skipping the first 6 bytes which contain
    ; pointers which don't apply to ~~LIKKO.EXE as they do for ~~LLYE.EXE.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov bx,[fh_sb_valikko_exe]
    mov ax,4200h                            ; set to move file position, offset from the beginning.
    int 21h                                 ; move file position.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).
    mov dx,file_buffer+6
    mov cx,4
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
;;; Changes the palette's color at a given index.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Command_10_CHANGE_PALETTE_COLOR:
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; we expect 4 parameters + the command itself.
    ; p1 = the palette index of the color to be changed.
    ; p2 = the red color value (value range: 0-63).
    ; p3 = the green color value (value range: 0-63).
    ; p4 = the blue color value (value range: 0-63).
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    cmp [manifesto_cmd_len],5
    je .param_num_is_good
    mov [err_manifesto_additional],err_manifesto_parameters
    jmp .exit_fail
    .param_num_is_good:

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; make sure the palette index and color values are within valid ranges.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov ah,[manifesto_cmd+1]
    sub ah,1                                ; convert the palette index to 0-indexed for testing purposes.
    test ah,11100000b                       ; make sure the palette index is within 0-31.
    jnz .param_range_is_bad
    test [manifesto_cmd+2],11000000b        ; make sure the red channel is within 0-63.
    jnz .param_range_is_bad
    test [manifesto_cmd+3],11000000b        ; make sure the green channel is within 0-63.
    jnz .param_range_is_bad
    test [manifesto_cmd+4],11000000b        ; make sure the blue channel is within 0-63.
    jnz .param_range_is_bad
    jmp .param_range_is_good

    .param_range_is_bad:
    mov [err_manifesto_additional],err_manifesto_parameter_range
    jmp .exit_fail

    .param_range_is_good:

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; calculate the file offset for the given color in this track's palette in ~~LLYE.EXE.
    ; the index will be in cx:dx, from where int 21h, ah = 42h will read it.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    movzx bx,byte [track_id]
    movzx ebx,byte [track_palette_id+bx]    ; get which palette the track uses.
    lea ebx,dword [palette_offset+(ebx*4)]  ;
    mov ebx,[ebx]                           ; ebx = byte offset in RALLYE.EXE where this track's palette begins.
    movzx ax,byte [manifesto_cmd+1]
    mov cl,3
    mul cl                                  ; ax = byte offset of the given palette index relative to the palette's offset.
    add ebx,eax                             ; ebx = the byte offset of the given color in the track's palette.
    mov dx,bx                               ; dx = lowest bits of the offset.
    shr ebx,16
    mov cx,bx                               ; cx = highest bits of the offset.

    mov bx,[fh_sb_rallye_exe]               ; file handle to the sandboxed ~~LLYE.EXE file.

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; load palette entry's data from disk.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov ax,4200h                            ; set to move file position, offset from the beginning.
    int 21h                                 ; move file position to cx:dx.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).
    mov dx,file_buffer                      ; write from disk to the file buffer.
    mov cx,3                                ; each palette entry is 3 bytes.
    mov ah,3fh
    int 21h                                 ; read the palette entry from disk.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; modify the palette entry.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov al,byte [manifesto_cmd+2]
    mov [file_buffer],al                    ; red.
    mov al,byte [manifesto_cmd+3]
    mov [file_buffer+1],al                  ; green.
    mov al,byte [manifesto_cmd+4]
    mov [file_buffer+2],al                  ; blue

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; write the modified data back into ~~LLYE.EXE.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov cx,-1                               ; cx:dx = new file offset.
    mov dx,-3                               ; go back 3 bytes to the start of the palette block.
    mov ax,4201h                            ; set to move file position, offset from start.
    int 21h                                 ; move file position.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).
    mov dx,file_buffer
    mov cx,3                                ; each palette entry is 3 bytes.
    mov ah,40h
    int 21h                                 ; write the new maasto index.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; calculate the file offset for the given color in this track's palette in ~~LIKKO.EXE.
    ; the index will be in cx:dx, from where int 21h, ah = 42h will read it.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    movzx bx,byte [track_id]
    movzx ebx,byte [track_palette_id+bx]    ; get which palette the track uses.
    lea ebx,dword [palette_offset_valikko+(ebx*4)]  ;
    mov ebx,[ebx]                           ; ebx = byte offset in RALLYE.EXE where this track's palette begins.
    movzx ax,byte [manifesto_cmd+1]
    mov cl,3
    mul cl                                  ; ax = byte offset of the given palette index relative to the palette's offset.
    add ebx,eax                             ; ebx = the byte offset of the given color in the track's palette.
    mov dx,bx                               ; dx = lowest bits of the offset.
    shr ebx,16
    mov cx,bx                               ; cx = highest bits of the offset.

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; write the modified color into ~~LIKKO.EXE.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov bx,[fh_sb_valikko_exe]
    mov ax,4200h                            ; set to move file position, offset from the beginning.
    int 21h                                 ; move file position.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).
    mov dx,file_buffer
    mov cx,3                                ; each palette entry is 3 bytes.
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
