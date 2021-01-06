/*
 * Copyright 2019 - 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_clock.h"
/* Component ID definition, used by tools. */
#ifndef FSL_COMPONENT_ID
#define FSL_COMPONENT_ID "platform.drivers.clock"
#endif

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* To make full use of CM7 hardware FPU, use double instead of uint64_t in clock driver to
achieve better performance, it is depend on the IDE Floating point settings, if double precision is selected
in IDE, clock_64b_t will switch to double type automatically. only support IAR and MDK here */
#if __FPU_USED

#if (defined(__ICCARM__))

#if (__ARMVFP__ >= __ARMFPV5__) && \
    (__ARM_FP == 0xE) /*0xe implies support for half, single and double precision operations*/
typedef double clock_64b_t;
#else
typedef uint64_t clock_64b_t;
#endif

#elif (defined(__GNUC__))

#if (__ARM_FP == 0xE) /*0xe implies support for half, single and double precision operations*/
typedef double clock_64b_t;
#else
typedef uint64_t clock_64b_t;
#endif

#elif defined(__CC_ARM) || defined(__ARMCC_VERSION)

#if defined __TARGET_FPU_FPV5_D16
typedef double clock_64b_t;
#else
typedef uint64_t clock_64b_t;
#endif

#else
typedef uint64_t clock_64b_t;
#endif

#else
typedef uint64_t clock_64b_t;
#endif

/*******************************************************************************
 * Variables
 ******************************************************************************/

/* External XTAL (OSC) clock frequency. */
volatile uint32_t g_xtalFreq;
/* External RTC XTAL clock frequency. */
volatile uint32_t g_rtcXtalFreq;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*!
 * @brief Get the periph clock frequency.
 *
 * @return Periph clock frequency in Hz.
 */
static uint32_t CLOCK_GetPeriphClkFreq(void);

/*!
 * @brief Get the periph clock2 frequency.
 *
 * @return Periph clock2 frequency in Hz.
 */
static uint32_t CLOCK_GetPeriphClk2Freq(void);

/*!
 * @brief Get Flexspi selection frequency.
 *
 * @return Flexspi selection frequency in Hz.
 */
static uint32_t CLOCK_GetFlexspiSelFreq(void);

/*!
 * @brief Get the frequency of PLL USB1 software clock.
 *
 * @return The frequency of PLL USB1 software clock.
 */
static uint32_t CLOCK_GetPllUsb1SWFreq(void);
/*******************************************************************************
 * Code
 ******************************************************************************/

static uint32_t CLOCK_GetPeriphClkFreq(void)
{
    uint32_t freq;

    /* Periph_clk2_clk ---> Periph_clk */
    if ((CCM->CBCDR & CCM_CBCDR_PERIPH_CLK_SEL_MASK) != 0UL)
    {
        switch (CCM->CBCMR & CCM_CBCMR_PERIPH_CLK2_SEL_MASK)
        {
            /* Pll3_sw_clk ---> Periph_clk2_clk ---> Periph_clk */
            case CCM_CBCMR_PERIPH_CLK2_SEL(0U):
                freq = CLOCK_GetPllFreq(kCLOCK_PllUsb1);
                break;

            /* Osc_clk ---> Periph_clk2_clk ---> Periph_clk */
            case CCM_CBCMR_PERIPH_CLK2_SEL(1U):
                freq = CLOCK_GetOscFreq();
                break;

            case CCM_CBCMR_PERIPH_CLK2_SEL(2U):
                freq = CLOCK_GetPllFreq(kCLOCK_PllSys);
                break;

            case CCM_CBCMR_PERIPH_CLK2_SEL(3U):
            default:
                freq = 0U;
                break;
        }
    }
    /* Pre_Periph_clk ---> Periph_clk */
    else
    {
        switch (CCM->CBCMR & CCM_CBCMR_PRE_PERIPH_CLK_SEL_MASK)
        {
            /* PLL2 */
            case CCM_CBCMR_PRE_PERIPH_CLK_SEL(0U):
                freq = CLOCK_GetPllFreq(kCLOCK_PllSys);
                break;

            /* PLL3 PFD3 */
            case CCM_CBCMR_PRE_PERIPH_CLK_SEL(1U):
                freq = CLOCK_GetUsb1PfdFreq(kCLOCK_Pfd3);
                break;

            /* PLL2 PFD3 */
            case CCM_CBCMR_PRE_PERIPH_CLK_SEL(2U):
                freq = CLOCK_GetSysPfdFreq(kCLOCK_Pfd3);
                break;

            /* PLL6 divided(/1) */
            case CCM_CBCMR_PRE_PERIPH_CLK_SEL(3U):
                freq = 500000000U;
                break;

            default:
                freq = 0U;
                break;
        }
    }

    return freq;
}

static uint32_t CLOCK_GetPllUsb1SWFreq(void)
{
    uint32_t freq;

    switch ((CCM->CCSR & CCM_CCSR_PLL3_SW_CLK_SEL_MASK) >> CCM_CCSR_PLL3_SW_CLK_SEL_SHIFT)
    {
        case 0:
        {
            freq = CLOCK_GetPllFreq(kCLOCK_PllUsb1);
            break;
        }
        case 1:
        {
            freq = 24000000UL;
            break;
        }
        default:
            freq = 0UL;
            break;
    }

    return freq;
}

static uint32_t CLOCK_GetPeriphClk2Freq(void)
{
    uint32_t freq;

    switch (CCM->CBCMR & CCM_CBCMR_PERIPH_CLK2_SEL_MASK)
    {
        /* Pll3_sw_clk ---> Periph_clk2_clk ---> Periph_clk */
        case CCM_CBCMR_PERIPH_CLK2_SEL(0U):
            freq = CLOCK_GetPllFreq(kCLOCK_PllUsb1);
            break;

        /* Osc_clk ---> Periph_clk2_clk ---> Periph_clk */
        case CCM_CBCMR_PERIPH_CLK2_SEL(1U):
            freq = CLOCK_GetOscFreq();
            break;

        case CCM_CBCMR_PERIPH_CLK2_SEL(2U):
            freq = CLOCK_GetPllFreq(kCLOCK_PllSys);
            break;

        case CCM_CBCMR_PERIPH_CLK2_SEL(3U):
        default:
            freq = 0U;
            break;
    }

    return freq;
}

static uint32_t CLOCK_GetFlexspiSelFreq(void)
{
    uint32_t freq;

    switch ((CCM->CSCMR1 & CCM_CSCMR1_FLEXSPI_CLK_SEL_MASK) >> CCM_CSCMR1_FLEXSPI_CLK_SEL_SHIFT)
    {
        case 0:
            freq = CLOCK_GetPllFreq(kCLOCK_PllSys);
            break;
        case 1:
            freq = CLOCK_GetPllUsb1SWFreq();
            break;
        case 2:
            freq = CLOCK_GetSysPfdFreq(kCLOCK_Pfd2);
            break;
        case 3:
            freq = CLOCK_GetUsb1PfdFreq(kCLOCK_Pfd0);
            break;
        default:
            freq = 0UL;
            break;
    }

    return freq;
}

/*!
 * brief Initialize the external 24MHz clock.
 *
 * This function supports two modes:
 * 1. Use external crystal oscillator.
 * 2. Bypass the external crystal oscillator, using input source clock directly.
 *
 * After this function, please call ref CLOCK_SetXtal0Freq to inform clock driver
 * the external clock frequency.
 *
 * param bypassXtalOsc Pass in true to bypass the external crystal oscillator.
 * note This device does not support bypass external crystal oscillator, so
 * the input parameter should always be false.
 */
void CLOCK_InitExternalClk(bool bypassXtalOsc)
{
    /* This device does not support bypass XTAL OSC. */
    assert(!bypassXtalOsc);

    CCM_ANALOG->MISC0_CLR = CCM_ANALOG_MISC0_XTAL_24M_PWD_MASK; /* Power up */
    while ((XTALOSC24M->LOWPWR_CTRL & XTALOSC24M_LOWPWR_CTRL_XTALOSC_PWRUP_STAT_MASK) == 0UL)
    {
    }
    CCM_ANALOG->MISC0_SET = (uint32_t)CCM_ANALOG_MISC0_OSC_XTALOK_EN_MASK; /* detect freq */
    while ((CCM_ANALOG->MISC0 & CCM_ANALOG_MISC0_OSC_XTALOK_MASK) == 0UL)
    {
    }
    CCM_ANALOG->MISC0_CLR = (uint32_t)CCM_ANALOG_MISC0_OSC_XTALOK_EN_MASK;
}

/*!
 * brief Deinitialize the external 24MHz clock.
 *
 * This function disables the external 24MHz clock.
 *
 * After this function, please call ref CLOCK_SetXtal0Freq to set external clock
 * frequency to 0.
 */
void CLOCK_DeinitExternalClk(void)
{
    CCM_ANALOG->MISC0_SET = CCM_ANALOG_MISC0_XTAL_24M_PWD_MASK; /* Power down */
}

/*!
 * brief Switch the OSC.
 *
 * This function switches the OSC source for SoC.
 *
 * param osc   OSC source to switch to.
 */
void CLOCK_SwitchOsc(clock_osc_t osc)
{
    if (osc == kCLOCK_RcOsc)
    {
        XTALOSC24M->LOWPWR_CTRL_SET = XTALOSC24M_LOWPWR_CTRL_SET_OSC_SEL_MASK;
    }
    else
    {
        XTALOSC24M->LOWPWR_CTRL_CLR = XTALOSC24M_LOWPWR_CTRL_CLR_OSC_SEL_MASK;
    }
}

/*!
 * brief Initialize the RC oscillator 24MHz clock.
 */
void CLOCK_InitRcOsc24M(void)
{
    XTALOSC24M->LOWPWR_CTRL |= XTALOSC24M_LOWPWR_CTRL_RC_OSC_EN_MASK;
}

/*!
 * brief Power down the RCOSC 24M clock.
 */
void CLOCK_DeinitRcOsc24M(void)
{
    XTALOSC24M->LOWPWR_CTRL &= ~XTALOSC24M_LOWPWR_CTRL_RC_OSC_EN_MASK;
}

/*!
 * brief Gets the AHB clock frequency.
 *
 * return  The AHB clock frequency value in hertz.
 */
uint32_t CLOCK_GetCoreFreq(void)
{
    return CLOCK_GetPeriphClkFreq() / (((CCM->CBCDR & CCM_CBCDR_AHB_PODF_MASK) >> CCM_CBCDR_AHB_PODF_SHIFT) + 1U);
}

/*!
 * brief Gets the IPG clock frequency.
 *
 * return  The IPG clock frequency value in hertz.
 */
uint32_t CLOCK_GetIpgFreq(void)
{
    return CLOCK_GetCoreFreq() / (((CCM->CBCDR & CCM_CBCDR_IPG_PODF_MASK) >> CCM_CBCDR_IPG_PODF_SHIFT) + 1U);
}

/*!
 * brief Gets the PER clock frequency.
 *
 * return  The PER clock frequency value in hertz.
 */
uint32_t CLOCK_GetPerClkFreq(void)
{
    uint32_t freq;

    /* Osc_clk ---> PER Clock*/
    if ((CCM->CSCMR1 & CCM_CSCMR1_PERCLK_CLK_SEL_MASK) != 0UL)
    {
        freq = CLOCK_GetOscFreq();
    }
    /* Periph_clk ---> AHB Clock ---> IPG Clock ---> PER Clock */
    else
    {
        freq = CLOCK_GetIpgFreq();
    }

    freq /= (((CCM->CSCMR1 & CCM_CSCMR1_PERCLK_PODF_MASK) >> CCM_CSCMR1_PERCLK_PODF_SHIFT) + 1U);

    return freq;
}

/*!
 * brief Gets the clock frequency for a specific clock name.
 *
 * This function checks the current clock configurations and then calculates
 * the clock frequency for a specific clock name defined in clock_name_t.
 *
 * param clockName Clock names defined in clock_name_t
 * return Clock frequency value in hertz
 */
uint32_t CLOCK_GetFreq(clock_name_t name)
{
    uint32_t freq;

    switch (name)
    {
        case kCLOCK_CpuClk:
        case kCLOCK_CoreClk:
            freq = CLOCK_GetCoreFreq();
            break;

        case kCLOCK_IpgClk:
            freq = CLOCK_GetIpgFreq();
            break;

        case kCLOCK_PerClk:
            freq = CLOCK_GetPerClkFreq();
            break;

        case kCLOCK_OscClk:
            freq = CLOCK_GetOscFreq();
            break;
        case kCLOCK_RtcClk:
            freq = CLOCK_GetRtcFreq();
            break;
        case kCLOCK_Usb1PllClk:
            freq = CLOCK_GetPllFreq(kCLOCK_PllUsb1);
            break;
        case kCLOCK_Usb1PllPfd0Clk:
            freq = CLOCK_GetUsb1PfdFreq(kCLOCK_Pfd0);
            break;
        case kCLOCK_Usb1PllPfd1Clk:
            freq = CLOCK_GetUsb1PfdFreq(kCLOCK_Pfd1);
            break;
        case kCLOCK_Usb1PllPfd2Clk:
            freq = CLOCK_GetUsb1PfdFreq(kCLOCK_Pfd2);
            break;
        case kCLOCK_Usb1PllPfd3Clk:
            freq = CLOCK_GetUsb1PfdFreq(kCLOCK_Pfd3);
            break;
        case kCLOCK_Usb1SwClk:
            freq = CLOCK_GetPllUsb1SWFreq();
            break;
        case kCLOCK_Usb1Sw60MClk:
            freq = CLOCK_GetPllUsb1SWFreq() / 8UL;
            break;
        case kCLOCK_Usb1Sw80MClk:
            freq = CLOCK_GetPllUsb1SWFreq() / 6UL;
            break;
        case kCLOCK_SysPllClk:
            freq = CLOCK_GetPllFreq(kCLOCK_PllSys);
            break;
        case kCLOCK_SysPllPfd0Clk:
            freq = CLOCK_GetSysPfdFreq(kCLOCK_Pfd0);
            break;
        case kCLOCK_SysPllPfd1Clk:
            freq = CLOCK_GetSysPfdFreq(kCLOCK_Pfd1);
            break;
        case kCLOCK_SysPllPfd2Clk:
            freq = CLOCK_GetSysPfdFreq(kCLOCK_Pfd2);
            break;
        case kCLOCK_SysPllPfd3Clk:
            freq = CLOCK_GetSysPfdFreq(kCLOCK_Pfd3);
            break;
        case kCLOCK_EnetPll500MClk:
            freq = CLOCK_GetPllFreq(kCLOCK_PllEnet500M);
            break;
        case kCLOCK_AudioPllClk:
            freq = CLOCK_GetPllFreq(kCLOCK_PllAudio);
            break;
        default:
            freq = 0U;
            break;
    }

    return freq;
}

/*!
 * brief Gets the frequency of selected clock root.
 *
 * param clockRoot The clock root used to get the frequency, please refer to @ref clock_root_t.
 * return The frequency of selected clock root.
 */
uint32_t CLOCK_GetClockRootFreq(clock_root_t clockRoot)
{
    const clock_name_t clockRootSourceArray[][4]  = CLOCK_ROOT_SOUCE;
    const clock_mux_t clockRootMuxTupleArray[]    = CLOCK_ROOT_MUX_TUPLE;
    const clock_div_t clockRootDivTupleArray[][2] = CLOCK_ROOT_DIV_TUPLE;
    uint32_t freq                                 = 0UL;
    clock_mux_t clockRootMuxTuple                 = clockRootMuxTupleArray[(uint8_t)clockRoot];
    clock_div_t clockRootPreDivTuple              = clockRootDivTupleArray[(uint8_t)clockRoot][0];
    clock_div_t clockRootPostDivTuple             = clockRootDivTupleArray[(uint8_t)clockRoot][1];
    uint32_t clockRootMuxValue = (CCM_TUPLE_REG(CCM, clockRootMuxTuple) & CCM_TUPLE_MASK(clockRootMuxTuple)) >>
                                 CCM_TUPLE_SHIFT(clockRootMuxTuple);
    clock_name_t clockSourceName;

    clockSourceName = clockRootSourceArray[(uint8_t)clockRoot][clockRootMuxValue];

    assert(clockSourceName != kCLOCK_NoneName);

    if (clockSourceName == kCLOCK_PeriphClk2)
    {
        freq = CLOCK_GetPeriphClk2Freq();
    }
    else if (clockSourceName == kCLOCK_FlexspiSel)
    {
        freq = CLOCK_GetFlexspiSelFreq();
    }
    else
    {
        freq = CLOCK_GetFreq(clockSourceName);
    }

    if (clockRootPreDivTuple != kCLOCK_NonePreDiv)
    {
        freq /= ((CCM_TUPLE_REG(CCM, clockRootPreDivTuple) & CCM_TUPLE_MASK(clockRootPreDivTuple)) >>
                 CCM_TUPLE_SHIFT(clockRootPreDivTuple)) +
                1UL;
    }

    freq /= ((CCM_TUPLE_REG(CCM, clockRootPostDivTuple) & CCM_TUPLE_MASK(clockRootPostDivTuple)) >>
             CCM_TUPLE_SHIFT(clockRootPostDivTuple)) +
            1UL;

    return freq;
}

/*! brief Enable USB HS clock.
 *
 * This function only enables the access to USB HS prepheral, upper layer
 * should first call the ref CLOCK_EnableUsbhs0PhyPllClock to enable the PHY
 * clock to use USB HS.
 *
 * param src  USB HS does not care about the clock source, here must be ref kCLOCK_UsbSrcUnused.
 * param freq USB HS does not care about the clock source, so this parameter is ignored.
 * retval true The clock is set successfully.
 * retval false The clock source is invalid to get proper USB HS clock.
 */
bool CLOCK_EnableUsbhs0Clock(clock_usb_src_t src, uint32_t freq)
{
    uint32_t i;
    CCM->CCGR6 |= CCM_CCGR6_CG0_MASK;
    USB->USBCMD |= USBHS_USBCMD_RST_MASK;

    /* Add a delay between RST and RS so make sure there is a DP pullup sequence*/
    for (i = 0; i < 400000UL; i++)
    {
        __ASM("nop");
    }
    PMU->REG_3P0 = (PMU->REG_3P0 & (~PMU_REG_3P0_OUTPUT_TRG_MASK)) |
                   (PMU_REG_3P0_OUTPUT_TRG(0x17) | PMU_REG_3P0_ENABLE_LINREG_MASK);
    return true;
}

/*! brief Enable USB HS PHY PLL clock.
 *
 * This function enables the internal 480MHz USB PHY PLL clock.
 *
 * param src  USB HS PHY PLL clock source.
 * param freq The frequency specified by src.
 * retval true The clock is set successfully.
 * retval false The clock source is invalid to get proper USB HS clock.
 */
bool CLOCK_EnableUsbhs0PhyPllClock(clock_usb_phy_src_t src, uint32_t freq)
{
    const clock_usb_pll_config_t g_ccmConfigUsbPll = {.loopDivider = 0U};
    if ((CCM_ANALOG->PLL_USB1 & CCM_ANALOG_PLL_USB1_ENABLE_MASK) != 0UL)
    {
        CCM_ANALOG->PLL_USB1 |= CCM_ANALOG_PLL_USB1_EN_USB_CLKS_MASK;
    }
    else
    {
        CLOCK_InitUsb1Pll(&g_ccmConfigUsbPll);
    }
    USBPHY->CTRL &= ~USBPHY_CTRL_SFTRST_MASK; /* release PHY from reset */
    USBPHY->CTRL &= ~USBPHY_CTRL_CLKGATE_MASK;

    USBPHY->PWD = 0;
    USBPHY->CTRL |= USBPHY_CTRL_ENAUTOCLR_PHY_PWD_MASK | USBPHY_CTRL_ENAUTOCLR_CLKGATE_MASK |
                    USBPHY_CTRL_ENUTMILEVEL2_MASK | USBPHY_CTRL_ENUTMILEVEL3_MASK;
    return true;
}

/*! brief Disable USB HS PHY PLL clock.
 *
 * This function disables USB HS PHY PLL clock.
 */
void CLOCK_DisableUsbhs0PhyPllClock(void)
{
    CCM_ANALOG->PLL_USB1 &= ~CCM_ANALOG_PLL_USB1_EN_USB_CLKS_MASK;
    USBPHY->CTRL |= USBPHY_CTRL_CLKGATE_MASK; /* Set to 1U to gate clocks */
}

/*!
 * brief Initialize the System PLL.
 *
 * This function initializes the System PLL with specific settings
 *
 * param config Configuration to set to PLL.
 */
void CLOCK_InitSysPll(const clock_sys_pll_config_t *config)
{
    /* Bypass PLL first */
    CCM_ANALOG->PLL_SYS = (CCM_ANALOG->PLL_SYS & (~CCM_ANALOG_PLL_SYS_BYPASS_CLK_SRC_MASK)) |
                          CCM_ANALOG_PLL_SYS_BYPASS_MASK | CCM_ANALOG_PLL_SYS_BYPASS_CLK_SRC(config->src);

    CCM_ANALOG->PLL_SYS =
        (CCM_ANALOG->PLL_SYS & (~(CCM_ANALOG_PLL_SYS_DIV_SELECT_MASK | CCM_ANALOG_PLL_SYS_POWERDOWN_MASK))) |
        CCM_ANALOG_PLL_SYS_ENABLE_MASK | CCM_ANALOG_PLL_SYS_DIV_SELECT(config->loopDivider);

    /* Initialize the fractional mode */
    CCM_ANALOG->PLL_SYS_NUM   = CCM_ANALOG_PLL_SYS_NUM_A(config->numerator);
    CCM_ANALOG->PLL_SYS_DENOM = CCM_ANALOG_PLL_SYS_DENOM_B(config->denominator);

    /* Initialize the spread spectrum mode */
    CCM_ANALOG->PLL_SYS_SS = CCM_ANALOG_PLL_SYS_SS_STEP(config->ss_step) |
                             CCM_ANALOG_PLL_SYS_SS_ENABLE(config->ss_enable) |
                             CCM_ANALOG_PLL_SYS_SS_STOP(config->ss_stop);

    while ((CCM_ANALOG->PLL_SYS & CCM_ANALOG_PLL_SYS_LOCK_MASK) == 0UL)
    {
    }

    /* Disable Bypass */
    CCM_ANALOG->PLL_SYS &= ~CCM_ANALOG_PLL_SYS_BYPASS_MASK;
}

/*!
 * brief De-initialize the System PLL.
 */
void CLOCK_DeinitSysPll(void)
{
    CCM_ANALOG->PLL_SYS = CCM_ANALOG_PLL_SYS_POWERDOWN_MASK;
}

/*!
 * brief Initialize the USB1 PLL.
 *
 * This function initializes the USB1 PLL with specific settings
 *
 * param config Configuration to set to PLL.
 */
void CLOCK_InitUsb1Pll(const clock_usb_pll_config_t *config)
{
    /* Bypass PLL first */
    CCM_ANALOG->PLL_USB1 = (CCM_ANALOG->PLL_USB1 & (~CCM_ANALOG_PLL_USB1_BYPASS_CLK_SRC_MASK)) |
                           CCM_ANALOG_PLL_USB1_BYPASS_MASK | CCM_ANALOG_PLL_USB1_BYPASS_CLK_SRC(config->src);

    CCM_ANALOG->PLL_USB1 = (CCM_ANALOG->PLL_USB1 & (~CCM_ANALOG_PLL_USB1_DIV_SELECT_MASK)) |
                           CCM_ANALOG_PLL_USB1_ENABLE_MASK | CCM_ANALOG_PLL_USB1_POWER_MASK |
                           CCM_ANALOG_PLL_USB1_EN_USB_CLKS_MASK | CCM_ANALOG_PLL_USB1_DIV_SELECT(config->loopDivider);

    while ((CCM_ANALOG->PLL_USB1 & CCM_ANALOG_PLL_USB1_LOCK_MASK) == 0UL)
    {
    }

    /* Disable Bypass */
    CCM_ANALOG->PLL_USB1 &= ~CCM_ANALOG_PLL_USB1_BYPASS_MASK;
}

/*!
 * brief Deinitialize the USB1 PLL.
 */
void CLOCK_DeinitUsb1Pll(void)
{
    CCM_ANALOG->PLL_USB1 = 0U;
}

/*!
 * brief Initializes the Audio PLL.
 *
 * This function initializes the Audio PLL with specific settings
 *
 * param config Configuration to set to PLL.
 */
void CLOCK_InitAudioPll(const clock_audio_pll_config_t *config)
{
    uint32_t pllAudio;
    uint32_t misc2 = 0;

    /* Bypass PLL first */
    CCM_ANALOG->PLL_AUDIO = (CCM_ANALOG->PLL_AUDIO & (~CCM_ANALOG_PLL_AUDIO_BYPASS_CLK_SRC_MASK)) |
                            CCM_ANALOG_PLL_AUDIO_BYPASS_MASK | CCM_ANALOG_PLL_AUDIO_BYPASS_CLK_SRC(config->src);

    CCM_ANALOG->PLL_AUDIO_NUM   = CCM_ANALOG_PLL_AUDIO_NUM_A(config->numerator);
    CCM_ANALOG->PLL_AUDIO_DENOM = CCM_ANALOG_PLL_AUDIO_DENOM_B(config->denominator);

    /*
     * Set post divider:
     *
     * ------------------------------------------------------------------------
     * | config->postDivider | PLL_AUDIO[POST_DIV_SELECT]  | MISC2[AUDIO_DIV] |
     * ------------------------------------------------------------------------
     * |         1           |            2                |        0         |
     * ------------------------------------------------------------------------
     * |         2           |            1                |        0         |
     * ------------------------------------------------------------------------
     * |         4           |            2                |        3         |
     * ------------------------------------------------------------------------
     * |         8           |            1                |        3         |
     * ------------------------------------------------------------------------
     * |         16          |            0                |        3         |
     * ------------------------------------------------------------------------
     */
    pllAudio =
        (CCM_ANALOG->PLL_AUDIO & (~(CCM_ANALOG_PLL_AUDIO_DIV_SELECT_MASK | CCM_ANALOG_PLL_AUDIO_POWERDOWN_MASK))) |
        CCM_ANALOG_PLL_AUDIO_ENABLE_MASK | CCM_ANALOG_PLL_AUDIO_DIV_SELECT(config->loopDivider);

    switch (config->postDivider)
    {
        case 16:
            pllAudio |= CCM_ANALOG_PLL_AUDIO_POST_DIV_SELECT(0);
            misc2 = CCM_ANALOG_MISC2_AUDIO_DIV_MSB_MASK | CCM_ANALOG_MISC2_AUDIO_DIV_LSB_MASK;
            break;

        case 8:
            pllAudio |= CCM_ANALOG_PLL_AUDIO_POST_DIV_SELECT(1);
            misc2 = CCM_ANALOG_MISC2_AUDIO_DIV_MSB_MASK | CCM_ANALOG_MISC2_AUDIO_DIV_LSB_MASK;
            break;

        case 4:
            pllAudio |= CCM_ANALOG_PLL_AUDIO_POST_DIV_SELECT(2);
            misc2 = CCM_ANALOG_MISC2_AUDIO_DIV_MSB_MASK | CCM_ANALOG_MISC2_AUDIO_DIV_LSB_MASK;
            break;

        case 2:
            pllAudio |= CCM_ANALOG_PLL_AUDIO_POST_DIV_SELECT(1);
            break;

        default:
            pllAudio |= CCM_ANALOG_PLL_AUDIO_POST_DIV_SELECT(2);
            break;
    }

    CCM_ANALOG->MISC2 =
        (CCM_ANALOG->MISC2 & ~(CCM_ANALOG_MISC2_AUDIO_DIV_LSB_MASK | CCM_ANALOG_MISC2_AUDIO_DIV_MSB_MASK)) | misc2;

    CCM_ANALOG->PLL_AUDIO = pllAudio;

    while ((CCM_ANALOG->PLL_AUDIO & CCM_ANALOG_PLL_AUDIO_LOCK_MASK) == 0UL)
    {
    }

    /* Disable Bypass */
    CCM_ANALOG->PLL_AUDIO &= ~CCM_ANALOG_PLL_AUDIO_BYPASS_MASK;
}

/*!
 * brief De-initialize the Audio PLL.
 */
void CLOCK_DeinitAudioPll(void)
{
    CCM_ANALOG->PLL_AUDIO = CCM_ANALOG_PLL_AUDIO_POWERDOWN_MASK;
}

/*!
 * brief Initialize the ENET PLL.
 *
 * This function initializes the ENET PLL with specific settings.
 *
 * param config Configuration to set to PLL.
 */
void CLOCK_InitEnetPll(const clock_enet_pll_config_t *config)
{
    uint32_t enet_pll = 0;

    CCM_ANALOG->PLL_ENET = (CCM_ANALOG->PLL_ENET & (~CCM_ANALOG_PLL_ENET_BYPASS_CLK_SRC_MASK)) |
                           CCM_ANALOG_PLL_ENET_BYPASS_MASK | CCM_ANALOG_PLL_ENET_BYPASS_CLK_SRC(config->src);

    if (config->enableClkOutput500M)
    {
        enet_pll |= CCM_ANALOG_PLL_ENET_ENET_500M_REF_EN_MASK;
    }

    CCM_ANALOG->PLL_ENET = (CCM_ANALOG->PLL_ENET & (~CCM_ANALOG_PLL_ENET_POWERDOWN_MASK)) | enet_pll;

    /* Wait for stable */
    while ((CCM_ANALOG->PLL_ENET & CCM_ANALOG_PLL_ENET_LOCK_MASK) == 0UL)
    {
    }

    /* Disable Bypass */
    CCM_ANALOG->PLL_ENET &= ~CCM_ANALOG_PLL_ENET_BYPASS_MASK;
}

/*!
 * brief Deinitialize the ENET PLL.
 *
 * This function disables the ENET PLL.
 */
void CLOCK_DeinitEnetPll(void)
{
    CCM_ANALOG->PLL_ENET = CCM_ANALOG_PLL_ENET_POWERDOWN_MASK;
}

/*!
 * brief Get current PLL output frequency.
 *
 * This function get current output frequency of specific PLL
 *
 * param pll   pll name to get frequency.
 * return The PLL output frequency in hertz.
 */
uint32_t CLOCK_GetPllFreq(clock_pll_t pll)
{
    uint32_t freq;
    uint32_t divSelect;
    clock_64b_t freqTmp;

    /* check if PLL is enabled */
    if (!CLOCK_IsPllEnabled(CCM_ANALOG, pll))
    {
        return 0U;
    }

    /* get pll reference clock */
    freq = CLOCK_GetPllBypassRefClk(CCM_ANALOG, pll);

    /* check if pll is bypassed */
    if (CLOCK_IsPllBypassed(CCM_ANALOG, pll))
    {
        return freq;
    }

    switch (pll)
    {
        case kCLOCK_PllSys:
            /* PLL output frequency = Fref * (DIV_SELECT + NUM/DENOM). */
            freqTmp = ((clock_64b_t)freq * ((clock_64b_t)(CCM_ANALOG->PLL_SYS_NUM)));
            freqTmp /= ((clock_64b_t)(CCM_ANALOG->PLL_SYS_DENOM));

            if ((CCM_ANALOG->PLL_SYS & CCM_ANALOG_PLL_SYS_DIV_SELECT_MASK) != 0UL)
            {
                freq *= 22U;
            }
            else
            {
                freq *= 20U;
            }

            freq += (uint32_t)freqTmp;
            break;

        case kCLOCK_PllUsb1:
            freq = (freq * (((CCM_ANALOG->PLL_USB1 & CCM_ANALOG_PLL_USB1_DIV_SELECT_MASK) != 0UL) ? 22U : 20U));
            break;

        case kCLOCK_PllAudio:
            /* PLL output frequency = Fref * (DIV_SELECT + NUM/DENOM). */
            divSelect =
                (CCM_ANALOG->PLL_AUDIO & CCM_ANALOG_PLL_AUDIO_DIV_SELECT_MASK) >> CCM_ANALOG_PLL_AUDIO_DIV_SELECT_SHIFT;

            freqTmp = ((clock_64b_t)freq * ((clock_64b_t)(CCM_ANALOG->PLL_AUDIO_NUM)));
            freqTmp /= ((clock_64b_t)(CCM_ANALOG->PLL_AUDIO_DENOM));

            freq = freq * divSelect + (uint32_t)freqTmp;

            /* AUDIO PLL output = PLL output frequency / POSTDIV. */

            /*
             * Post divider:
             *
             * PLL_AUDIO[POST_DIV_SELECT]:
             * 0x00: 4
             * 0x01: 2
             * 0x02: 1
             *
             * MISC2[AUDO_DIV]:
             * 0x00: 1
             * 0x01: 2
             * 0x02: 1
             * 0x03: 4
             */
            switch (CCM_ANALOG->PLL_AUDIO & CCM_ANALOG_PLL_AUDIO_POST_DIV_SELECT_MASK)
            {
                case CCM_ANALOG_PLL_AUDIO_POST_DIV_SELECT(0U):
                    freq = freq >> 2U;
                    break;

                case CCM_ANALOG_PLL_AUDIO_POST_DIV_SELECT(1U):
                    freq = freq >> 1U;
                    break;

                default:
                    /* Add comment to avoid MISRA C-2012 rule 16.4 */
                    break;
            }

            switch (CCM_ANALOG->MISC2 & (CCM_ANALOG_MISC2_AUDIO_DIV_MSB_MASK | CCM_ANALOG_MISC2_AUDIO_DIV_LSB_MASK))
            {
                case CCM_ANALOG_MISC2_AUDIO_DIV_MSB(1) | CCM_ANALOG_MISC2_AUDIO_DIV_LSB(1):
                    freq >>= 2U;
                    break;

                case CCM_ANALOG_MISC2_AUDIO_DIV_MSB(0) | CCM_ANALOG_MISC2_AUDIO_DIV_LSB(1):
                    freq >>= 1U;
                    break;

                default:
                    /* Add comment to avoid MISRA C-2012 rule 16.4 */
                    break;
            }
            break;

        case kCLOCK_PllEnet500M:
            /* PLL6 is fixed at 25MHz. */
            freq = 500000000UL;
            break;

        default:
            freq = 0U;
            break;
    }

    return freq;
}

/*!
 * brief Initialize the System PLL PFD.
 *
 * This function initializes the System PLL PFD. During new value setting,
 * the clock output is disabled to prevent glitch.
 *
 * param pfd Which PFD clock to enable.
 * param pfdFrac The PFD FRAC value.
 * note It is recommended that PFD settings are kept between 12-35.
 */
void CLOCK_InitSysPfd(clock_pfd_t pfd, uint8_t pfdFrac)
{
    uint32_t pfdIndex = (uint32_t)pfd;
    uint32_t pfd528;

    pfd528 = CCM_ANALOG->PFD_528 &
             ~(((uint32_t)((uint32_t)CCM_ANALOG_PFD_528_PFD0_CLKGATE_MASK | (uint32_t)CCM_ANALOG_PFD_528_PFD0_FRAC_MASK)
                << (8UL * pfdIndex)));

    /* Disable the clock output first. */
    CCM_ANALOG->PFD_528 = pfd528 | ((uint32_t)CCM_ANALOG_PFD_528_PFD0_CLKGATE_MASK << (8UL * pfdIndex));

    /* Set the new value and enable output. */
    CCM_ANALOG->PFD_528 = pfd528 | (CCM_ANALOG_PFD_528_PFD0_FRAC(pfdFrac) << (8UL * pfdIndex));
}

/*!
 * brief De-initialize the System PLL PFD.
 *
 * This function disables the System PLL PFD.
 *
 * param pfd Which PFD clock to disable.
 */
void CLOCK_DeinitSysPfd(clock_pfd_t pfd)
{
    CCM_ANALOG->PFD_528 |= (uint32_t)CCM_ANALOG_PFD_528_PFD0_CLKGATE_MASK << (8UL * (uint32_t)pfd);
}

/*!
 * brief Initialize the USB1 PLL PFD.
 *
 * This function initializes the USB1 PLL PFD. During new value setting,
 * the clock output is disabled to prevent glitch.
 *
 * param pfd Which PFD clock to enable.
 * param pfdFrac The PFD FRAC value.
 * note It is recommended that PFD settings are kept between 12-35.
 */
void CLOCK_InitUsb1Pfd(clock_pfd_t pfd, uint8_t pfdFrac)
{
    uint32_t pfdIndex = (uint32_t)pfd;
    uint32_t pfd480;

    pfd480 = CCM_ANALOG->PFD_480 &
             ~((uint32_t)((uint32_t)CCM_ANALOG_PFD_480_PFD0_CLKGATE_MASK | (uint32_t)CCM_ANALOG_PFD_480_PFD0_FRAC_MASK)
               << (8UL * pfdIndex));

    /* Disable the clock output first. */
    CCM_ANALOG->PFD_480 = pfd480 | ((uint32_t)CCM_ANALOG_PFD_480_PFD0_CLKGATE_MASK << (8UL * pfdIndex));

    /* Set the new value and enable output. */
    CCM_ANALOG->PFD_480 = pfd480 | (CCM_ANALOG_PFD_480_PFD0_FRAC(pfdFrac) << (8UL * pfdIndex));
}

/*!
 * brief De-initialize the USB1 PLL PFD.
 *
 * This function disables the USB1 PLL PFD.
 *
 * param pfd Which PFD clock to disable.
 */
void CLOCK_DeinitUsb1Pfd(clock_pfd_t pfd)
{
    CCM_ANALOG->PFD_480 |= (uint32_t)CCM_ANALOG_PFD_480_PFD0_CLKGATE_MASK << (8UL * (uint32_t)pfd);
}

/*!
 * brief Get current System PLL PFD output frequency.
 *
 * This function get current output frequency of specific System PLL PFD
 *
 * param pfd   pfd name to get frequency.
 * return The PFD output frequency in hertz.
 */
uint32_t CLOCK_GetSysPfdFreq(clock_pfd_t pfd)
{
    uint32_t freq = CLOCK_GetPllFreq(kCLOCK_PllSys);

    switch (pfd)
    {
        case kCLOCK_Pfd0:
            freq /= ((CCM_ANALOG->PFD_528 & CCM_ANALOG_PFD_528_PFD0_FRAC_MASK) >> CCM_ANALOG_PFD_528_PFD0_FRAC_SHIFT);
            break;

        case kCLOCK_Pfd1:
            freq /= ((CCM_ANALOG->PFD_528 & CCM_ANALOG_PFD_528_PFD1_FRAC_MASK) >> CCM_ANALOG_PFD_528_PFD1_FRAC_SHIFT);
            break;

        case kCLOCK_Pfd2:
            freq /= ((CCM_ANALOG->PFD_528 & CCM_ANALOG_PFD_528_PFD2_FRAC_MASK) >> CCM_ANALOG_PFD_528_PFD2_FRAC_SHIFT);
            break;

        case kCLOCK_Pfd3:
            freq /= ((CCM_ANALOG->PFD_528 & CCM_ANALOG_PFD_528_PFD3_FRAC_MASK) >> CCM_ANALOG_PFD_528_PFD3_FRAC_SHIFT);
            break;

        default:
            freq = 0U;
            break;
    }
    freq *= 18U;

    return freq;
}

/*!
 * brief Get current USB1 PLL PFD output frequency.
 *
 * This function get current output frequency of specific USB1 PLL PFD
 *
 * param pfd   pfd name to get frequency.
 * return The PFD output frequency in hertz.
 */
uint32_t CLOCK_GetUsb1PfdFreq(clock_pfd_t pfd)
{
    uint32_t freq = CLOCK_GetPllFreq(kCLOCK_PllUsb1);

    switch (pfd)
    {
        case kCLOCK_Pfd0:
            freq /= ((CCM_ANALOG->PFD_480 & CCM_ANALOG_PFD_480_PFD0_FRAC_MASK) >> CCM_ANALOG_PFD_480_PFD0_FRAC_SHIFT);
            break;

        case kCLOCK_Pfd1:
            freq /= ((CCM_ANALOG->PFD_480 & CCM_ANALOG_PFD_480_PFD1_FRAC_MASK) >> CCM_ANALOG_PFD_480_PFD1_FRAC_SHIFT);
            break;

        case kCLOCK_Pfd2:
            freq /= ((CCM_ANALOG->PFD_480 & CCM_ANALOG_PFD_480_PFD2_FRAC_MASK) >> CCM_ANALOG_PFD_480_PFD2_FRAC_SHIFT);
            break;

        case kCLOCK_Pfd3:
            freq /= ((CCM_ANALOG->PFD_480 & CCM_ANALOG_PFD_480_PFD3_FRAC_MASK) >> CCM_ANALOG_PFD_480_PFD3_FRAC_SHIFT);
            break;

        default:
            freq = 0U;
            break;
    }
    freq *= 18U;

    return freq;
}

/*!
 * brief Set the clock source and the divider of the clock output1.
 *
 * param selection The clock source to be output, please refer to clock_output1_selection_t.
 * param divider The divider of the output clock signal, please refer to clock_output_divider_t.
 */
void CLOCK_SetClockOutput1(clock_output1_selection_t selection, clock_output_divider_t divider)
{
    uint32_t tmp32;

    tmp32 = CCM->CCOSR;
    if (selection == kCLOCK_DisableClockOutput1)
    {
        tmp32 &= ~CCM_CCOSR_CLKO1_EN_MASK;
    }
    else
    {
        tmp32 |= CCM_CCOSR_CLKO1_EN_MASK;
        tmp32 &= ~(CCM_CCOSR_CLKO1_SEL_MASK | CCM_CCOSR_CLKO1_DIV_MASK);
        tmp32 |= CCM_CCOSR_CLKO1_SEL(selection) | CCM_CCOSR_CLKO1_DIV(divider);
    }
    CCM->CCOSR = tmp32;
}

/*!
 * brief Set the clock source and the divider of the clock output2.
 *
 * param selection The clock source to be output, please refer to clock_output2_selection_t.
 * param divider The divider of the output clock signal, please refer to clock_output_divider_t.
 */
void CLOCK_SetClockOutput2(clock_output2_selection_t selection, clock_output_divider_t divider)
{
    uint32_t tmp32;

    tmp32 = CCM->CCOSR;
    if (selection == kCLOCK_DisableClockOutput2)
    {
        tmp32 &= CCM_CCOSR_CLKO2_EN_MASK;
    }
    else
    {
        tmp32 |= CCM_CCOSR_CLKO2_EN_MASK;
        tmp32 &= ~(CCM_CCOSR_CLKO2_SEL_MASK | CCM_CCOSR_CLKO2_DIV_MASK);
        tmp32 |= CCM_CCOSR_CLKO2_SEL(selection) | CCM_CCOSR_CLKO2_DIV(divider);
    }

    CCM->CCOSR = tmp32;
}

/*!
 * brief Get the frequency of clock output1 clock signal.
 *
 * return The frequency of clock output1 clock signal.
 */
uint32_t CLOCK_GetClockOutCLKO1Freq(void)
{
    uint32_t freq = 0U;
    uint32_t tmp32;

    tmp32 = CCM->CCOSR;

    if ((tmp32 & CCM_CCOSR_CLKO1_EN_MASK) != 0UL)
    {
        switch ((tmp32 & CCM_CCOSR_CLKO1_SEL_MASK) >> CCM_CCOSR_CLKO1_SEL_SHIFT)
        {
            case (uint32_t)kCLOCK_OutputPllUsb1Sw:
                freq = CLOCK_GetPllUsb1SWFreq() / 2UL;
                break;
            case (uint32_t)kCLOCK_OutputPllSys:
                freq = CLOCK_GetPllFreq(kCLOCK_PllSys) / 2UL;
                break;
            case (uint32_t)kCLOCK_OutputPllENET:
                freq = CLOCK_GetPllFreq(kCLOCK_PllEnet500M) / 2UL;
                break;
            case (uint32_t)kCLOCK_OutputCoreClk:
                freq = CLOCK_GetCoreFreq();
                break;
            case (uint32_t)kCLOCK_OutputIpgClk:
                freq = CLOCK_GetIpgFreq();
                break;
            case (uint32_t)kCLOCK_OutputPerClk:
                freq = CLOCK_GetPerClkFreq();
                break;
            case (uint32_t)kCLOCK_OutputPll4MainClk:
                freq = CLOCK_GetPllFreq(kCLOCK_PllAudio);
                break;
            default:
                /* This branch should never be hit. */
                break;
        }

        freq /= (((tmp32 & CCM_CCOSR_CLKO1_DIV_MASK) >> CCM_CCOSR_CLKO1_DIV_SHIFT) + 1U);
    }
    else
    {
        freq = 0UL;
    }

    return freq;
}

/*!
 * brief Get the frequency of clock output2 clock signal.
 *
 * return The frequency of clock output2 clock signal.
 */
uint32_t CLOCK_GetClockOutClkO2Freq(void)
{
    uint32_t freq = 0U;
    uint32_t tmp32;

    tmp32 = CCM->CCOSR;

    if ((tmp32 & CCM_CCOSR_CLKO2_EN_MASK) != 0UL)
    {
        switch ((tmp32 & CCM_CCOSR_CLKO2_SEL_MASK) >> CCM_CCOSR_CLKO2_SEL_SHIFT)
        {
            case (uint32_t)kCLOCK_OutputLpi2cClk:
                freq = CLOCK_GetClockRootFreq(kCLOCK_Lpi2cClkRoot);
                break;
            case (uint32_t)kCLOCK_OutputOscClk:
                freq = CLOCK_GetOscFreq();
                break;
            case (uint32_t)kCLOCK_OutputLpspiClk:
                freq = CLOCK_GetClockRootFreq(kCLOCK_LpspiClkRoot);
                break;
            case (uint32_t)kCLOCK_OutputSai1Clk:
                freq = CLOCK_GetClockRootFreq(kCLOCK_Sai1ClkRoot);
                break;
            case (uint32_t)kCLOCK_OutputSai3Clk:
                freq = CLOCK_GetClockRootFreq(kCLOCK_Sai3ClkRoot);
                break;
            case (uint32_t)kCLOCK_OutputTraceClk:
                freq = CLOCK_GetClockRootFreq(kCLOCK_TraceClkRoot);
                break;
            case (uint32_t)kCLOCK_OutputFlexspiClk:
                freq = CLOCK_GetClockRootFreq(kCLOCK_FlexspiClkRoot);
                break;
            case (uint32_t)kCLOCK_OutputUartClk:
                freq = CLOCK_GetClockRootFreq(kCLOCK_UartClkRoot);
                break;
            case (uint32_t)kCLOCK_OutputSpdif0Clk:
                freq = CLOCK_GetClockRootFreq(kCLOCK_SpdifClkRoot);
                break;
            default:
                /* This branch should never be hit. */
                break;
        }

        freq /= (((tmp32 & CCM_CCOSR_CLKO2_DIV_MASK) >> CCM_CCOSR_CLKO2_DIV_SHIFT) + 1U);
    }
    else
    {
        freq = 0UL;
    }

    return freq;
}