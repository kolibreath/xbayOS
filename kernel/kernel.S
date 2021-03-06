;保护模式下
[bits 32]

;定义两个宏，用来作为中断处理程序
%define ERROR_CODE nop
%define ZERO push 0

extern put_str;
extern idt_table;

section .data
intr_str db "interrupt occur!", 0xa, 0 
global intr_entry_table

intr_entry_table:

%macro VECTOR 2
section .text
intr%1entry:
  %2
  ;保存寄存器上下文
  push ds
  push es
  push fs 
  push gs
  pushad

  mov al, 0x20
  ;设置中断控制器8259A的中断结束方式和优先级模式
  ;OCW2写入到主片的0x20端口和从片的0xA0端口
  out 0xa0, al
  out 0x20, al

  push %1                   ;作为函数调用参数
  call [idt_table + %1 * 4] ;调用中断处理函数
  jmp intr_exit

section .data
  ;存储各个中断入口程序的地址，形成数组
  dd intr%1entry
%endmacro

section .text
extern syscall_table
global syscall_handler
;中断0x80会调用该函数作为中断入口程序
syscall_handler:
  push 0
  push ds
  push es
  push fs 
  push gs
  pushad
  push 0x80

  ;在栈中压入通过寄存器传来的系统调用参数
  push edx
  push ecx
  push ebx
  
  call [syscall_table + 4 * eax]
  add esp, 12

  ;子功能处理函数返回值在eax中，为了防止被覆盖，存入将会写入eax的栈的位置
  mov [esp + 8*4], eax
  jmp intr_exit
  
global intr_exit
intr_exit:
  ;恢复寄存器上下文
  add esp, 4  ;跳过栈中中断号
  popad
  pop gs
  pop fs
  pop es
  pop ds
  add esp, 4
  iretd



;如果中断号没有error code，那么就push 0到栈里面
VECTOR 0x00, ZERO
VECTOR 0x01, ZERO
VECTOR 0x02, ZERO
VECTOR 0x03, ZERO 
VECTOR 0x04, ZERO
VECTOR 0x05, ZERO
VECTOR 0x06, ZERO
VECTOR 0x07, ZERO 
VECTOR 0x08, ERROR_CODE
VECTOR 0x09, ZERO
VECTOR 0x0a, ERROR_CODE
VECTOR 0x0b, ERROR_CODE 
VECTOR 0x0c, ZERO
VECTOR 0x0d, ERROR_CODE
VECTOR 0x0e, ERROR_CODE
VECTOR 0x0f, ZERO 
VECTOR 0x10, ZERO
VECTOR 0x11, ERROR_CODE
VECTOR 0x12, ZERO
VECTOR 0x13, ZERO 
VECTOR 0x14, ZERO
VECTOR 0x15, ZERO
VECTOR 0x16, ZERO
VECTOR 0x17, ZERO 
VECTOR 0x18, ERROR_CODE
VECTOR 0x19, ZERO
VECTOR 0x1a, ERROR_CODE
VECTOR 0x1b, ERROR_CODE 
VECTOR 0x1c, ZERO
VECTOR 0x1d, ERROR_CODE
VECTOR 0x1e, ERROR_CODE
VECTOR 0x1f, ZERO 
VECTOR 0x20, ZERO ;时钟中断
VECTOR 0x21, ZERO ;键盘中断
VECTOR 0x22, ZERO
VECTOR 0x23, ZERO
VECTOR 0x24, ZERO
VECTOR 0x25, ZERO
VECTOR 0x26, ZERO
VECTOR 0x27, ZERO
VECTOR 0x28, ZERO
VECTOR 0x29, ZERO
VECTOR 0x2a, ZERO
VECTOR 0x2b, ZERO
VECTOR 0x2c, ZERO
VECTOR 0x2d, ZERO
VECTOR 0x2e, ZERO
VECTOR 0x2f, ZERO
