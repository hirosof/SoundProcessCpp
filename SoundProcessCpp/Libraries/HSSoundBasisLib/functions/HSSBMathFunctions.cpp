//	HSSBCommon.hpp内部で以下をインクルードしている
//		cmath
//		Windows.h
// 以下はこのファイル内部でインクルードしている
//		HSSBCommon.hpp
#include "../headers/HSSBMathFunctions.hpp" 

HSSOUNDBASISLIB_FUNCEXPORT double  HSSBMath_UnnormalizedSinc( double v ) {
	if ( v == 0.0 ) {
		return 1.0;
	} else {
		return sin( v ) / v;
	}
}

HSSOUNDBASISLIB_FUNCEXPORT double  HSSBMath_NormalizedSinc( double v ) {
	return HSSBMath_UnnormalizedSinc( v * M_PI );
}


