#include <errno.h>
#include <manos.h>
#include <string.h>

#include <arch/k70/derivative.h>

#define R 0
#define G 1
#define B 2

typedef struct Control {
    void*    mmap;
    uint32_t xsize;
    uint32_t ysize;
    uint32_t bpp;
    uint32_t colorMask[3];
    uint32_t colorShift[3];
} Control;

static Control k70Control[] = {
{    .mmap       = (void*)LCD_SDRAM_START
,    .xsize      = LCD_XSIZE
,    .ysize      = LCD_YSIZE
,    .bpp        = LCD_BPP
,    .colorMask  = { 0x00ff0000, 0x0000ff00, 0x000000ff }
,    .colorShift = { 16, 8, 0 }
}
};

extern LcdHw k70LcdHw;

Lcd k70Lcd[] = {
{    .regs = &k70Control[0]
,    .name = "k70Lcd"
,    .hw   = &k70LcdHw
},
};

#define LCD_PIXEL(c, p, w) ((p & (c)->colorMask[(w)]) >> (c)->colorShift[(w)])

static Lcd* k70LcdHotplug(void) {
#ifdef PLATFORM_K70CW
    return &k70Lcd[0];
#else
    return NULL;
#endif
}

static void k70LcdEnable(Lcd* lcd) {
    Control* ctrl = lcd->regs;
    /* enable clocks */
    SIM_SCGC5 |= SIM_SCGC5_PORTE_MASK | SIM_SCGC5_PORTF_MASK;
    SIM_SCGC3 |= SIM_SCGC3_LCDC_MASK;

    /* disable MPU */
    MPU_CESR &= ~MPU_CESR_VLD_MASK;
    
    /* enable pins */
    for (unsigned i = 0; i < 28; i++) {
        switch (i) {
        case 16:
        case 17:
        case 18:
        case 19:
        case 20:
            PORT_PCR_REG(PORTF_BASE_PTR, i) = PORT_PCR_MUX(5) | PORT_PCR_DSE_MASK;
            break;
        default:
            PORT_PCR_REG(PORTF_BASE_PTR, i) = PORT_PCR_MUX(7) | PORT_PCR_DSE_MASK;
        }
    }

    /* base of the video ram */
    LCDC_LSSAR = (uintptr_t)ctrl->mmap;
    /* vram size */
    LCDC_LSR        = LCDC_LSR_XMAX(ctrl->xsize  / 16) | LCDC_LSR_YMAX(ctrl->ysize);
    lcd->fbSize     = ctrl->xsize * ctrl->ysize * ctrl->bpp;
    lcd->colorDepth = ctrl->bpp;
    /* virtual page width */
    LCDC_LVPWR      = LCDC_LVPWR_VPW(ctrl->xsize);
    /* init cursor off */
    LCDC_LCPR  = 0;
    LCDC_LCWHB = 0;

    /* configure hardware */
    LCDC_LPCR = LCDC_LPCR_TFT_MASK      /* isa TFT Screen */
              | LCDC_LPCR_COLOR_MASK    /* has color support */
              | LCDC_LPCR_BPIX(0x7)     /* bpp */
              | LCDC_LPCR_FLMPOL_MASK   /* vsync */
              | LCDC_LPCR_LPPOL_MASK    /* hsync */
              | LCDC_LPCR_END_SEL_MASK  /* big endian  0xffRRGGBB */
              | LCDC_LPCR_SCLKIDLE_MASK /* LSCLK enable when vsync idle */
              | LCDC_LPCR_SCLKSEL_MASK  /* always enable clock */
              | LCDC_LPCR_PCD(11)       /* PLL0 clock divider (11) -> 10MHz */
              ;

    /* set TFT active edge */
    LCDC_LPCR |= LCDC_LPCR_CLKPOL_MASK;
    /* configure horizontal for seiko device */
    LCDC_LHCR = LCDC_LHCR_H_WIDTH(41)   /* 42 cycle hsync */
              | LCDC_LHCR_H_WAIT_1(1)   /* 2 cycle hsync delay */
              | LCDC_LHCR_H_WAIT_2(0)   /* 3 cycle hsync blank */
              ;

    /* configure vertical for seiko device */
    LCDC_LVCR = LCDC_LVCR_V_WIDTH(2)
              | LCDC_LVCR_V_WAIT_1(1)
              | LCDC_LVCR_V_WAIT_2(1)
              ;

    /* disable panning mode in 24bpp */
    LCDC_LPOR = 0;
    /* zero lcd int CR */
    LCDC_LICR = 0;
    /* disable interrupts */
    LCDC_LIER = 0;
    /* disable graphic window */
    LCDC_LGWCR &= ~LCDC_LGWCR_GWE_MASK;
    /* background DMA burst mode */
    LCDC_LDCR &= ~LCDC_LDCR_BURST_MASK;
    /* set graphic window to DMA burst */
    LCDC_LGWDCR &= ~ LCDC_LGWDCR_GWBT_MASK;

    /* setup initital colors bg = white, fg = black */
    lcd->colors.bg = 0x00ffffff;
    lcd->colors.fg = 0;

    lcd->hw->clear(lcd);

    /* start */
    SIM_MCR |= SIM_MCR_LCDSTART_MASK;
}

static void k70LcdDisable(Lcd* lcd) {
    UNUSED(lcd);
    /* noop */
}

static void k70LcdClear(Lcd* lcd) {
    Control* ctrl = lcd->regs;
    uint8_t r = LCD_PIXEL(ctrl, lcd->colors.bg, R);
    uint8_t g = LCD_PIXEL(ctrl, lcd->colors.bg, G);
    uint8_t b = LCD_PIXEL(ctrl, lcd->colors.bg, B);

    if (r == g && r == b) { /* monochromatic case */
        memset(ctrl->mmap, r, lcd->fbSize);
    } else {
        for (uint32_t* pixel = (uint32_t*)ctrl->mmap; pixel < (uint32_t*)ctrl->mmap + (ctrl->xsize + ctrl->ysize); pixel++) {
            *pixel = lcd->colors.bg;
        }
    }
}

LcdHw k70LcdHw = {
    .name    = "k70Lcd"
,   .hotplug = k70LcdHotplug
,   .enable  = k70LcdEnable
,   .disable = k70LcdDisable
,   .clear   = k70LcdClear
};
