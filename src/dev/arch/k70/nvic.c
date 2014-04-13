#include <manos.h>
#include <arch/k70/derivative.h>

/**
 * enableNvicIrq() - enable IRQ with a priority
 * @irq:             IRQ to enable
 * @priority:        priority level to enable with
 */
void enableNvicIrq(unsigned irq, uint8_t priority) {
    if (irq > NVIC_IRQ_MAX || priority > NVIC_IPR_MAX_PRIORITY)
        return;

    /* K70 reference manual (p90) states that the ISER reg num for an IRQ is found with IRQ div 32 */
    /* the manual goes on to state (p91) that to locate a bit field in a register the offset is irq mod 32 */
    NVIC_ISER_REG(NVIC_BASE_PTR, irq >> 5) = NVIC_ISER_SETENA(1 << (irq % 32));

    /* for a given IRQ, the high 4 bits of its 8bit field hold the priority level */
    NVIC_IP_REG(NVIC_BASE_PTR, irq) = (priority & 0xf) << 4;
}
