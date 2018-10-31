#include "interrupt.h"

#include "global.h"
#include "io.h"
#include "stdint.h"
#include "print.h"

#define IDT_DESC_CNT 0x21 //支持的中断数

//8259A主片(master)的控制端口和数据端口
#define PIC_M_CTRL 0x20
#define PIC_M_DATA 0x21

//8259A从片(slave)的控制端口和数据端口
#define PIC_S_CTRL 0xa0
#define PIC_S_DATA 0xa1

//中断门描述符
struct gate_desc {
    uint16_t func_offset_low_word;      //中断处理程序在目标段内的偏移量的低16位
    uint16_t selector;                  //中断处理程序目标代码段描述符选择子
    uint8_t  dcount;                    //未使用的bits，置为0即可
    uint8_t  attribute;                 //中断门描述符属性，包括P位、DPL、S位和type
    uint16_t func_offset_high_word;     //中断处理程序在目标段内的偏移量的高16位
};

static struct gate_desc idt[IDT_DESC_CNT];
extern intr_handler intr_entry_table[IDT_DESC_CNT];

intr_handler idt_table[IDT_DESC_CNT];
char* intr_name[IDT_DESC_CNT]; 

static void make_idt_desc(struct gate_desc* gdesc_ptr, uint8_t attr, intr_handler function);
static void pic_init();
static void default_intr_hanlder(uint8_t intr_vec_num);
static void exception_init();

//创建中断门描述符
//参数1：指向新创建的中断门描述符的指针
//参数2：属性
//参数3：中断处理程序
static void make_idt_desc(struct gate_desc* gdesc_ptr, uint8_t attr, intr_handler function) {
    gdesc_ptr->func_offset_low_word = (uint32_t)function & 0x0000ffff;
    gdesc_ptr->selector = SELECTOR_K_CODE;
    gdesc_ptr->dcount = 0;
    gdesc_ptr->attribute = attr;
    gdesc_ptr->func_offset_high_word = ((uint32_t)function >> 16) & 0xffff0000;
}

//初始化中断描述符表
static void idt_desc_init() {
    for (int i = 0; i < IDT_DESC_CNT; ++i) {
        make_idt_desc(&idt[i], IDT_DESC_ATTR_DPL0, intr_entry_table[i]);
    }
}

static void pic_init() {
    //初始化主片
    outb(PIC_M_CTRL, 0x11);
    outb(PIC_M_DATA, 0x20);
    outb(PIC_M_DATA, 0x04);
    outb(PIC_M_DATA, 0x01);

    //初始化从片
    outb(PIC_S_CTRL, 0x11);
    outb(PIC_S_DATA, 0x28);
    outb(PIC_S_DATA, 0x02);
    outb(PIC_S_DATA, 0x01);

    //打开主片上的IR0(时钟产生的中断)
    outb(PIC_M_DATA, 0xfe);
    outb(PIC_S_DATA, 0xff);
}

//如果设置中断处理函数，就会执行这个默认中断处理函数
static void default_intr_hanlder(uint8_t intr_vec_num) {
    //伪中断，不需要处理
    if (intr_vec_num == 0x27 || intr_vec_num == 0x2f) {
        return;
    }
    put_str("interrupt: [");
    put_int(intr_vec_num);
    put_str("]");
    put_str(intr_name[intr_vec_num]);
    put_str("\n");
}

//初始化中断处理函数表和异常名字
static void exception_init() {
    for (int i = 0; i < IDT_DESC_CNT; ++i) {
        idt_table[i] = default_intr_hanlder;
        intr_name[i] = "unkown"; 
    }

    intr_name[0]  = "#DE Divide Error";
    intr_name[1]  = "#DB Debug";
    intr_name[2]  = "/   NMI Interrupt";
    intr_name[3]  = "#BP Breakpoint";
    intr_name[4]  = "#OF Overflow";
    intr_name[5]  = "#BR BOUND Range Exceeded";
    intr_name[6]  = "#UD Invalid Opcode(UnDefined Opcode)";
    intr_name[7]  = "#NM Device Not Available(No Math Coprocessor)";
    intr_name[8]  = "#DF Double Fault";
    intr_name[9]  = "#MF CoProcessor Segment Overrun(reserved)";
    intr_name[10] = "#TS Invalid TSS";
    intr_name[11] = "#NP Segment Not Present";
    intr_name[12] = "#SS Stack Segment Fault";
    intr_name[13] = "#GP General Protection";
    intr_name[14] = "#PF Page Fault";
    intr_name[15] = "    Reserved";
    intr_name[16] = "#MF Floating-Point Error(MathFault)";
    intr_name[17] = "#AC Alignment Check";
    intr_name[18] = "#MC Machine Check";
    intr_name[19] = "#XM SIMD Floating-Point Exception";
}

void idt_init() {
    idt_desc_init();    //初始化中断描述符表
    exception_init();   //初始化中断处理函数和名字
    pic_init();         //初始化程序中断控制器
    
    //加载idt
    uint64_t idt_operand = ((sizeof(idt) - 1) | ((uint64_t)(uint32_t)idt << 16));
    asm volatile("lidt %0" : : "m" (idt_operand));
}