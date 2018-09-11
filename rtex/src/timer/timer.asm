;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;
;;; Subroutines for dealing with timing.
;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

Handle_Timer_Interrupt:
    cli                                     ; protect this code from re-entrancy.

    push ax                                 ; custom interrupts need to reserve all registers they change.

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; measure the number of 10 ms elapsed during the rendering of the current frame.
    ; this value is always reset externally at the start of each frame.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    add [fs:frame_time],TIMER_RES

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; keep track of the number of seconds elapsed since the start of the program.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov al,[fs:timer_ticks]
    inc al
    cmp al,TIMER_TICKS_PER_SEC
    jb .save
    inc [fs:timer_seconds]
    xor al,al
    .save:
    mov [fs:timer_ticks],al

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; see whether the current time is above 55 ms. if it is, we need to call dos's
    ; own timer interrupt, so it can keep system time correct.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov al,[fs:timer_keepup]
    add al,TIMER_RES
    mov [fs:timer_keepup],al
    cmp al,55
    jb .skip_dos_interrupt

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; call dos's own timer interrupt.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov [fs:timer_keepup],0                 ; restart the counter.
    pushf                                   ; simulate an interrupt by pushing the flags,
    call [fs:int_8h_handler]                ; then calling the interrupt. the interrupt will automatically pop the flags.
    jmp .exit

    .skip_dos_interrupt:
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; clear the interrupt. we only need to do this if we didn't call dos's own interrupt, above.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov al, 20h
    out 20h, al

    .exit:
    pop ax

    sti

    iret

Set_Timer_Interrupt_Handler:
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; get the address of dos's own timer interrupt handler.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    push es
    mov ax,3508h
    int 21h
    mov word [int_8h_handler],bx
    mov word [int_8h_handler+2],es
    pop es

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; tell dos to use our custom timer interrupt handler.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    push ds
    mov ax,cs
    mov ds,ax
    mov ax,2508h
    mov dx,Handle_Timer_Interrupt           ; the custom timer interrupt we want dos to call.
    int 21h
    pop ds

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; set a custom clock rate for the timer.
    ; in dosbox: 279c = good for 10 ms, 13ce = good for 5 ms.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov al,36h
    out 43h,al
    mov al,0ceh
    out 40h,al
    mov al,13h
    out 40h,al

    ret

Restore_Timer_Interrupt_Handler:
    cli

    push ds

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; tell dos to go back to using its own timer interrupt handler.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov ax,word [fs:int_8h_handler+2]       ; set to restore the timer segment.
    mov ds,ax
    mov dx,word [fs:int_8h_handler]         ; set to restore the timer address.
    mov ax,2508h
    int 21h

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; set the timer to run at its normal dos rate, i.e. at about 18 hz.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov al,36h
    out 43h,al
    mov al,0
    out 40h,al
    mov al,0
    out 40h,al

    pop ds

    sti

    ret
