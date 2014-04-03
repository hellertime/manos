/*
 * devled - manos LED devices
 *
 * Typically mounted on /dev/led
 *
 * Represents a single level file system:
 *
 *    ./orange
 *    ./yellow
 *    ./green
 *    ./blue
 *
 *    Reading a byte from ./<color> returns in a '1' or a '0' indicating the current state of the LED
 *    Writing a byte ('1' or '0') to ./<color> updates the state of the LED.
 */

#include <errno.h>
#include <stdint.h>

#include <manos.h>

#ifdef PLATFORM_NICE

/* stub platform macros for nice development */

static uint32_t __FAKE_REG = 0;
#define PORTA_PCR10 __FAKE_REG
#define PORTA_PCR11 __FAKE_REG
#define PORTA_PCR28 __FAKE_REG
#define PORTA_PCR29 __FAKE_REG
#define PORT_PCR_MUX(x) 0
#define GPIO_PDDR_REG(x) __FAKE_REG
#define GPIO_PDOR_REG(x) __FAKE_REG
#define GPIO_PSOR_REG(x) __FAKE_REG
#define GPIO_PCOR_REG(x) __FAKE_REG
#define SIM_SCGC5 __FAKE_REG
#define SIM_SCGC5_PORTA_MASK 0
#define PTA_BASE_PTR 0

#else
#include <derivative.h>
#endif

/*
 * Define names for the bits in the PSOR and PCOR which correspond to the LED
 * drive lines. This mapping can be found on sheet #9 of the TWR-K70F120M
 * schematic.
 */

#define BIT_10 (1 << 10)
#define BIT_11 (1 << 11)
#define BIT_28 (1 << 28)
#define BIT_29 (1 << 29)

/*
 * Both PSOR and PCOR are 32-bit wide registers with the property that
 * when written to the word is xor'd on the register so only set bits
 * are modified. Define four words one for each bit pattern.
 */

static const uint32_t ORANGE_BIT = BIT_11;
static const uint32_t YELLOW_BIT = BIT_28;
static const uint32_t GREEN_BIT  = BIT_29;
static const uint32_t BLUE_BIT   = BIT_10;

/*
 * The PCR (Pin-control register) is a 32 word wide array. The memory map to the hardware
 * is defined in arch/MK70F12.h, which also provides some convenience names to the hardware
 * addresses. Assign even more convenient names here.
 */

#define ORANGE_LED_PCR PORTA_PCR11
#define YELLOW_LED_PCR PORTA_PCR28
#define GREEN_LED_PCR  PORTA_PCR29
#define BLUE_LED_PCR   PORTA_PCR10

typedef enum {
  LedOff,
  LedOn
} LedState;

typedef enum {
  LedOrange,
  LedYellow,
  LedGreen,
  LedBlue
} LedColor;

/*
 * Both PSOR and PCOR are 32-bit wide registers with the property that
 * when written to the word is xor'd on the register so only set bits
 * are modified. Define four words one for each bit pattern.
 *
 * Map the LedColor enum to the associated bit pattern
 */
static uint32_t ledColorBits[] = {
  BIT_11, /* orange */
  BIT_28, /* yellow */
  BIT_29, /* green */
  BIT_10, /* blue */
};

/*
 * makeGPIOOut :: LedColor -> ()
 *
 * Setup the appropriate Pin-control register.
 * The LEDs are treated as GPIO output devices.
 */
static void makeGPIOOut(LedColor which) {
  int gpio = 1;
  USED(gpio);
  volatile uint32_t * const ledColorPCR[] = { &ORANGE_LED_PCR, &YELLOW_LED_PCR, &GREEN_LED_PCR, &BLUE_LED_PCR };
  *(ledColorPCR[which]) = PORT_PCR_MUX(gpio);
  GPIO_PDDR_REG(PTA_BASE_PTR) |= ledColorBits[which];
}

/*
 * setLed :: LedColor -> LedState -> ()
 *
 * Set the repective LED device to the given state
 */
static void setLed(LedColor which, LedState state) {
  volatile uint32_t * const regPxOR[] = { &GPIO_PSOR_REG(PTA_BASE_PTR), &GPIO_PCOR_REG(PTA_BASE_PTR) };
  *(regPxOR[state]) = ledColorBits[which];
}

/*
 * getLed :: LedColor -> LedState
 *
 * Get the state of the respecive LED device
 */
static LedState getLed(LedColor which) {
  /* TODO: seems to always be 0, is this not wired up ? */
  int rawState = (GPIO_PDOR_REG(PTA_BASE_PTR) & ledColorBits[which]);
  LedState state = (LedState)rawState;
  return state;
}

typedef enum {
  FidDot,
  FidOrange,
  FidYellow,
  FidGreen,
  FidBlue,
} LedFidEnt;

/*
 * initLed :: ()
 *
 * 'init' message handler for the LED device.
 */
static void initLed(void) {
  static const LedColor leds[4] = { LedOrange, LedYellow, LedGreen, LedBlue };

  /* enable PTA clock */
  SIM_SCGC5 |= SIM_SCGC5_PORTA_MASK;

  for (unsigned i = 0; i < COUNT_OF(leds); i++) {
    setLed(leds[i], LedOff);
    makeGPIOOut(leds[i]);
  }
}

static StaticNS ledSNS[] = {
    /* root */
    { "#l", MKSTATICNS_CRUMB(STATICNS_SENTINEL, 0, CRUMB_ISDIR), 0, 0555, 0 }

    /* one level */
,   { "orange", MKSTATICNS_CRUMB(0, FidOrange, CRUMB_ISFILE), 0, 0644, 0 }
,   { "yellow", MKSTATICNS_CRUMB(0, FidYellow, CRUMB_ISFILE), 0, 0644, 0 }
,   { "green",  MKSTATICNS_CRUMB(0, FidGreen, CRUMB_ISFILE),  0, 0644, 0 }
,   { "blue",   MKSTATICNS_CRUMB(0, FidBlue, CRUMB_ISFILE),   0, 0644, 0 }

    /* sentinel */
,   { "", MKSTATICNS_SENTINEL_CRUMB, 0, 0, 0 }
};

/*
 * attachLed :: String -> Portal
 *
 * 'attach' message handler. delgates to the generic device handler.
 */
static Portal* attachLed(char *path) {
  Portal* p = attachDev(DEV_DEVLED, path);
  p->crumb = ledSNS[0].crumb;
  return p;
}

static NodeInfo* ledNodeInfoFn(const Portal* p, WalkDirection d, NodeInfo* ni) {
    return getNodeInfoStaticNS(p, ledSNS, d, ni);
}

/*
 * walkLed :: Portal -> [String] -> Int -> WalkTrail
 *
 * 'walk' the device in the static namespace
 */
static WalkTrail* walkLed(Portal *p, char **path, unsigned n) {
    return genericWalk((const Portal*)p, (const char**)path, n, ledNodeInfoFn);
}

/*
 * openLed :: Portal -> OMode -> Portal
 *
 * 'open' message handler. The opens a Portal to the led device.
 */
static Portal* openLed(Portal *p, Caps caps) {
  return openDev(p, caps);
}

/*
 * closeLed :: Portal -> ()
 *
 * noop
 */
static void closeLed(Portal *p) {
  UNUSED(p);
}

/*
 * getInfoLed :: Portal -> DevInfo -> Err
 * 
 * Fill out a DevInfo struct generically.
 */
static int getInfoLed(const Portal *p, NodeInfo *ni) {
  return getNodeInfoStaticNS(p, ledSNS, WalkSelf, ni) == NULL ? -1 : 0;
}

/*
 * readLed :: Portal -> [Byte] -> Int -> Int
 *
 * Read is always a single byte of data. So no matter the
 * size of the read (except 0) the result of the read is 
 * one byte. If offset is non-zero the read has no value.
 */
static ptrdiff_t readLed(Portal *p, void *buf, size_t size, Offset offset) {
  if (size == 0) return 0;

  if (p->crumb.flags & CRUMB_ISDIR) {
    return readStaticNS(p, ledSNS, buf, size, offset);
  }

  if (p->offset) return 0; /* eof */

  LedFidEnt fid = STATICNS_CRUMB_SELF_IDX(p->crumb);
  switch (fid) {
  case FidOrange:
  case FidYellow:
  case FidGreen:
  case FidBlue: {
    LedColor which = (LedColor)(fid - FidOrange);
    *(char*)buf = '0' + getLed(which);
    p->offset++;
    return 1;
  }
  default:
    errno = ENODEV;
    return -1;
  }
}

/*
 * writeLed :: Portal -> [Bytes] -> Int -> Int
 *
 * A write to the control file of a led device sets its activity.
 * Like the read operation zero sized writes and non-zero offsets 
 * result in a non-write.
 */
static ptrdiff_t writeLed(Portal *p, void *buf, size_t size, Offset offset) {
  UNUSED(offset);
  if (size == 0) {
    return 0;
  }

  char c = *(char*)buf;
  if (!(c == '0' || c == '1')) {
    return 0;
  }

  LedFidEnt fid = STATICNS_CRUMB_SELF_IDX(p->crumb);
  switch (fid) {
  case FidOrange:
  case FidYellow:
  case FidGreen:
  case FidBlue: {
    LedColor which = (LedColor)(fid - FidOrange);
    LedState state = (LedState)(c - '0');
    setLed(which, state);
    return 1;
  }
  default:
    return 0;
  }
}

Dev devLed = {
    .id       = DEV_DEVLED
,   .name     = "led"
,   .init     = initLed
,   .reset    = resetDev
,   .shutdown = shutdownDev
,   .attach   = attachLed
,   .walk     = walkLed
,   .create   = createDev
,   .open     = openLed
,   .close    = closeLed
,   .remove   = removeDev
,   .getInfo  = getInfoLed
,   .setInfo  = setInfoDev
,   .read     = readLed
,   .write    = writeLed
};
