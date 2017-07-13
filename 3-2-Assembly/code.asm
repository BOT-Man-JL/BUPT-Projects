;====================================================================
; Created by: BOT Man
; Processor: 8086
; Compiler: MASM32
;
; Before starting simulation set Internal Memory Size 
; in the 8086 model properties to 0x10000
;====================================================================

PORT82530   EQU 00H
PORT8253T   EQU 06H

PORT8251D   EQU 08H
PORT8251T   EQU 0AH

PORT8255A   EQU 10H
PORT8255B   EQU 12H
PORT8255C   EQU 14H
PORT8255T   EQU 16H

CODE    SEGMENT PUBLIC 'CODE'
        ASSUME CS:CODE,DS:DATA,SS:STACK

; In: none
; Out: none

DELAY   PROC NEAR
        PUSH CX
        MOV  CX,50H 
DELAYWAIT:
        LOOP DELAYWAIT
        POP  CX
        RET
DELAY   ENDP

; In: CX (position in BUFFER / SCANTABLE)
; Out: none

DISPLAY PROC NEAR
        PUSH AX
        PUSH BX
        PUSH CX
        PUSH DX

        ; Clear bit flag
        MOV  DX, PORT8255B
        IN   AL, DX
        OR   AL, 0F0H
        OUT  DX, AL

        ; Set BUFFER[CX] to Port A
        MOV  BX, OFFSET BUFFER
        ADD  BX, CX
        MOV  AL, [BX]
        MOV  DX, PORT8255A
        OUT  DX, AL

        ; Get bit flag mask to SCANTABLE[CX]
        MOV  BX, OFFSET SCANTABLE
        ADD  BX, CX
        MOV  CL, [BX]

        ; Set bit flag with mask
        MOV  DX, PORT8255B
        IN   AL, DX
        AND  AL, 0FH
        OR   AL, CL
        OUT  DX, AL
        CALL DELAY

        POP  DX
        POP  CX
        POP  BX
        POP  AX
        RET
DISPLAY ENDP

; Main Function

START:
        MOV  AX, DATA
        MOV  DS, AX
        
        ; Init 8253
        MOV  DX, PORT8253T
        MOV  AL, 16H              ; 计数器 0 低八位, 方式 3, 二进制计数 (00010110)
        OUT  DX, AL

        MOV  DX, PORT82530
        MOV  AL, 52               ; 时钟 1M, 波特率 1200baud, 波特率因子16 (52)
        OUT  DX, AL

        ; Init 8251
        MOV  DX, PORT8251T

        XOR  AL, AL               ; Init 3 times
        OUT  DX, AL
        OUT  DX, AL
        OUT  DX, AL

        MOV  AL, 40H              ; Reset (01000000)
        OUT  DX, AL
        NOP

        MOV  AL, 4EH              ; 方式命令字: 一个停止位, 无校验, 8 个数据位, 波特率因子 16, 异步 (01001110)
        OUT  DX, AL
        NOP

        MOV  AL, 27H              ; 工作命令字: 允许发送接收 (00100111)
        OUT  DX, AL
        NOP
        
        ; Init 8255
        MOV  DX, PORT8255T
        MOV  AL, 081H             ; 方式控制字: C 口低四位输入, B 口输出 (方式 0), A 口输出 (方式 0)
        OUT  DX, AL

        ; Clear READY flag
        MOV  READY, 00H

GETKEY:
        MOV  CX, 00H              ; Line Number
CHECKLINE:
        MOV  BX, OFFSET SCANTABLE
        ADD  BX, CX
        MOV  AL, [BX]
        MOV  DX, PORT8255C
        OUT  DX, AL

        ; Check again
        IN   AL, DX               ; 判断是否有键盘按下
        MOV  AH, AL
        CALL DELAY                ; 延迟一段时间，避过抖动区间
        IN   AL, DX               ; 延时后再次读入
        CMP  AL, AH               ; 比较两次读入的值
        JNZ  CHECKLINE            ; 不相等说明为抖动，重新检测

        AND  AL, 0FH
NEXT1:
        CMP  AL, 0EH              ; column 3
        JNZ  NEXT2
        MOV  BX, 03H
        JMP  PARSE
NEXT2:
        CMP  AL, 0DH              ; column 2
        JNZ  NEXT3
        MOV  BX, 02H
        JMP  PARSE
NEXT3:
        CMP  AL, 0BH              ; column 1
        JNZ  NEXT4
        MOV  BX, 01H
        JMP  PARSE
NEXT4:
        CMP  AL, 07H              ; column 0
        JNZ  NEXTROUND
        MOV  BX, 00H
        JMP  PARSE

NEXTROUND:
        INC  CX                   ; next line
        CMP  CX, 04H              ; has checked 4 lines?
        JNZ  CHECKLINE
        JMP  KEYDONE

PARSE:
        MOV  AL, CL               ; CL = row; AL = row
        MOV  DL, 04H
        MUL  DL
        ADD  BL, AL               ; BL = col; BL = row * 4 + col
        
        ; Wait for pop up
        MOV  DX, PORT8255C
        IN   AL, DX
        MOV  AH, AL
NOTUP:
        CALL DELAY
        IN   AL, DX
        CMP  AL, AH
        JZ   NOTUP

        ; Set READY flag
        MOV  READY, 01H

KEYDONE:
		; Update Display
        MOV  CX, 3
        CALL DISPLAY
        MOV  CX, 2
        CALL DISPLAY
        MOV  CX, 1
        CALL DISPLAY
        MOV  CX, 0
        CALL DISPLAY

        MOV  DX, PORT8251T        ; Check 8251 TxRDY
        IN   AL, DX
        AND  AL, 01H
        JNZ  TXRDY

CHECKRXRDY:    
        MOV  DX, PORT8251T
        IN   AL, DX
        AND  AL, 02H              ; Check 8251 RxRDY
        JNZ  RXRDY
        JMP  GETKEY

TXRDY:
        CMP  READY, 01H
        JE   KEYRDY
        JMP  CHECKRXRDY

KEYRDY:
        MOV  DX, PORT8251D
        MOV  AL, BL
        OUT  DX, AL
        CALL DELAY
        MOV  READY, 00H
        JMP  CHECKRXRDY

RXRDY:
        ; Get Input Index to AX
        XOR  AX, AX
        MOV  DX, PORT8251D
        IN   AL, DX

        ; Set Input Seg Code to INPUT
        MOV  BX, OFFSET SEGTABLE
        ADD  BX, AX
        MOV  AL, [BX]
        MOV  INPUT, AL

        ; Move INPUT to BUFFER
        MOV  BX, OFFSET BUFFER
        MOV  DL, [BX + 2]
        MOV  [BX + 3], DL
        MOV  DL, [BX + 1]
        MOV  [BX + 2], DL
        MOV  DL, [BX + 0]
        MOV  [BX + 1], DL
        MOV  DL, INPUT
        MOV  [BX + 0], DL

        JMP  GETKEY
CODE    ENDS
    
;====================================================================

DATA    SEGMENT
        SEGTABLE   DB 3FH, 06H, 5BH, 4FH, 66H, 6DH, 7DH, 07H
                   DB 7FH, 6FH, 77H, 7CH, 39H, 5EH, 79H, 71H
        SCANTABLE  DB 70H, 0B0H, 0D0H, 0E0H

        INPUT      DB 00H
        BUFFER     DB 00H, 00H, 00H, 00H
        READY      DB 00H
DATA    ENDS

;====================================================================

STACK   SEGMENT STACK 'STACK'
        DB 100 DUP(?)
STACK   ENDS

        END START