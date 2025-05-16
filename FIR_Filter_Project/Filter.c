/**********************************************************************
* File: Filter.c
* Description: Contains IQmath filter for C28x workshop labs
* Devices: TMS320F2812, TMS320F2811, TMS320F2810
* Author: Technical Training Organization (TTO), Texas Instruments
* Function List:
*   _iq fir(_iq *x, _iq *a, Uint16 n)
* History:
*   11/10/03 - original (based on DSP281x header files v1.00)
*                        and IQmath v1.4d)
* Notes: none
**********************************************************************/
#include "DSP2833x_Device.h"
#include "IQmathLib.h"


/**********************************************************************
* Function: IQssfir()
* Description: IQmath n-tap single-sample FIR filter.
*
*         y(k) = a(0)*x(k) + a(1)*x(k-1) + ... + a(n-1)*x(k-n+1)
*
* DSP: TMS320F2812, TMS320F2811, TMS320F2810
* Include files: DSP281x_Device.h, IQmathLib.h
* Function Prototype: _iq IQssfir(_iq*, _iq*, Uint16)
* Useage: y = IQssfir(x, a, n);
* Input Parameters: x = array of input and delay chain (global IQ format)
*                   a = array of coefficients (global IQ format)
*                   n = number of coefficients
* Return Value: y = result
* Notes:
* 1) This is just a simple filter example, and completely unoptimized.
*    The goal with the code was clarity and simplicity, not efficiency.
* 2) The filtering is done from last tap to first tap.  This allows
*    more efficient delay chain updating.
**********************************************************************/
_iq IQssfir(_iq *x, _iq *a, Uint16 n)
{

Uint16 i;								// general purpose
_iq y;									// result
_iq *xold;								// delay line pointer

/*** Setup the pointers ***/
	a = a + (n-1);						// a points to last coefficient
	x = x + (n-1);						// x points to last buffer element
	xold = x;							// xold points to last buffer element

/*** Last tap has no delay line update ***/
	y = _IQmpy(*a--, *x--);

/*** Do the other taps from end to beginning ***/
	for(i=0; i<n-1; i++)
	{
		y = y + _IQmpy(*a--, *x);		// filter tap
		*xold-- = *x--;					// delay line update
	}

/*** Finish up ***/
	return(y);

} //end IQssfir()


/*** end of file *****************************************************/
