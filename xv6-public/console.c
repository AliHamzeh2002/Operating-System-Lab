// Console input and output.
// Input is from the keyboard or serial port.
// Output is written to the screen and serial port.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "traps.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"

static void consputc(int);

static int panicked = 0;

static struct {
  struct spinlock lock;
  int locking;
} cons;

static void
printint(int xx, int base, int sign)
{
  static char digits[] = "0123456789abcdef";
  char buf[16];
  int i;
  uint x;

  if(sign && (sign = xx < 0))
    x = -xx;
  else
    x = xx;

  i = 0;
  do{
    buf[i++] = digits[x % base];
  }while((x /= base) != 0);

  if(sign)
    buf[i++] = '-';

  while(--i >= 0)
    consputc(buf[i]);
}
//PAGEBREAK: 50

// Print to the console. only understands %d, %x, %p, %s.
void
cprintf(char *fmt, ...)
{
  int i, c, locking;
  uint *argp;
  char *s;

  locking = cons.locking;
  if(locking)
    acquire(&cons.lock);

  if (fmt == 0)
    panic("null fmt");

  argp = (uint*)(void*)(&fmt + 1);
  for(i = 0; (c = fmt[i] & 0xff) != 0; i++){
    if(c != '%'){
      consputc(c);
      continue;
    }
    c = fmt[++i] & 0xff;
    if(c == 0)
      break;
    switch(c){
    case 'd':
      printint(*argp++, 10, 1);
      break;
    case 'x':
    case 'p':
      printint(*argp++, 16, 0);
      break;
    case 's':
      if((s = (char*)*argp++) == 0)
        s = "(null)";
      for(; *s; s++)
        consputc(*s);
      break;
    case '%':
      consputc('%');
      break;
    default:
      // Print unknown % sequence to draw attention.
      consputc('%');
      consputc(c);
      break;
    }
  }

  if(locking)
    release(&cons.lock);
}

void
panic(char *s)
{
  int i;
  uint pcs[10];

  cli();
  cons.locking = 0;
  // use lapiccpunum so that we can call panic from mycpu()
  cprintf("lapicid %d: panic: ", lapicid());
  cprintf(s);
  cprintf("\n");
  getcallerpcs(&s, pcs);
  for(i=0; i<10; i++)
    cprintf(" %p", pcs[i]);
  panicked = 1; // freeze other CPU
  for(;;)
    ;
}

//PAGEBREAK: 50
#define BACKSPACE 0x100
#define CLEAR 0x101
#define MOVE_LEFT 0x102
#define MOVE_RIGHT 0x103
#define GO_END_OF_LINE 0x104
#define ARROW_UP 0xE2
#define ARROW_DOWN 0xE3
#define CRTPORT 0x3d4
static ushort *crt = (ushort*)P2V(0xb8000);  // CGA memory

#define INPUT_BUF 128
struct {
  char buf[INPUT_BUF];
  uint r;  // Read index
  uint w;  // Write index
  uint e;  // Edit index
  uint end;
} input;

static void
cgaputc(int c)
{
  int pos;

  // Cursor position: col + 80*row.
  outb(CRTPORT, 14);
  pos = inb(CRTPORT+1) << 8;
  outb(CRTPORT, 15);
  pos |= inb(CRTPORT+1);

  if(c == '\n')
    pos += 80 - pos%80;
  else if(c == BACKSPACE){
    if(pos > 0) --pos;
    for (int i = pos ; i < pos + (input.end - input.e); i++){
      crt[i] = crt[i + 1];
    }
    crt[pos + input.end - input.e] = ' ' | 0x0700;

  }
  else if(c == CLEAR){
    for (int i = 0; i < 25 * 80; i++){
      crt[i] = ' ' | 0x0700;
    }
    pos = 0;
  } 
  else if(c == MOVE_LEFT){
    if (pos > 0)
      pos --;
  }
  else if(c == MOVE_RIGHT){
    if (pos < 80 * 25)
      pos ++;
  }

  else if(c == GO_END_OF_LINE){
    pos += input.end - input.e;
  }
  else{
    if (input.end != input.e){
      for (int i = pos + (input.end - input.e); i > pos; i--)
        crt[i] = crt[i - 1];
    }
    
    crt[pos++] = (c&0xff) | 0x0700;  // black on white
  }

  if(pos < 0 || pos > 25*80)
    panic("pos under/overflow");

  if((pos/80) >= 24){  // Scroll up.
    memmove(crt, crt+80, sizeof(crt[0])*23*80);
    pos -= 80;
    memset(crt+pos, 0, sizeof(crt[0])*(24*80 - pos));
  }

  outb(CRTPORT, 14);
  outb(CRTPORT+1, pos>>8);
  outb(CRTPORT, 15);
  outb(CRTPORT+1, pos);
  if (input.end == input.e && c != CLEAR)
    crt[pos] = ' ' | 0x0700;
}

void
consputc(int c)
{
  if(panicked){
    cli();
    for(;;)
      ;
  }

  if(c == BACKSPACE){
    uartputc('\b'); uartputc(' '); uartputc('\b');
  } else
    uartputc(c);
  cgaputc(c);
}



#define C(x)  ((x)-'@')  // Control-x

#define MAX_HISTORY 10

struct {
  char recent_cmds[MAX_HISTORY][INPUT_BUF];
  ushort first;
  ushort cur;
  ushort size;
} cmd_history;

void
push_command_to_history(){
  ushort last_idx = (cmd_history.first + cmd_history.size) % MAX_HISTORY;
  if (cmd_history.size == MAX_HISTORY){
    cmd_history.first ++;
  }
  for (int i = input.w; i < input.end; i++){
    cmd_history.recent_cmds[last_idx][i - input.w] = input.buf[i % INPUT_BUF];
  }
  //cmd_history.recent_cmds[last_idx][input.end - input.w - 1] = '\n';
  if (cmd_history.size < MAX_HISTORY){
    cmd_history.size ++;
  }
  cmd_history.cur = (cmd_history.size + cmd_history.first) % MAX_HISTORY;
}

void
consoleintr(int (*getc)(void))
{
  int c, doprocdump = 0;

  acquire(&cons.lock);
  while((c = getc()) >= 0){
    switch(c){
    case C('P'):  // Process listing.
      // procdump() locks cons.lock indirectly; invoke later
      doprocdump = 1;
      break;
    case C('U'):  // Kill line.
      consputc(GO_END_OF_LINE);
      input.e = input.end;
      while(input.e != input.w &&
            input.buf[(input.e-1) % INPUT_BUF] != '\n'){
        input.e--;
        input.end--;
        consputc(BACKSPACE);
      }
      break;
    case C('H'): case '\x7f':  // Backspace
      if(input.e != input.w){
        input.e--;
        for (int i = input.e ; i < input.end; i++){
          input.buf[i % INPUT_BUF] = input.buf[(i + 1) % INPUT_BUF];
        }
        input.end--;
        consputc(BACKSPACE);
        
      }
      

      break;
    case C('L'):
      input.w = input.e;
      input.end = input.e;
      consputc(CLEAR);
      consputc('$');
      consputc(' ');
      break;
    case C('B'):
      if (input.e > input.w){
        input.e--;
        consputc(MOVE_LEFT);
      }
      break;
    case C('F'):
      if (input.e < input.end){
        input.e++;
      consputc(MOVE_RIGHT);
      }
      break;
    case ARROW_UP:
      if (cmd_history.cur == cmd_history.first){
        break;
      }
      consputc(GO_END_OF_LINE);
      input.e = input.end;
      while(input.e != input.w &&
            input.buf[(input.e-1) % INPUT_BUF] != '\n'){
        input.e--;
        input.end--;
        consputc(BACKSPACE);
      }
      if ((cmd_history.cur - 1) % MAX_HISTORY != (cmd_history.first - 1) % MAX_HISTORY){
        cmd_history.cur --;
        cmd_history.cur %= MAX_HISTORY;
      }
      char* last_cmd = cmd_history.recent_cmds[cmd_history.cur];
      for (int i = 0; i < INPUT_BUF; i++){
        if (last_cmd[i] == '\n' || last_cmd[i] == C('D'))
          break;
        input.buf[input.e++ % INPUT_BUF] = last_cmd[i];
        input.end++;
        consputc(last_cmd[i]);
      }
      
      break;

    case ARROW_DOWN:
      if (cmd_history.cur == cmd_history.first + cmd_history.size){
        break;
      }
      consputc(GO_END_OF_LINE);
      input.e = input.end;
      while(input.e != input.w &&
            input.buf[(input.e-1) % INPUT_BUF] != '\n'){
        input.e--;
        input.end--;
        consputc(BACKSPACE);
      }
      cmd_history.cur ++;
      cmd_history.cur %= MAX_HISTORY;
      if ((cmd_history.cur) % MAX_HISTORY != (cmd_history.first + cmd_history.size) % MAX_HISTORY){
        last_cmd = cmd_history.recent_cmds[cmd_history.cur];
        for (int i = 0; i < INPUT_BUF; i++){
          if (last_cmd[i] == '\n' || last_cmd[i] == C('D'))
            break;
          input.buf[input.e++ % INPUT_BUF] = last_cmd[i];
          input.end++;
          consputc(last_cmd[i]);
        }
      }
      
      
      break;
    

    default:
      if(c != 0 && input.end-input.r < INPUT_BUF){
        if (input.end != input.e && c!='\n'){
          for (int i = input.end; i > input.e; i--){
              input.buf[i % INPUT_BUF] = input.buf[(i - 1) % INPUT_BUF];
            }
        }
        c = (c == '\r') ? '\n' : c;
        if(c == '\n' || c == C('D') || input.end == input.r+INPUT_BUF){
          input.buf[input.end++ % INPUT_BUF] = c;
          push_command_to_history();
          input.e = input.end;
          input.w = input.e;
          wakeup(&input.r);
        }
        else{
          input.buf[input.e++ % INPUT_BUF] = c;
          input.end++;
        }
        consputc(c);   

      }
      break;
    }
  }
  release(&cons.lock);
  if(doprocdump) {
    procdump();  // now call procdump() wo. cons.lock held
  }
}

int
consoleread(struct inode *ip, char *dst, int n)
{
  uint target;
  int c;

  iunlock(ip);
  target = n;
  acquire(&cons.lock);
  while(n > 0){
    while(input.r == input.w){
      if(myproc()->killed){
        release(&cons.lock);
        ilock(ip);
        return -1;
      }
      sleep(&input.r, &cons.lock);
    }
    c = input.buf[input.r++ % INPUT_BUF];
    if(c == C('D')){  // EOF
      if(n < target){
        // Save ^D for next time, to make sure
        // caller gets a 0-byte result.
        input.r--;
      }
      break;
    }
    *dst++ = c;
    --n;
    if(c == '\n')
      break;
  }
  release(&cons.lock);
  ilock(ip);

  return target - n;
}

int
consolewrite(struct inode *ip, char *buf, int n)
{
  int i;

  iunlock(ip);
  acquire(&cons.lock);
  for(i = 0; i < n; i++)
    consputc(buf[i] & 0xff);
  release(&cons.lock);
  ilock(ip);

  return n;
}

void
consoleinit(void)
{
  initlock(&cons.lock, "console");

  devsw[CONSOLE].write = consolewrite;
  devsw[CONSOLE].read = consoleread;
  cons.locking = 1;

  ioapicenable(IRQ_KBD, 0);
}

