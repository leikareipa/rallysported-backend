;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;
;;; Subroutines for applying the manifesto to the game's (sandboxed) executable.
;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Applies the patches dictated in the manifesto to the game's executable.
;;;
;;; EXPECTS:
;;;     - ds to point to the base data segment.
;;;     - ds:fh_sb_rallye_exe should give the file handle of the sandboxed RALLYE.EXE executable.
;;;     - ds:fh_sb_valikko_exe should give the file handle of the sandboxed VALIKKO.EXE executable.
;;;     - fn_project_file to be the file name of the manifesto to apply.
;;; DESTROYS:
;;;     (- unknown)
;;; RETURNS:
;;;     (- unknown)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Apply_Manifesto:
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; open the manifesto file.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    xor cx,cx
    mov dx,fn_project_file                  ; file name.
    mov ah,3dh                              ; set to open.
    mov al,0                                ; read only.
    int 21h                                 ; do it.
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; read the manifesto file's data into memory, for easier access.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov dx,file_buffer
    mov bx,ax
    mov ah,3fh
    mov cx,FILE_BUFFER_SIZE                 ; read the maximum number of bytes.
    int 21h
    jc .exit_fail                           ; error-checking (the cf flag will be set by int 21h if there was an error).

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; extract and apply the manifesto's commands.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    xor si,si                               ; our index into the manifesto file's data.
    .do_next_command:
        ; for debugging.
        ;pusha
        ;mov dl,10   ; newline.
        ;mov ah,2
        ;int 21h
        ;popa

        call Get_Next_Manifesto_Command         ; get a string of values that represent the next command and its parameters.

        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        ; verify the command's integrity.
        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        cmp byte [manifesto_cmd_len],0          ; error flag.
        je .exit_fail

        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        ; apply the command.
        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        mov al,[manifesto_cmd]

        .test_0:
        cmp al,0
        jne .test_2
        call Command_0_REQUIRE
        cmp al,0
        je .exit_fail
        jmp .do_next_command

        .test_2:
        cmp al,2
        jne .test_4
        call Command_2_SET_NUM_OBJECTS
        cmp al,0
        je .exit_fail
        jmp .do_next_command

        .test_4:
        cmp al,4
        jne .test_5
        call Command_4_CHANGE_OBJECT_TYPE
        cmp al,0
        je .exit_fail
        jmp .do_next_command

        .test_5:
        cmp al,5
        jne .test_10
        call Command_5_MOVE_OBJECT
        cmp al,0
        je .exit_fail
        jmp .do_next_command

        .test_10:
        cmp al,10
        jne .test_99
        call Command_10_CHANGE_PALETTE_COLOR
        cmp al,0
        je .exit_fail
        jmp .do_next_command

        .test_99:
        cmp al,99                               ; the successful terminating command.
        je .exit_success

        .unknown:
        mov [err_manifesto_additional],err_manifesto_unknown_cmd
        jmp .exit_fail

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
;;; Parses the next line from the manifesto file (up to the '\n'), and extracts the command and parameters from it.
;;;
;;; EXPECTS:
;;;     - ds:fh_sb_rallye_exe should give the file handle of the sandboxed RALLYE.EXE executable.
;;;     - ds:fh_sb_valikko_exe should give the file handle of the sandboxed VALIKKO.EXE executable.
;;;     - es:file_buffer should contain the manifesto's data in memory.
;;;     - si to point to our current location in the manifesto data.
;;; DESTROYS:
;;;     (- unknown)
;;; RETURNS:
;;;     - the command and its parameters in ds:manifesto_cmd.
;;;     - the number of parameters (counting the command itself) in ds:manifesto_cmd_len.
;;; NOTES:
;;;     - if ds:manifesto_cmd_len is 0 on return, the caller may interpret it as an error flag from this routine.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Get_Next_Manifesto_Command:
    mov byte [manifesto_cmd_len],0              ; reset the temporary data holders.

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; extract the command and parameters from the line.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    xor bx,bx                               ; how many values we've extracted.
    xor di,di                               ; our index in the temporary string we use to convert strings into values.
    xor cx,cx                               ; used to signal whether we encountered a carriage return or newline, after which we exit.
    .loop_for_values:
        mov al,[file_buffer+si]                 ; get the next character from the line.
        add si,1

        mov byte [tmp_str+di],al                ; add the character to the temporary string we'll convert into a decimal value.

        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        ; take a space or tab to mean the end of the value's string, so we're ready to convert the string to decimal.
        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        cmp al,' '
        je .convert

        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        ; take a newline or carriage return to mean the end of the line, so we're ready to convert the string and then exit.
        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        .test_newline_dos:
        cmp al,13                               ; for dos, we expected newlines to start with a carriage return '\r'.
        jne .test_newline_unix
        add si,1                                ; eat away the '\n' that we assume follows '\r'.
        mov cx,1
        jmp .convert

        .test_newline_unix:
        cmp al,10                               ; for unix, we expect newlines to just have '\n' without the carriage return.
        jne .line_didnt_end
        mov cx,1
        jmp .convert

        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        ; if we didn't encounter a newline, we need to keep parsing.
        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        .line_didnt_end:
        add di,1
        cmp di,3                                ; we can only convert up to 3 decimals.
        jg .error

        jmp .loop_for_values

        .convert:
        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        ; convert the value from a string to a decimal.
        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        push cx                                 ; preserve cx since it's our flag for end of line.
        push si                                 ; preserve our index in the manifesto string.
        mov byte [tmp_str+di],0                 ; null-terminate the string.
        call ASCIZ_To_Decimal
        pop si
        pop cx

        ; for debugging, print out the value we just extracted.
        ;pusha
        ;add dl,'0'
        ;mov ah,2
        ;int 21h
        ;mov dl,' '
        ;mov ah,2
        ;int 21h
        ;popa

        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        ; store the extracted value in the temporary buffer.
        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        mov byte [manifesto_cmd+bx],dl
        add bl,1
        mov [manifesto_cmd_len],bl

        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        ; make sure we don't try to write more parameters than we have reserved memory space for.
        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        cmp bl,(MAX_MANIFESTO_PARAMS + 1)
        jg .error

        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        ; cx will be set to 1 if we found the newline. if it's set, we know we can exit.
        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        cmp cx,1
        je .done
        xor cx,cx

        xor di,di                               ; move back to the start of the string we use to convert to decimal.

        jmp .loop_for_values

    .error:
    mov byte [manifesto_cmd_len],0          ; signal that something went wrong.
    jmp .exit

    .done_cr:
    add si,1                                ; eat away the '\n' that we assume follows '\r'.

    .done:

    .exit:
    ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Converts an ASCIZ string into a decimal. Valid range: 0-255.
;;;
;;; EXPECTS:
;;;     (- unknown)
;;;     - di to give the string's length.
;;; DESTROYS:
;;;     - ax, cx, edx, si, di.
;;; RETURNS:
;;;     - dx = the value. if there was an error, dx = 0.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ASCIZ_To_Decimal:
    xor edx,edx
    xor eax,eax

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; convert.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov cx,di
    xor di,di
    mov si,3
    sub si,cx                               ; our index into the lookup table.
    .conv:
            mov al,[tmp_str+di]
            sub al,'0'                      ; convert to a decimal.
            mov ah,[decimal_lut+si]
            mul ah
            add edx,eax
            add si,1
            add di,1
            loop .conv

    .done:
    cmp edx,256                             ; we only support values up to 255.
    jl .exit
    mov dl,0
    jmp .exit

    .exit:
    ret
