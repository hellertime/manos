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

#include <errno.h>
#include <stdint.h>

#include <manos.h>

#if defined PLATFORM_NICE

/* stub constants for compiling on other hardware */
static uint32_t __FAKE_REG = 0;
#define PORTD_PCR0 __FAKE_REG
#define PORTE_PCR26 __FAKE_REG
#define PORT_PCR_MUX(x) 0
#define PORT_PCR_PE_MASK 0
#define PORT_PCR_PS_MASK 0
#define SIM_SCGC5 __FAKE_REG
#define SIM_SCGC5_PORTD_MASK 0
#define SIM_SCGC5_PORTE_MASK 0
#define PTE_BASE_PTR 0
#define PTD_BASE_PTR 0
#define GPIO_PDIR_REG(x) __FAKE_REG
#else
#include <arch/k70/derivative.h>
#endif

/* for now put this here */
#if defined PLATFORM_K70CW
void nanosleep(unsigned long int nanos);
__asm(
		"    .global nanosleep\n"
		"nanosleep:\n"
		"    adds r0,r0,#-1\n"
		"    bne  nanosleep\n"
		"    bx   lr\n"
	);
#else
void nanosleep(unsigned long int nanos) {
    while(nanos-->0)
        ;
    return;
}
#endif
/*
 * Define names for the bits in the PDIR register. Which correspond to the selection bit for each switch.
 * The mapping comes from sheet #7 of the TWR-K70F120M schematic
 */

#define BIT_0  (1 << 0)
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
  USED(gpio);;
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

static StaticNS swpbSNS[] = {
    /* root */
    { ".", MKSTATICNS_CRUMB(STATICNS_SENTINEL, 0, CRUMB_ISDIR), 0, 0555, 0 }

    /* one level */
,   { "1",    MKSTATICNS_CRUMB(0, FidOne,    CRUMB_ISFILE), 0, 0444, 0 }
,   { "2",    MKSTATICNS_CRUMB(0, FidTwo,    CRUMB_ISFILE), 0, 0444, 0 }
,   { "1raw", MKSTATICNS_CRUMB(0, FidOneRaw, CRUMB_ISFILE), 0, 0444, 0 }
,   { "2raw", MKSTATICNS_CRUMB(0, FidTwoRaw, CRUMB_ISFILE), 0, 0444, 0 }

    /* sentinel */
,   { "", MKSTATICNS_SENTINEL_CRUMB, 0, 0, 0 }
};

static Portal* attachSwpb(char *path) {
  Portal* p = attachDev(DEV_DEVSWPB, path);
  p->crumb = swpbSNS[0].crumb;
  return p;
}

static NodeInfo* swpbNodeInfoFn(const Portal* p, WalkDirection d, NodeInfo* ni) {
   return getNodeInfoStaticNS(p, swpbSNS, d, ni);
}

static WalkTrail* walkSwpb(Portal* p, char** path, unsigned n) {
    return genericWalk((const Portal*)p, (const char**)path, n, swpbNodeInfoFn);
}

static Portal* openSwpb(Portal *p, Caps caps) {
  return openDev(p, caps);
}

static void closeSwpb(Portal *p) {
  UNUSED(p);
}

static int getInfoSwpb(const Portal *p, NodeInfo* ni) {
  return getNodeInfoStaticNS(p, swpbSNS, WalkSelf, ni) == NULL ? -1 : 0;
}

static ptrdiff_t readSwpb(Portal *p, void *buf, size_t size, Offset offset) {
  if (size == 0) return 0;
  
  if (p->crumb.flags & CRUMB_ISDIR) {
    return readStaticNS(p, swpbSNS, buf, size, offset);
  }

  if (p->offset) return 0;
  
  SwpbFidEnt fid = STATICNS_CRUMB_SELF_IDX(p->crumb); 
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
    p->offset++;
    return 1;
  }
  default:
    errno = ENODEV;
    return -1;
  }
}

static ptrdiff_t writeSwpb(Portal *p, void *buf, size_t size, Offset offset) {
  UNUSED(p);
  UNUSED(buf);
  UNUSED(size);
  UNUSED(offset);
  errno = EPERM;
  return -1;
}

Dev devSwpb = {
    .id       = DEV_DEVSWPB
,   .name     = "swpb"
,   .power    = powerDev
,   .init     = initSwpb
,   .reset    = resetDev
,   .shutdown = shutdownDev
,   .attach   = attachSwpb
,   .walk     = walkSwpb
,   .create   = createDev
,   .open     = openSwpb
,   .close    = closeSwpb
,   .remove   = removeDev
,   .getInfo  = getInfoSwpb
,   .setInfo  = setInfoDev
,   .read     = readSwpb
,   .write    = writeSwpb
};
