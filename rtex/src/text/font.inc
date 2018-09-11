; A hardcoded character set. Each line defines the bitmap of one character. The bitmap is given as pixel offsets, such that
; each word defines th number of pixels that must be added to the current video memory index to arrive at the location of
; the character's next pixel (this assumes that the screen is 320 pixels wide). A word value of FFFFh indicates that there
; are no more pixels in the character - each bitmap is padded out to 16 words to make the character set quicker to index into.
;
; Note that the character set does not include lowercase letters.
;
; Note also that no character in the set should have all of its pixels filled. That would result in an infinite loop in rendering.
;
font	dw 0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh ; [ ].
        dw 00001h,00140h,00140h,00280h,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh ; [!].
        dw 00000h,00002h,0013eh,00002h,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh ; ["].
        dw 00001h,00001h,00140h,0013fh,00280h,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh ; [#].
        dw 00001h,00001h,00140h,0013fh,00280h,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh ; [$].
        dw 00000h,00002h,00140h,0013fh,0013fh,00140h,00002h,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh ; [%].
        dw 00001h,00001h,00140h,0013fh,00280h,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh ; [&].
        dw 00001h,00140h,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh ; ['].
        dw 00002h,0013fh,00140h,00140h,00141h,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh ; [(].
        dw 00000h,00141h,00140h,00140h,0013fh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh ; [)].
        dw 00001h,0013fh,00001h,00001h,0013fh,0013fh,00002h,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh ; [*].
        dw 00141h,0013fh,00001h,00001h,0013fh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh ; [+].
        dw 00501h,0013fh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh ; [,].
        dw 00280h,00001h,00001h,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh ; [-].
        dw 00501h,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh ; [.].
        dw 00142h,0013fh,0013fh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh ; [/].
        dw 00001h,0013fh,00002h,0013eh,00002h,0013eh,00002h,0013fh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh ; [0].
        dw 00001h,0013fh,00001h,00140h,00140h,00140h,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh ; [1].
        dw 00001h,0013fh,00002h,00140h,0013fh,0013fh,00001h,00001h,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh ; [2].
        dw 00000h,00001h,00141h,0013fh,00141h,0013eh,00001h,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh ; [3].
        dw 00002h,0013fh,00001h,0013eh,00002h,0013eh,00001h,00001h,00140h,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh ; [4].
        dw 00000h,00001h,00001h,0013eh,00140h,00001h,00141h,0013eh,00001h,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh ; [5].
        dw 00001h,00001h,0013eh,00140h,00001h,0013fh,00002h,0013fh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh ; [6].
        dw 00000h,00001h,00001h,00140h,0013fh,00140h,00140h,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh ; [7].
        dw 00001h,0013fh,00002h,0013fh,0013fh,00002h,0013fh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh ; [8].
        dw 00001h,0013fh,00002h,0013fh,00001h,00140h,0013fh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh ; [9].
        dw 00141h,00280h,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh ; [:].
        dw 00281h,00280h,0013fh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh ; [;].
        dw 00002h,0013fh,0013fh,00141h,00141h,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh ; [<].
        dw 00140h,00001h,00001h,0027eh,00001h,00001h,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh ; [=].
        dw 00000h,00141h,00141h,0013fh,0013fh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh ; [>].
        dw 00001h,0013fh,00002h,00140h,0013fh,00281h,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh ; [?].
        dw 00140h,00001h,00001h,0013eh,00002h,00140h,0013eh,00001h,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh ; [@].
        dw 00001h,0013fh,00002h,0013eh,00001h,00001h,0013eh,00002h,0013eh,00002h,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh ; [A].
        dw 00000h,00001h,0013fh,00002h,0013eh,00001h,0013fh,00002h,0013eh,00001h,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh ; [B].
        dw 00001h,00001h,0013eh,00140h,00140h,00141h,00001h,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh ; [C].
        dw 00000h,00001h,0013fh,00002h,0013eh,00002h,0013eh,00002h,0013eh,00001h,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh ; [D].
        dw 00000h,00001h,00001h,0013eh,00140h,00001h,0013fh,00140h,00001h,00001h,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh ; [E].
        dw 00000h,00001h,00001h,0013eh,00140h,00001h,00001h,0013eh,00140h,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh ; [F].
        dw 00000h,00001h,00001h,0013eh,00140h,00002h,0013eh,00002h,0013eh,00001h,00001h,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh ; [G].
        dw 00000h,00002h,0013eh,00002h,0013eh,00001h,00001h,0013eh,00002h,0013eh,00002h,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh ; [H].
        dw 00000h,00001h,00001h,0013fh,00140h,00140h,0013fh,00001h,00001h,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh ; [I].
        dw 00002h,00140h,00140h,0013eh,00002h,0013eh,00001h,00001h,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh ; [J].
        dw 00000h,00002h,0013eh,00002h,0013eh,00001h,0013fh,00002h,0013eh,00002h,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh ; [K].
        dw 00000h,00140h,00140h,00140h,00140h,00001h,00001h,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh ; [L].
        dw 00000h,00002h,0013eh,00001h,00001h,0013eh,00002h,0013eh,00002h,0013eh,00002h,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh ; [M].
        dw 00000h,00002h,0013eh,00001h,00001h,0013eh,00001h,00001h,0013eh,00002h,0013eh,00002h,0ffffh,0ffffh,0ffffh,0ffffh ; [N].
        dw 00001h,0013fh,00002h,0013eh,00002h,0013eh,00002h,0013fh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh ; [O].
        dw 00000h,00001h,0013fh,00002h,0013eh,00001h,0013fh,00140h,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh ; [P].
        dw 00001h,0013fh,00002h,0013eh,00002h,0013eh,00002h,0013fh,00141h,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh ; [Q].
        dw 00000h,00001h,0013fh,00002h,0013eh,00001h,0013fh,00002h,0013eh,00002h,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh ; [R].
        dw 00001h,00001h,0013eh,00141h,00141h,0013eh,00001h,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh ; [S].
        dw 00000h,00001h,00001h,0013fh,00140h,00140h,00140h,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh ; [T].
        dw 00000h,00002h,0013eh,00002h,0013eh,00002h,0013eh,00002h,0013eh,00001h,00001h,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh ; [U].
        dw 00000h,00002h,0013eh,00002h,0013eh,00002h,0013eh,00002h,0013fh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh ; [V].
        dw 00000h,00002h,0013eh,00002h,0013eh,00001h,00001h,0013eh,00001h,00001h,0013fh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh ; [W].
        dw 00000h,00002h,0013eh,00002h,0013fh,0013fh,00002h,0013eh,00002h,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh ; [X].
        dw 00000h,00002h,0013eh,00002h,0013fh,00140h,00140h,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh ; [Y].
        dw 00000h,00001h,00001h,00140h,0013fh,0013fh,00140h,00001h,00001h,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh,0ffffh ; [Z].
