/* Alternate pin name */
PA_0_ALT1  = PA_0  | ALT1,
PA_0_ALT2  = PA_0  | ALT2,
PA_1_ALT1  = PA_1  | ALT1,
PA_1_ALT2  = PA_1  | ALT2,
PA_2_ALT1  = PA_2  | ALT1,
PA_2_ALT2  = PA_2  | ALT2,
PA_3_ALT1  = PA_3  | ALT1,
PA_3_ALT2  = PA_3  | ALT2,
PA_4_ALT1  = PA_4  | ALT1,
PA_4_ALT2  = PA_4  | ALT2,
PA_5_ALT1  = PA_5  | ALT1,
PA_5_ALT2  = PA_5  | ALT2,
PA_6_ALT1  = PA_6  | ALT1,
PA_7_ALT1  = PA_7  | ALT1,
PA_7_ALT2  = PA_7  | ALT2,
PA_7_ALT3  = PA_7  | ALT3,
PA_8_ALT1  = PA_8  | ALT1,
PA_8_ALT2  = PA_8  | ALT2,
PA_8_ALT3  = PA_8  | ALT3,
PA_8_ALT4  = PA_8  | ALT4,
PA_8_ALT5  = PA_8  | ALT5,
PA_15_ALT1 = PA_15 | ALT1,
PB_3_ALT1  = PB_3  | ALT1,
PB_3_ALT2  = PB_3  | ALT2,
PB_5_ALT1  = PB_5  | ALT1,
PB_6_ALT1  = PB_6  | ALT1,
PB_6_ALT2  = PB_6  | ALT2,
PB_6_ALT3  = PB_6  | ALT3,
PB_6_ALT4  = PB_6  | ALT4,
PB_6_ALT5  = PB_6  | ALT5,
PB_7_ALT1  = PB_7  | ALT1,
PB_7_ALT2  = PB_7  | ALT2,
PB_7_ALT3  = PB_7  | ALT3,
PB_7_ALT4  = PB_7  | ALT4,
PB_8_ALT1  = PB_8  | ALT1,
PC_14_ALT1 = PC_14 | ALT1,

/* SYS_WKUP */
#ifdef PWR_WAKEUP_PIN1
  SYS_WKUP1 = PA_0,
#endif
#ifdef PWR_WAKEUP_PIN2
  SYS_WKUP2 = PA_4,
#endif
#ifdef PWR_WAKEUP_PIN3
  SYS_WKUP3 = PB_6,
#endif
#ifdef PWR_WAKEUP_PIN4
  SYS_WKUP4 = PA_2,
#endif
#ifdef PWR_WAKEUP_PIN5
  SYS_WKUP5 = NC,
#endif
#ifdef PWR_WAKEUP_PIN6
  SYS_WKUP6 = PB_5,
#endif
#ifdef PWR_WAKEUP_PIN7
  SYS_WKUP7 = NC,
#endif
#ifdef PWR_WAKEUP_PIN8
  SYS_WKUP8 = NC,
#endif

/* USB */
#ifdef USBCON
  USB_DM  = PA_11,
  USB_DP  = PA_12,
  #ifdef USB_NOE_PA_4
    USB_NOE = PA_4,
  #endif
  #ifdef USB_NOE_PA_13
    USB_NOE = PA_13,
  #endif
  #ifdef USB_NOE_PA_15
    USB_NOE = PA_15,
  #endif
#endif
