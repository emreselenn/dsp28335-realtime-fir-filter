
#include "DSP2833x_Device.h"
#include "IQmathLib.h"

#define AdcBufLen 50
#define AdcFsVoltage	_IQ(3.0)		// ADC full scale voltage

// external function prototypes
extern void InitSysCtrl(void);
extern void InitPieCtrl(void);
extern void InitPieVectTable(void);
extern void InitCpuTimers(void);
extern void InitAdc(void);
extern void ConfigCpuTimer(struct CPUTIMER_VARS *, float, float);
extern  _iq IQssfir(_iq*, _iq*, Uint16);


// Prototype statements for functions found within this file.
void Gpio_select(void);
void Setup_ePWM1A(void);
interrupt void cpu_timer0_isr(void);
interrupt void adc_isr(void);
void Setup_ADC(void);

// global variables
_iq AdcBuf[AdcBufLen];					// ADC results buffer
_iq AdcBufFiltered[AdcBufLen];			// filtered ADC results buffer
_iq xBuffer[5] = {0,0,0,0,0};			// filter sample buffer
// filter coefficients
_iq coeffs[5] = {_IQ(0.0357), _IQ(0.2411), _IQ(0.4465), _IQ(0.2411), _IQ(0.0357)};

//###########################################################################
//						main code
//###########################################################################
void main(void)
{
	int counter=0;	// binary counter for digital output

	InitSysCtrl();	// Basic Core Init from DSP2833x_SysCtrl.c

	EALLOW;
   	SysCtrlRegs.WDCR= 0x00AF;	// Re-enable the watchdog
   	EDIS;			// 0x00AF  to NOT disable the Watchdog, Prescaler = 64

	DINT;				// Disable all interrupts

	Gpio_select();		// GPIO9, GPIO11, GPIO34 and GPIO49 as output
						// to 4 LEDs at Peripheral Explorer

	Setup_ePWM1A();		// init of ePWM1A

	Setup_ADC();		// initialize ADC channel A2

	InitPieCtrl();		// basic setup of PIE table; from DSP2833x_PieCtrl.c

	InitPieVectTable();	// default ISR's in PIE

	EALLOW;
	PieVectTable.TINT0 = &cpu_timer0_isr;
	PieVectTable.ADCINT = &adc_isr;
	EDIS;

	InitCpuTimers();	// basic setup CPU Timer0, 1 and 2

	ConfigCpuTimer(&CpuTimer0,150,20);	// 50 kHz sample period

	PieCtrlRegs.PIEIER1.bit.INTx7 = 1;	// CPU -Timer 0
	PieCtrlRegs.PIEIER1.bit.INTx6 = 1;	// ADC

	IER |=1;

	EINT;
	ERTM;

	CpuTimer0Regs.TCR.bit.TSS = 0;	// start timer0

	while(1)
	{
	  		while(CpuTimer0.InterruptCount < 5000);
			CpuTimer0.InterruptCount = 0;

			EALLOW;
			SysCtrlRegs.WDKEY = 0x55;	// service WD #1
			EDIS;

	  		counter++;
			if(counter&1) GpioDataRegs.GPASET.bit.GPIO9 = 1;
				else GpioDataRegs.GPACLEAR.bit.GPIO9 = 1;
			if(counter&2) GpioDataRegs.GPASET.bit.GPIO11 = 1;
				else GpioDataRegs.GPACLEAR.bit.GPIO11 = 1;
			if(counter&4) GpioDataRegs.GPBSET.bit.GPIO34 = 1;
				else GpioDataRegs.GPBCLEAR.bit.GPIO34 = 1;
			if(counter&8) GpioDataRegs.GPBSET.bit.GPIO49 = 1;
				else GpioDataRegs.GPBCLEAR.bit.GPIO49 = 1;
	}
}

void Gpio_select(void)
{
	EALLOW;
	GpioCtrlRegs.GPAMUX1.all = 0;		// GPIO15 ... GPIO0 = General Puropse I/O
	GpioCtrlRegs.GPAMUX1.bit.GPIO0 = 1;	// ePWM1A active

	GpioCtrlRegs.GPAMUX2.all = 0;		// GPIO31 ... GPIO16 = General Purpose I/O
	GpioCtrlRegs.GPBMUX1.all = 0;		// GPIO47 ... GPIO32 = General Purpose I/O
	GpioCtrlRegs.GPBMUX2.all = 0;		// GPIO63 ... GPIO48 = General Purpose I/O
	GpioCtrlRegs.GPCMUX1.all = 0;		// GPIO79 ... GPIO64 = General Purpose I/O
	GpioCtrlRegs.GPCMUX2.all = 0;		// GPIO87 ... GPIO80 = General Purpose I/O

	GpioCtrlRegs.GPADIR.all = 0;
	GpioCtrlRegs.GPADIR.bit.GPIO9 = 1;	// peripheral explorer: LED LD1 at GPIO9
	GpioCtrlRegs.GPADIR.bit.GPIO11 = 1;	// peripheral explorer: LED LD2 at GPIO11

	GpioCtrlRegs.GPBDIR.all = 0;		// GPIO63-32 as inputs
	GpioCtrlRegs.GPBDIR.bit.GPIO34 = 1;	// peripheral explorer: LED LD3 at GPIO34
	GpioCtrlRegs.GPBDIR.bit.GPIO49 = 1; // peripheral explorer: LED LD4 at GPIO49

	GpioCtrlRegs.GPCDIR.all = 0;		// GPIO87-64 as inputs
	EDIS;
}

void Setup_ePWM1A(void)
{
	EPwm1Regs.TBCTL.bit.CLKDIV =  0;	// CLKDIV = 1
	EPwm1Regs.TBCTL.bit.HSPCLKDIV = 0;	// HSPCLKDIV = 1
	EPwm1Regs.TBCTL.bit.CTRMODE = 2;	// up - down mode

	EPwm1Regs.AQCTLA.all = 0x0060;		// set ePWM1A on CMPA up
										// clear ePWM1A on CMPA down

	EPwm1Regs.TBPRD = 37500;			// 2KHz - PWM signal
	// TBPRD = fCPU / ( 2 * fPWM *CLKDIV * HSPCLKDIV)
	// TBPRD = 150MHz / (2 * 2kHz * 1 *1)
	// TBPRD = 37500
	EPwm1Regs.CMPA.half.CMPA  = 28125;	// 25% duty cycle
	// CMPA = (100% - duty cycle)*TBPRD
	// CMPA = 0.75 * 37500 = 28125
}

interrupt void cpu_timer0_isr(void)
{
	CpuTimer0.InterruptCount++;
	AdcRegs.ADCTRL2.bit.SOC_SEQ1 = 1;	// start ADC by software
	EALLOW;
	SysCtrlRegs.WDKEY = 0xAA;	// service WD #2
	EDIS;

	PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}

void Setup_ADC(void)
{
	InitAdc();
	// Configure ADC
   	AdcRegs.ADCTRL1.bit.SEQ_CASC = 1;	   // Cascaded Sequencer Mode
   	AdcRegs.ADCTRL1.bit.CONT_RUN = 0;	   // No Continuous run
   	AdcRegs.ADCTRL1.bit.CPS = 0;		   // prescaler = 1
   	AdcRegs.ADCTRL2.bit.EPWM_SOCA_SEQ1 = 0;  // Disable EPWM_SOCA to start SEQ1
	AdcRegs.ADCTRL2.bit.INT_ENA_SEQ1 = 1;  // Enable SEQ1 interrupt
	AdcRegs.ADCTRL2.bit.INT_MOD_SEQ1 = 0;  // with every EOS
	AdcRegs.ADCTRL3.bit.ADCCLKPS = 3;	   // Divide HSPCLK by 6
	AdcRegs.ADCMAXCONV.all = 0;		       // 1 Conversion per start
    AdcRegs.ADCCHSELSEQ1.bit.CONV00 = 2;   // Setup ADCINA2 as input channel.
}

interrupt void adc_isr(void)
{
	static Uint16 index=0;                     	    // index into ADC buffers
	AdcBuf[index] = _IQmpy(AdcFsVoltage, _IQ16toIQ((_iq)AdcRegs.ADCRESULT0));

	/*** Call the filter function ***/
	xBuffer[0] = AdcBuf[index];				// Add the new entry to the delay chain
	AdcBufFiltered[index] = IQssfir(xBuffer, coeffs, 5);

	index++;									// Increment the index
	if(index == AdcBufLen) index = 0;			// Rewind the pointer to beginning
	// Reinitialize for next ADC sequence
  	AdcRegs.ADCTRL2.bit.RST_SEQ1 = 1;   	// Reset SEQ1
  	AdcRegs.ADCST.bit.INT_SEQ1_CLR = 1;		// Clear INT SEQ1 bit
  	PieCtrlRegs.PIEACK.all = 1;   			// Acknowledge interrupt to PIE
}
//===========================================================================
// End of SourceCode.
//===========================================================================
