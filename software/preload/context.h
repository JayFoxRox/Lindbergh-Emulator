
#include <ucontext.h>

typedef union {
  ucontext_t ctx; //TODO: Rename to ucontext
  struct {
    unsigned long int uc_flags;
    ucontext_t* uc_link;
    stack_t uc_stack;
    
    union {
      unsigned int registers[NGREG];
      struct {
        uint32_t gs;
        uint32_t fs;
        uint32_t es;
        uint32_t ds;
        union {
          uint32_t edi;
          //TODO: more?
        };
        union {
          uint32_t esi;
          //TODO: more?
        };
        union {
          uint32_t ebp;
          //TODO: more?
        };
        union {
          uint32_t esp;
          //FIXME: More!
        };
        union {
          uint32_t ebx;
          uint16_t bx;
          struct { uint8_t bl, bh; };
        };
        union {
          uint32_t edx;
          uint16_t dx;
          struct { uint8_t dl, dh; };
        };
        union {
          uint32_t ecx;
          uint16_t cx;
          struct { uint8_t cl, ch; };
        };
        union {
          uint32_t eax;
          uint16_t ax;
          struct { uint8_t al, ah; };
        };
        unsigned int trapno;
        unsigned int err;
        union {
          uint32_t eip;
          //FIXME: More?
        };
        uint32_t cs;
        union {
          uint32_t eflags;
          uint16_t flags;
          struct {
            uint16_t cFlag:1;
            uint16_t pad_cFlag:1;
            uint16_t pFlag:1;
            uint16_t pad_pFlag:1;
            uint16_t aFlag:1;
            uint16_t pad_aFlag:1;
            uint16_t zFlag:1;
            uint16_t sFlag:1;
            uint16_t tFlag:1;
            uint16_t tPad:1;
            uint16_t dFlag:1;
            uint16_t oFlag:1;
            uint16_t pad_oFlag:4;
          };
        };
        unsigned int uesp;
        uint32_t ss;
      };
    };
    /* Note that fpregs is a pointer.  */
    /*fpregset_t*/ struct _libc_fpstate* fpregs;
    unsigned long __reserved1 [8];
  
    __sigset_t uc_sigmask;
    struct _libc_fpstate __fpregs_mem;
  };  
} hostContext_t;


