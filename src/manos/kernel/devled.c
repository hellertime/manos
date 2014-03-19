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

#include <libc.h>
#include <arch/mk70f12.h>

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

#define ORANGE_LED_PCR PORTA_PCR10
#define YELLOW_LED_PCR PORTA_PRC28
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
 * Map the LedColor enum to the associated bit pattern
 */
static uint32_t ledColorBits[4] = {
  ORANGE_BIT,
  YELLOW_BIT,
  GREEN_BIT,
  BLUE_BIT
};

/*
 * makeGPIOOut :: LedColor -> ()
 *
 * Setup the appropriate Pin-control register.
 * The LEDs are treated as GPIO output devices.
 */
static void makeGPIOut(LedColor which) {
  int gopi = 1;
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

DevId LED_DEVID = 'l';

typedef enum {
  FidDot,
  FidOrange,
  FidYellow,
  FidGreen,
  FidBlue,
  FidOrangeCtl,
  FidYellowCtl,
  FidGreenCtl,
  FidBlueCtl
} LedFidEnt;

DirEnt ledDirEnt[] = {
  { ".",         { FidDot,       FIDISDIR }, 0, 0555 }, 
  { "orange",    { FidOrange,    0        }, 0, 0444 },
  { "orangeCtl", { FidOrangeCtl, 0        }, 0, 0664 },
  { "yellow",    { FidYellow,    0        }, 0, 0444 },
  { "yellowCtl", { FidYellowCtl, 0        }, 0, 0664 },
  { "green",     { FidGreen,     0        }, 0, 0444 },
  { "greenCtl",  { FidGreenCtl,  0        }, 0, 0664 }
};

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

/*
 * attachLed :: String -> Portal
 *
 * 'attach' message handler. delgates to the generic device handler.
 */
static struct Portal* attachLed(char *path) {
  return attachDev(LED_DEVID, path);
}

/*
 * walkLed :: Portal -> Portal -> [String] -> Trail
 *
 * 'walk' message handler. delegates to the generic device handler, using generic DirEnt iterator
 */
static struct Trail* walkLed(struct Portal *from, struct Portal *to, char **components, size_t count) {
  return walkDev(from, to, component, count, ledDirEnt, COUNT_OF(ledDirEnt), indexDirEntDev);
}

/*
 * getInfoLed :: Portal -> DevInfo -> Err
 *
 * 'getStat' message handler. delegate to the generic device handler.
 */
static Err getStatLed(struct Portal *p, struct DevInfo *info) {
  return getInfoDev(p, info, ledDirEnt, COUNT_OF(ledDirEnt), indexDirEntDev);
}

/*
 * openLed :: Portal -> Mode
 *
 * 'open' message handler. The opens a Portal to the led device.
 * It the Portal ent is one of ./orange, ./blue, etc... then
 * we check to be sure this is a read only open.
 */
static struct Portal* openLed(struct Portal *p, Mode mode) {
  switch ((LedFidEnt)p->fid.ent) {
  case FidOrange:
  case FidYellow:
  case FidGreen:
  case FidBlue:
    if (mode & OpenWrite) return NULL;
  case FidOrangeCtl:
  case FidYellowCtl:
  case FidGreenCtl:
  case FidBlueCtl:
    if (mode & OpenRead) return NULL;
  default:
    break;
  }

  return openDev(p, mode, ledDirEnt, COUNT_OF(ledDirEnt), indexDirEntDev);
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
static int32_t readLed(struct Portal *p, void *buf, uint32_t size, Offset offset) {
  if (size == 0 || offset != 0) {
    return 0;
  }

  if (p->fid.type & FIDISDIR) {
    return readDirDev(p, (struct DevInfo*)buf, size, ledDirEnt, COUNT_OF(ledDirEnt), indexDirEntDev);
  }

  switch ((LedFidEnt)p->fid.ent) {
  case FidOrange:
  case FidYellow:
  case FidGreen:
  case FidBlue:
    *(char*)buf = '0' + getLed((LedFidEnt)p->fid.ent - FidDot);
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

  switch ((LedFidEnt)p->fid.ent) {
  case FidOrangeCtl:
  case FidYellowCtl:
  case FidGreenCtl:
  case FidBlueCtl:
    setLed((LedFidEnt)p->fid.ent - FidBlue, *(char*)buf, (LedState)(c - '0'));
    return 1;
  default:
    return 0;
  }
}

struct Dev* ledDev = {
  LED_DEVID,
  "led",
  initLed,
  resetDev,
  shutdownDev,
  attachLed,
  walkLed,
  getInfoLed,
  setInfoDev,
  createDev,
  openLed,
  closeLed,
  removeDev,
  readLed,
  writeLed
};
