#pragma once

#include "HSSBCommon.hpp"



HSSOUNDBASISLIB_FUNCEXPORT double  HSSBMath_UnNormalizedSinc( double v );
HSSOUNDBASISLIB_FUNCEXPORT double  HSSBMath_NormalizedSinc( double v );


HSSOUNDBASISLIB_FUNCEXPORT bool  HSSBMathSupporter_CheckValidUInt64ToDouble( uint64_t value );

HSSOUNDBASISLIB_FUNCEXPORT HRESULT HSSBMath64_CalculateSamplesBySeconds_Strict( uint64_t* pSamples, uint32_t sampling_frequency, double number_of_seconds, EHSSB_RoundMode  round_mode = EHSSB_RoundMode::Down );
HSSOUNDBASISLIB_FUNCEXPORT uint64_t HSSBMath64_CalculateSamplesBySeconds( uint32_t sampling_frequency, double number_of_seconds, EHSSB_RoundMode  round_mode = EHSSB_RoundMode::Down );
HSSOUNDBASISLIB_FUNCEXPORT HRESULT HSSBMath32_CalculateSamplesBySeconds_Strict( uint32_t* pSamples, uint32_t sampling_frequency, double number_of_seconds, EHSSB_RoundMode  round_mode = EHSSB_RoundMode::Down );
HSSOUNDBASISLIB_FUNCEXPORT uint32_t HSSBMath32_CalculateSamplesBySeconds( uint32_t sampling_frequency, double number_of_seconds, EHSSB_RoundMode  round_mode = EHSSB_RoundMode::Down );


HSSOUNDBASISLIB_FUNCEXPORT HRESULT HSSBMath_CalculateBytesBySamples_Strict( size_t* pBytesSize, uint64_t samples, uint8_t channels, uint16_t bits_per_sample );


template<typename T> HRESULT HSSBMath_CalculateBytesBySamples_StrictType( size_t *pBytesSize ,  uint64_t samples, uint8_t channels ) {

	// 内部でビット数を計算して呼び出す
	//		sizeof(T) はバイト数なので、8倍してビット数に変換してから渡している (bits_per_sampleはTのサイズに準拠させる)
	//			ただし、sizeof(T) が非常に大きい型 (例: 128ビット = 16バイト) の場合、 bits_per_sample が uint16_t の範囲を超える可能性があるが、
	//			現実的にはそのような型は音声データの量子化ビット数として使用されることはないと考えられるため、ここでは特別なチェックは行っていない。
	//		pBytesSize などの値は内部関数側でチェックしているのでここでのチェックは省略する
	return HSSBMath_CalculateBytesBySamples_Strict( pBytesSize, samples, channels, static_cast<uint16_t>( sizeof( T ) * 8 ) );
}
