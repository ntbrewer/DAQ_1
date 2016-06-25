/*
   This is a file to define the labjack error messages so that they
   can be included rather than copied from program to program.
*/

char *errormsg[] =
   {
     "no msg code",   //0
	"SCRATCH_WRT_FAIL",   //1	
	"SCRATCH_ERASE_FAIL",   //2	
	"DATA_BUFFER_OVERFLOW",   //3	
	"ADC0_BUFFER_OVERFLOW",   //4
	"FUNCTION_INVALID",   //5	
	"SWDT_TIME_INVALID",   //6	
	"XBR_CONFIG_ERROR",   //7	
        "no msg code",   //8
        "no msg code",   //9
        "no msg code",   //10
        "no msg code",   //11
        "no msg code",   //12
        "no msg code",   //13
        "no msg code",   //14
        "no msg code",   //15
	"FLASH_WRITE_FAIL",   //	16
	"FLASH_ERASE_FAIL",   //	17
	"FLASH_JMP_FAIL",   //	18
	"FLASH_PSP_TIMEOUT",   //19	
	"FLASH_ABORT_RECEIVED",   //20	
	"FLASH_PAGE_MISMATCH",   //21	
	"FLASH_BLOCK_MISMATCH",   //22	
	"FLASH_PAGE_NOT_IN_CODE_AREA",   //23	
	"MEM_ILLEGAL_ADDRESS",   //24	
	"FLASH_LOCKED",   //	25
	"INVALID_BLOCK",   //	26
	"FLASH_ILLEGAL_PAGE",   //27	
	"FLASH_TOO_MANY_BYTES",   //28	
	"FLASH_INVALID_STRING_NUM",   //29	
        "no msg code",   //30
        "no msg code",   //31
        "no msg code",   //32
        "no msg code",   //33
        "no msg code",   //34
        "no msg code",   //35
        "no msg code",   //36
        "no msg code",   //37
        "no msg code",   //38
        "no msg code",   //39
	"SHT1x_COMM_TIME_OUT",   //40	
	"SHT1x_NO_ACK",   //	41
	"SHT1x_CRC_FAILED",   //	42
	"SHT1X_TOO_MANY_W_BYTES",   //43	
	"SHT1X_TOO_MANY_R_BYTES",   //44	
	"SHT1X_INVALID_MODE",   //45	
	"SHT1X_INVALID_LINE",   //46
        "no msg code",   //47	
	"STREAM_IS_ACTIVE",   //	48
	"STREAM_TABLE_INVALID",   //	49
	"STREAM_CONFIG_INVALID",   //	50
	"STREAM_BAD_TRIGGER_SOURCE",   //51	
	"STREAM_NOT_RUNNING",   //	52
	"STREAM_INVALID_TRIGGER",   //	53
	"STREAM_ADC0_BUFFER_OVERFLOW",   //54	
	"STREAM_SCAN_OVERLAP",   //	55
	"STREAM_SAMPLE_NUM_INVALID",   //56	
	"STREAM_BIPOLAR_GAIN_INVALID",   //57	
	"STREAM_SCAN_RATE_INVALID",   //	58
	"STREAM_AUTORECOVER_ACTIVE",   //59	
	"STREAM_AUTORECOVER_REPORT",   //60
        "no msg code",   //61	
        "no msg code",   //62	
	"STREAM_AUTORECOVER_OVERFLOW",   //63	
	"TIMER_INVALID_MODE",   //	64
	"TIMER_QUADRATURE_AB_ERROR",   //65	
	"TIMER_QUAD_PULSE_SEQUENCE",   //66	
	"TIMER_BAD_CLOCK_SOURCE",   //67
	"TIMER_STREAM_ACTIVE",   //68	
	"TIMER_PWMSTOP_MODULE_ERROR",   //69	
	"TIMER_SEQUENCE_ERROR",   //70	
	"TIMER_LINE_SEQUENCE_ERROR",   //71	
	"TIMER_SHARING_ERROR",   //72	
        "no msg code",   //73
        "no msg code",   //74
        "no msg code",   //75
        "no msg code",   //76
        "no msg code",   //77
        "no msg code",   //78
        "no msg code",   //79
	"EXT_OSC_NOT_STABLE",   //80	
	"INVALID_POWER_SETTING",   //81	
	"PLL_NOT_LOCKED",   //82	
        "no msg code",   //83
        "no msg code",   //84
        "no msg code",   //85
        "no msg code",   //86
        "no msg code",   //87
        "no msg code",   //88
        "no msg code",   //89
        "no msg code",   //90
        "no msg code",   //91
        "no msg code",   //92
        "no msg code",   //93
        "no msg code",   //94
        "no msg code",   //95
	"INVALID_PIN",   //96	
	"PIN_CONFIGURED_FOR_ANALOG",   //97	
	"PIN_CONFIGURED_FOR_DIGITAL",   //98	
	"IOTYPE_SYNCH_ERROR",   //99	
	"INVALID_OFFSET",   //100
	"IOTYPE_NOT_VALID",   //101	
	"TC_PIN_OFFSET_MUST_BE_4-8",   //102	
        "no msg code",   //103
        "no msg code",   //104
        "no msg code",   //105
        "no msg code",   //106
        "no msg code",   //107
        "no msg code",   //108
        "no msg code",   //109
        "no msg code",   //110
        "no msg code",   //111
	"UART_TIMEOUT",   //112	
	"UART_NOT_CONNECTED",   //113	
	"UART_NOT_ENABLED",   //114	
   };

