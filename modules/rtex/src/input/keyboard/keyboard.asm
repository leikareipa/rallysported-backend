;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;
;;; Subroutines for handling keyboard input.
;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; constants.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
KEY_NO_KEY          = 0
KEY_S               = 1
KEY_R               = 2
KEY_PLUS            = 3
KEY_MINUS           = 4
KEY_ZERO            = 5

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Gets the currently pressed keyboard key.
;;;
;;; EXPECTS:
;;;     - ds to be a data segment holding the key state variables.
;;; DESTROYS:
;;;     - ax.
;;; RETURNS:
;;;     (- unknown)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Poll_Keyboard_Status:
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; find whether there is a new key in the input buffer.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov ah,1
    int 16h
    jnz .test_key_lock                      ; if no keys were pressed.
    mov [key_lock],0                        ; release the key lock.
    jmp .clear_buffer

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; if the key lock is enabled, no new keys are allowed to be pressed.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    .test_key_lock:
    cmp [key_lock],1
    je .no_known_key

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; fetch the key from the input buffer.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov ah,0
    int 16h

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; suss out which key was pressed.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    .test_s:
    cmp ah,1fh
    jne .test_plus
    mov [key_pressed],KEY_S
    jmp .clear_buffer

    .test_plus:
    cmp ah,4eh
    jne .test_minus
    mov [key_pressed],KEY_PLUS
    jmp .clear_buffer

    .test_minus:
    cmp ah,4ah
    jne .test_zero_numrow
    mov [key_pressed],KEY_MINUS
    jmp .clear_buffer

    .test_zero_numrow:
    cmp ah,0bh
    jne .test_zero_numpad
    mov [key_pressed],KEY_ZERO
    jmp .clear_buffer

    .test_zero_numpad:
    cmp ah,52h
    jne .no_known_key
    mov [key_pressed],KEY_ZERO
    jmp .clear_buffer

    .no_known_key:
    mov [key_lock],0
    mov [key_pressed],KEY_NO_KEY
    jmp .clear_buffer

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; clear the keyboard buffer.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    .clear_buffer:
    ;mov ah,0ch
    ;mov al,0
    ;int 21h

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; engage the key lock if the user pressed a known key.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    cmp [key_pressed],KEY_NO_KEY
    je .exit
    mov [key_lock],1

    .exit:
    ret
