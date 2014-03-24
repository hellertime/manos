/*
 * devswpb - manos software push button device
 * 
 * Typically mounted on /dev/swpb
 * 
 * Represents a single level filesystem:
 * 
 * ./1
 * ./2
 * 
 * Reading a byte from ./<n> returns the current button state of the sw button with some delay to allow for flap
 * Reading a bytes from ./<n>raw returns the actual state right from hardware
 */

#include <stdint.h>

#include <libc.h>

#include <manos/dev.h>
#include <manos/types.h>

#if defined PLATFORM_NICE

/* stub constants for compiling on other hardware */
static uint32_t __FAKE_REG = 0;
#define PORTD_PCR0 __FAKE_REG
#define PORTE_PCR26 __FAKE_REG
#define PORT_PCR_MUX(x) 0
#define PORT_PCR_PE_MASK 0
#define PORT_PCR_PS_MASK 0
#define SIM_SGCG5 __FAKE_REG
#define SIM_SGCG5_PORTD_MASK 0
#define SIM_SGCG5_PORTE_MASK 0
#define PTE_BASE_PTR 0
#define PTD_BASE_PTR 0
#define GPIO_PDIR_REG(x) __FAKE_REG
#else
#include <derivative.h>
#endif

/*
 * Define names for the bits in the PDIR register. Which correspond to the selection bit for each switch.
 * The mapping comes from sheet #7 of the TWR-K70F120M schematic
 */

#define BIT_0  0
#define BIT_26 (1 << 26)

#define BUTTON_ONE_BIT BIT_0
#define BUTTON_TWO_BIT BIT_26

#define BUTTON_ONE_PCR PORTD_PCR0
#define BUTTON_TWO_PCR PORTE_PCR26

/*
 * Give the buttons nice names
 */

typedef enum {
  Button1,
  Button2
} Button;

typedef enum {
 ButtonDown,
 ButtonUp,
} ButtonState;

/*
 * Sets the buttons ports to be GPIO and out flowing.
 */
static void makeGPIOIn(Button which) {
  int gpio = 1;
  volatile uint32_t * const buttonPcr[] = { &BUTTON_ONE_PCR, &BUTTON_TWO_PCR };
  *(buttonPcr[which]) = PORT_PCR_MUX(gpio) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK; /* enable internal pull up/down resistor as pull up */
}

/*
 * Initialize the button clocks. And configure the buttons.
 */
static void initButtons(void) {
  static const Button buttons[] = { Button1, Button2 };
  
  SIM_SCGC5 |= (SIM_SCGC5_PORTD_MASK | SIM_SCGC5_PORTE_MASK); /* clock port D & E */
  
  for (unsigned i = 0; i < COUNT_OF(buttons); i++) {
    makeGPIOIn(buttons[i]);
  }
}

/*
 * Returns the present button up/down state unbuffered.
 * The button logic is set low when the button is down.
 */
static ButtonState getState(Button which) {
  volatile uint32_t * const buttonPdirReg[] = { &GPIO_PDIR_REG(PTD_BASE_PTR), &GPIO_PDIR_REG(PTE_BASE_PTR) };
  uint32_t buttonBit[] = { BUTTON_ONE_BIT, BUTTON_TWO_BIT };
  ButtonState buttonState[] = { ButtonUp, ButtonDown };
  return buttonState[!(*(buttonPdirReg[which]) & buttonBit[which])];
}

typedef enum {
  FidDot = 0,
  FidOne,
  FidTwo,
  FidOneRaw,
  FidTwoRaw
} SwpbFidEnt;

static void initSwpb(void) {
  initButtons();
}

static struct DirEnt swpbDirEnt[] = {
  { "1",    { FidOne,    0 }, 0, 0444 },
  { "1raw", { FidOneRaw, 0 }, 0, 0444 },
  { "2",    { FidTwo,    0 }, 0, 0444 },
  { "2raw", { FidTwoRaw, 0 }, 0, 0444 }
};

static struct Portal* attachSwpb(char *path) {
  for (unsigned i = 0; i < COUNT_OF(swpbDirEnt); i++) {
    if (streq(swpbDirEnt[i].path, path)) {
      struct Portal *p = attachDev(DEV_DEVSWPB, path);
      p->fid = swpbDirEnt[i].fid;
      return p;
    }
  }
  
  return NULL;
}

static struct Portal* openSwpb(struct Portal *p, OMode mode) {
  return openDev(p, mode);
}

static void closeSwpb(struct Portal *p) {
  UNUSED(p);
}

static Err getInfoSwpb(struct Portal *p, struct DevInfo *info) {
  return getInfoDev(p, swpbDirEnt, COUNT_OF(swpbDirEnt), info);
}

static int32_t readSwpb(struct Portal *p, void *buf, uint32_t size, Offset offset, Err *err) {
  *err = E_OK;
  if (size == 0 || offset != 0) {
    return 0;
  }
  
  if (p->fid.type & FID_ISDIR) {
    return 0;
  }
  
  SwpbFidEnt fid = (SwpbFidEnt)p->fid.tag;
  
  switch(fid) {
  case FidOne:
  case FidTwo:
    /* Non-raw read. Delay read slightly to let buttons settle. */
	nanosleep(151); /* ~ 5us */
	/* adjust fid to proceed to raw processing code path */
	fid += FidTwo;
  case FidOneRaw:
  case FidTwoRaw: {
	/* get state returns a '1' when the button is up */
	ButtonState state = getState((Button)fid - FidOneRaw);
    *(char*)buf = '0' + state;
    return 1;
  }
  default:
    return 0;
  }
}

static int32_t writeSwpb(struct Portal *p, void *buf, uint32_t size, Offset offset, Err *err) {
  UNUSED(p);
  UNUSED(buf);
  UNUSED(size);
  UNUSED(offset);
  *err = E_PERM;
  return -1;
}

struct Dev swpbDev = {
  .id = DEV_DEVSWPB,
  .name = "swpb",
  .init = initSwpb,
  .reset = resetDev,
  .shutdown = shutdownDev,
  .attach = attachSwpb,
  .create = createDev,
  .open = openSwpb,
  .close = closeSwpb,
  .remove = removeDev,
  .getInfo = getInfoSwpb,
  .setInfo = setInfoDev,
  .read = readSwpb,
  .write = writeSwpb
};
