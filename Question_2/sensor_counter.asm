; ========================================
; Assembly Program: Sensor Log Line Counter
; File: sensor_counter.asm
; Description: Counts total lines in sensor_log.txt
; ========================================

section .data
    filename db 'sensor_log.txt', 0      ; Null-terminated filename
    output_msg db 'Total sensor readings: ', 0
    output_msg_len equ $ - output_msg
    newline db 10                         ; Newline character
    
section .bss
    file_buffer resb 4096                 ; Buffer to store file contents (4KB)
    counter resb 16                       ; Buffer to store counter as string
    fd resd 1                             ; File descriptor storage

section .text
    global _start

_start:
    ; ===========================
    ; STEP 1: Open the file
    ; System call: open(filename, O_RDONLY)
    ; ===========================
    mov rax, 2                  ; syscall number for open
    mov rdi, filename           ; pointer to filename
    mov rsi, 0                  ; O_RDONLY flag (read-only mode)
    mov rdx, 0                  ; mode (not needed for reading)
    syscall                     ; invoke system call
    
    ; Check if file opened successfully (fd >= 0)
    cmp rax, 0
    jl exit_error               ; if rax < 0, file open failed
    mov [fd], rax               ; store file descriptor
    
    ; ===========================
    ; STEP 2: Read file into buffer
    ; System call: read(fd, buffer, count)
    ; ===========================
    mov rax, 0                  ; syscall number for read
    mov rdi, [fd]               ; file descriptor
    mov rsi, file_buffer        ; pointer to buffer
    mov rdx, 4096               ; maximum bytes to read
    syscall                     ; invoke system call
    
    ; rax now contains number of bytes read
    mov r15, rax                ; save bytes read count in r15
    
    ; ===========================
    ; STEP 3: Close the file
    ; System call: close(fd)
    ; ===========================
    mov rax, 3                  ; syscall number for close
    mov rdi, [fd]               ; file descriptor
    syscall                     ; invoke system call
    
    ; ===========================
    ; STEP 4: Count lines by traversing buffer
    ; Logic: Count newline characters ('\n')
    ; Each newline represents end of a line
    ; Add 1 for last line if file doesn't end with newline
    ; ===========================
    xor rcx, rcx                ; rcx = 0, line counter
    xor rbx, rbx                ; rbx = 0, buffer index
    xor r14, r14                ; r14 = 0, flag for last character
    
count_loop:
    cmp rbx, r15                ; compare index with bytes read
    jge count_done              ; if index >= bytes_read, exit loop
    
    ; Load current character
    movzx rax, byte [file_buffer + rbx]
    
    ; Check if character is newline (ASCII 10)
    cmp al, 10
    jne not_newline
    
    ; Found a newline - increment counter
    inc rcx
    xor r14, r14                ; reset last-char flag
    jmp next_char
    
not_newline:
    ; Track that we saw a non-newline character
    mov r14, 1
    
next_char:
    inc rbx                     ; move to next character
    jmp count_loop
    
count_done:
    ; If last character was not newline, count the last line
    cmp r14, 1
    jne print_result
    inc rcx                     ; add 1 for last line without newline
    
    ; Special case: empty file
    cmp r15, 0
    jne print_result
    xor rcx, rcx                ; if file is empty, count = 0
    
print_result:
    ; ===========================
    ; STEP 5: Convert counter to string
    ; ===========================
    mov rax, rcx                ; move counter to rax
    mov rdi, counter            ; pointer to string buffer
    add rdi, 15                 ; start from end of buffer
    mov byte [rdi], 0           ; null terminator
    dec rdi
    
    mov rbx, 10                 ; divisor for base-10 conversion
    
convert_loop:
    xor rdx, rdx                ; clear rdx for division
    div rbx                     ; divide rax by 10
    add dl, '0'                 ; convert remainder to ASCII
    mov [rdi], dl               ; store digit
    dec rdi                     ; move backwards in buffer
    test rax, rax               ; check if quotient is 0
    jnz convert_loop            ; continue if not zero
    
    inc rdi                     ; adjust pointer to start of number
    mov r12, rdi                ; save pointer to start of digits
    
    ; ===========================
    ; STEP 6: Print output message
    ; ===========================
    ; Print "Total sensor readings: "
    mov rax, 1                  ; syscall number for write
    mov rdi, 1                  ; file descriptor (stdout)
    mov rsi, output_msg         ; pointer to message
    mov rdx, output_msg_len     ; message length
    syscall
    
    ; Print the counter value
    mov rax, 1                  ; syscall number for write
    mov rdi, 1                  ; file descriptor (stdout)
    mov rsi, r12                ; pointer to counter string
    ; Calculate string length
    mov rdx, counter
    add rdx, 15                 ; pointer to null terminator
    sub rdx, r12                ; length excludes null terminator
    syscall
    
    ; Print newline
    mov rax, 1
    mov rdi, 1
    mov rsi, newline
    mov rdx, 1
    syscall
    
    ; ===========================
    ; STEP 7: Exit program successfully
    ; ===========================
    mov rax, 60                 ; syscall number for exit
    xor rdi, rdi                ; exit code 0 (success)
    syscall

exit_error:
    ; Exit with error code 1
    mov rax, 60
    mov rdi, 1
    syscall