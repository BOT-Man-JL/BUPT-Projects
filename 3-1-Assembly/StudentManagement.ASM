
; Student Management System
; By BOT Man, 2016

STACK	SEGMENT STACK
		dw 100 dup (?)  ; Buffering
STACK	ENDS

DATA	SEGMENT
		; Switch Pointer
		Table	dw Case0, Case1, Case2, Case3, Case4

		; Student Data
		SName	db 10 dup (9 dup (0), '$')
		Class	db 10 dup (9 dup (0), '$')
		StuId	db 10 dup (9 dup (0), '$')
		GradeS	db 10 dup (9 dup (0), '$')
		GradeC	dw 10 dup (0)
		Index	db 10 dup (0)

		; Count Student
		cStud	db 0
				db 0 ; even
		cGrad	dw 0

		; Count Grade Range
		cGrade0	dw 0
		cGrade6	dw 0
		cGrade7	dw 0
		cGrade8	dw 0
		cGrade9	dw 0

		; Prompt Messages

		pNewLn	db 0dh, 0ah, '$'

		pUsage	db 'Usage:', 0dh, 0ah
				db 'Input 0 to Add Student', 0dh, 0ah
				db 'Input 1 to Order by Grade', 0dh, 0ah
				db 'Input 2 to Get Average Grade', 0dh, 0ah
				db 'Input 3 to Group by Grade', 0dh, 0ah
				db 'Input 4 to Exit :-)', 0dh, 0ah
				db 'Command :$'
		pErr	db 'Invalid Input', 0dh, 0ah, '$'

		pSName	db 'Name:', '$'
		pClass	db 'Class:', '$'
		pStuId	db 'Id:  ', '$'
		pGrade	db 'Grade:', '$'

		pAvg	 db 'Average Grade:', '$'

		pGrade0 db 'Grade   0-60:', '$'
		pGrade6 db 'Grade  60-70:', '$'
		pGrade7 db 'Grade  70-80:', '$'
		pGrade8 db 'Grade  80-90:', '$'
		pGrade9 db 'Grade 90-100:', '$'

		; Input Buffer
		InputB	db 9
		InputC	db ?
		Buffer	db 9 dup (?), 'haha', '$'
DATA	ENDS

CODE	SEGMENT
		Assume CS:CODE, DS:DATA

Main	PROC

		; Init Scheme
		mov ax, DATA
		mov ds, ax

tagMain:
		; Prompt Usage
		mov dx, offset pUsage
		call PrintDX

		; Input Case
		call InputAL
		mov dx, offset pNewLn
		call PrintDX

		mov ah, 0
		sub al, '0'
		cmp al, 4
		jle tagValidInput
		jmp Case5

tagValidInput:
		mov bx, ax
		shl bx, 1  ; Note that: Table[i] is WORD !
		jmp Table[bx]

Case0:	; Add Student

		; Calc Offset to BX
		mov ah, 0
		mov al, cStud
		mov bl, 10
		mul bl
		mov bx, ax

		; Get Name
		mov dx, offset pSName
		call PrintDX
		call InputS
		mov di, offset SName
		call CopyStr
		mov dx, offset pNewLn
		call PrintDX

		; Get Class
		mov dx, offset pClass
		call PrintDX
		call InputS
		mov di, offset Class
		call CopyStr
		mov dx, offset pNewLn
		call PrintDX

		; Get Id
		mov dx, offset pStuId
		call PrintDX
		call InputS
		mov di, offset StuId
		call CopyStr
		mov dx, offset pNewLn
		call PrintDX

		; Get Grade
		mov dx, offset pGrade
		call PrintDX
		call InputS
		mov dx, offset pNewLn
		call PrintDX
		
		mov di, offset GradeS
		call CopyStr
		
		call ParseG
		mov bx, 0
		mov bl, cStud
		shl bx, 1
		mov word ptr GradeC [bx], ax

		; Add Grade to Total
		add cGrad, ax

		; Find Place to Insert
		
		mov bx, 0
		mov dx, 0
		mov ch, 0
		mov cl, cStud

		cmp cl, 0
		jz tagSetIndex

tagCountStudent:
		cmp word ptr GradeC [bx], ax
		jl tagLessThanCur
		shr bx, 1
		inc byte ptr Index[bx]
		shl bx, 1
		jmp tagDoneC
tagLessThanCur:
		inc dx
tagDoneC:
		add bx, 2
		loop tagCountStudent

tagSetIndex:
		mov bx, 0
		mov bl, cStud
		mov byte ptr Index [bx], dl

		; Increase Student Count
		inc cStud

		; Count to Group
		cmp ax, 600
		jl tagGrade0
		cmp ax, 700
		jl tagGrade6
		cmp ax, 800
		jl tagGrade7
		cmp ax, 900
		jl tagGrade8
		jmp tagGrade9

tagGrade0:
		inc cGrade0
		jmp tagMain
tagGrade6:
		inc cGrade6
		jmp tagMain
tagGrade7:
		inc cGrade7
		jmp tagMain
tagGrade8:
		inc cGrade8
		jmp tagMain
tagGrade9:
		inc cGrade9
		jmp tagMain

Case1:	; Order By Id

		mov ax, 0
		mov ch, 0
		mov cl, cStud

		cmp cx, 0
		jz tagNoStudentToPrint

tagPrintStudent:

		mov bx, 0
tagFindNext:
		cmp al, byte ptr Index[bx]
		jz tagPrintThis
		inc bx
		jmp tagFindNext

tagPrintThis:
		mov di, 0
		push ax
		mov ax, 10
		mul bx
		add di, ax
		pop ax

		mov dx, offset pSName
		call PrintDX
		mov dx, offset SName
		add dx, di
		call PrintDX
		mov dl, ','
		call PrintDL

		mov dx, offset pClass
		call PrintDX
		mov dx, offset Class
		add dx, di
		call PrintDX
		mov dx, offset pNewLn
		call PrintDX

		mov dx, offset pStuId
		call PrintDX
		mov dx, offset StuId
		add dx, di
		call PrintDX
		mov dl, ','
		call PrintDL

		mov dx, offset pGrade
		call PrintDX
		mov dx, offset GradeS
		add dx, di
		call PrintDX
		mov dx, offset pNewLn
		call PrintDX

		mov dx, offset pNewLn
		call PrintDX

		inc ax
		loop tagPrintStudent

tagNoStudentToPrint:
		jmp tagMain

Case2:	; Ger Avg Grade
		
		cmp cStud, 0
		jnz tagNotZero

		mov ax, 0
		call PrintAX

		mov dx, offset pNewLn
		call PrintDX

		jmp tagMain

tagNotZero:
		mov ax, cGrad

		mov dx, 0
		mov bx, 0
		mov bl, cStud
		div bx

		mov dx, 0
		mov bx, 10
		div bx

		call PrintAX

		push dx
		mov dl, '.'
		call PrintDL
		pop dx

		mov ax, dx
		call PrintAX

		mov dx, offset pNewLn
		call PrintDX

		jmp tagMain

Case3:	; Group By Grade

		mov dx, offset pGrade0
		call PrintDX
		mov ax, cGrade0
		call PrintAX
		mov dx, offset pNewLn
		call PrintDX

		mov dx, offset pGrade6
		call PrintDX
		mov ax, cGrade6
		call PrintAX
		mov dx, offset pNewLn
		call PrintDX
		
		mov dx, offset pGrade7
		call PrintDX
		mov ax, cGrade7
		call PrintAX
		mov dx, offset pNewLn
		call PrintDX
		
		mov dx, offset pGrade8
		call PrintDX
		mov ax, cGrade8
		call PrintAX
		mov dx, offset pNewLn
		call PrintDX
		
		mov dx, offset pGrade9
		call PrintDX
		mov ax, cGrade9
		call PrintAX
		mov dx, offset pNewLn
		call PrintDX

		jmp tagMain

Case4:	; End
		mov ax, 4c00h
		int 21h

Case5:	; Invalid Input
		mov dx, offset pErr
		call PrintDX
		jmp tagMain

Main    ENDP

; Sub-Routines

; Print Str of DX
PrintDX	PROC
		push ax

		mov ah, 09h
		int 21h

		pop ax
		ret
PrintDX	ENDP

; Print Char of DL
PrintDL	PROC
		push ax

		mov ah, 02h
		int 21h

		pop ax
		ret
PrintDL	ENDP

; Print Number of AX
PrintAX	PROC
		push bx
		push cx
		push dx

		mov cx, 0

		cmp ax, 0
		jnz tagPush

		mov dl, '0'
		call PrintDL
		
		jmp tagEndPrintAX
tagPush:
		cmp ax, 0
		jz tagPrint

		mov bx, 10
		mov dx, 0
		div bx

		add dl, '0'
		push dx

		inc cx
		jmp tagPush

tagPrint:
		pop dx
		call PrintDL

		loop tagPrint

tagEndPrintAX:
		pop dx
		pop cx
		pop bx
		ret
PrintAX	ENDP

; Input Str to Buffer, got InputC
InputS	PROC
		push ax
		push bx
		push dx

		mov ah, 0ah
		mov dx, offset InputB
		int 21h

		mov bh, 0
		mov bl, InputC
		mov byte ptr Buffer[bx], 0

		pop dx
		pop bx
		pop ax
		ret
InputS	ENDP

; Input Char to AL
InputAL	PROC
		mov ah, 01h
		int 21h
		ret
InputAL	ENDP

; Copy Buffer to [BX + DI]
CopyStr	PROC
		push ax
		push cx
		push si

		mov ch, 0
		mov cl, InputC
		mov si, offset Buffer
tagCopyLoop:
		mov al, byte ptr [si]
		mov byte ptr [di + bx], al
		inc si
		inc di
		loop tagCopyLoop

		pop si
		pop cx
		pop ax
		ret
CopyStr	ENDP

; Parse Buffer into AX
ParseG	PROC
		push bx
		push cx
		push dx

		mov ax, 0
		mov ch, 0
		mov cl, InputC
		mov bx, offset Buffer

tagCheck:
		cmp byte ptr [bx], '.'
		jz tagDot

		mov dh, 0
		mov dl, byte ptr [bx]
		sub dl, '0'
		add ax, dx

		mov dx, 10     ; ax *= 10
		mul dx

		inc bx
		loop tagCheck
tagDot:
		cmp cx, 0      ; is End
		jz tagEnd

		dec cx
		jz tagEnd
		
		inc bx         ; move forward
		mov dh, 0
		mov dl, byte ptr [bx]
		sub dl, '0'
		add ax, dx
		
tagEnd:
		pop dx
		pop cx
		pop bx
		ret
ParseG	ENDP

CODE	ENDS

		END Main