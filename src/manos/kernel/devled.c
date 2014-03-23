/*
 * devled - manos LED devices
 *
 * Typically mounted on /dev/led
 *
 * Represents a single level file system:
 *
 *    ./orange
 *    ./orangeCtl
 *    ./yellow
 *    ./yellowCtl
 *    ./green
 *    ./greenCtl
 *    ./blue
 *    ./blueCtl
 *
 *    Reading a byte from ./<color> returns in a '1' or a '0' indicating the current state of the LED
 *    Writing a byte ('1' or '0') to ./<color>Ctl updates the state of the LED.
 */

#include <stdint.h>

#include <libc.h>

#include <manos/types.h>
#include <manos/dev.h>

#ifdef PLATFORM_NICE

/* stub platform macros for nice development */

static uint32_t __FAKE_REG = 0;
#define PORTA_PCR10 __FAKE_REG
#define PORTA_PCR11 __FAKE_REG
#define PORTA_PCR28 __FAKE_REG
#define PORTA_PCR29 __FAKE_REG
#define PORT_PCR_MUX(x) 0
#define GPIO_PDDR_REG(x) __FAKE_REG
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
static uint32_t ledColorBits[4] = {
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
  volatile uint32_t * const ledColorPCR[4] = { &ORANGE_LED_PCR, &YELLOW_LED_PCR, &GREEN_LED_PCR, &BLUE_LED_PCR };
  *(ledColorPCR[which]) = PORT_PCR_MUX(gpio);
  GPIO_PDDR_REG(PTA_BASE_PTR) |= ledColorBits[which];
}

/*
 * setLed :: LedColor -> LedState -> ()
 *
 * Set the repective LED device to the given state
 */
static void setLed(LedColor which, LedState state) {
  volatile uint32_t * const regPxOR[2] = { &GPIO_PSOR_REG(PTA_BASE_PTR), &GPIO_PCOR_REG(PTA_BASE_PTR) };
  *(regPxOR[state]) = ledColorBits[which];
}

/*
 * getLed :: LedColor -> LedState
 *
 * Get the state of the respecive LED device
 */
static LedState getLed(LedColor which) {
  return LedOff;
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

  for (int i = 0; i < COUNT_OF(leds); i++) {
    setLed(leds[i], LedOff);
    makeGPIOOut(leds[i]);
  }
}

static struct DirEnt {
  char *path;
  struct Fid fid;
  uint32_t length;
  Mode mode;
} ledDirEnt[] = {
  { "orange",  { FidOrange, 0 }, 0, 0664 }
, { "yellow",  { FidYellow, 0 }, 0, 0664 }
, { "green",   { FidGreen,  0 }, 0, 0664 }
, { "blue",    { FidBlue,   0 }, 0, 0664 }
};

/*
 * attachLed :: String -> Portal
 *
 * 'attach' message handler. delgates to the generic device handler.
 */
static struct Portal* attachLed(char *path) {
  struct Portal *p = attachDev(DEV_DEVLED, path);

  for (int i = 0; i < COUNT_OF(ledDirEnt); i++) {
    if (streq(ledDirEnt[i].path, path)) {
      p->fid = ledDirEnt[i].fid;
    }
  }
  return p;
}

/*
 * openLed :: Portal -> OMode -> Portal
 *
 * 'open' message handler. The opens a Portal to the led device.
 */
static struct Portal* openLed(struct Portal *p, OMode mode) {
  return openDev(p, mode);
}

/*
 * closeLed :: Portal -> ()
 *
 * noop
 */
static void closeLed(struct Portal *p) {
  UNUSED(p);
}

/*
 * readLed :: Portal -> [Byte] -> Int -> Int
 *
 * Read is always a single byte of data. So no matter the
 * size of the read (except 0) the result of the read is 
 * one byte. If offset is non-zero the read has no value.
 */
static int32_t readLed(struct Portal *p, void *buf, uint32_t size, Offset offset, Err *err) {
  *err = E_OK;
  if (size == 0 || offset != 0) {
    return 0;
  }

  if (p->fid.type & FID_ISDIR) {
    return 0;
  }

  switch ((LedFidEnt)p->fid.tag) {
  case FidOrange:
  case FidYellow:
  case FidGreen:
  case FidBlue:
    *(char*)buf = '0' + getLed((LedFidEnt)p->fid.tag - FidDot);
    return 1;
  default:
    return 0; /* TODO: error */
  }
}

/*
 * writeLed :: Portal -> [Bytes] -> Int -> Int
 *
 * A write to the control file of a led device sets its activity.
 * Like the read operation zero sized writes and non-zero offsets 
 * result in a non-write.
 */
static int32_t writeLed(struct Portal *p, void *buf, uint32_t size, Offset offset, Err *err) {
  *err = E_OK; /* everythings gonna be alright */
  if (size == 0 || offset != 0) {
    return 0;
  }

  char c = *(char*)buf;
  if (!(c == '0' || c == '1')) {
    return 0;
  }

  switch ((LedFidEnt)p->fid.tag) {
  case FidOrange:
  case FidYellow:
  case FidGreen:
  case FidBlue:
    setLed((LedFidEnt)p->fid.tag - FidDot, (LedState)(c - '0'));
    return 1;
  default:
    return 0;
  }
}

struct Dev ledDev = {
  .id = DEV_DEVLED,
  .name = "led",
  .init = initLed,
  .reset = resetDev,
  .shutdown = shutdownDev,
  .attach = attachLed,
  .create = createDev,
  .open = openLed,
  .close = closeLed,
  .remove = removeDev,
  .read = readLed,
  .write = writeLed
};
