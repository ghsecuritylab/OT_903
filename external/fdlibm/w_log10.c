
/* @(#)w_log10.c 1.3 95/01/18 */


#include "fdlibm.h"


#ifdef __STDC__
	double ieee_log10(double x)		/* wrapper log10 */
#else
	double ieee_log10(x)			/* wrapper log10 */
	double x;
#endif
{
#ifdef _IEEE_LIBM
	return __ieee754_log10(x);
#else
	double z;
	z = __ieee754_log10(x);
	if(_LIB_VERSION == _IEEE_ || ieee_isnan(x)) return z;
	if(x<=0.0) {
	    if(x==0.0)
	        return __kernel_standard(x,x,18); /* ieee_log10(0) */
	    else 
	        return __kernel_standard(x,x,19); /* ieee_log10(x<0) */
	} else
	    return z;
#endif
}
