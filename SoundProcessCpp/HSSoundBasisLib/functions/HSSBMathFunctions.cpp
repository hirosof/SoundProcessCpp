//	HSSBCommon.hpp内部で以下をインクルードしている
//		cmath
//		Windows.h
// 以下はこのファイル内部でインクルードしている
//		HSSBCommon.hpp
#include "../headers/HSSBMathFunctions.hpp" 

// 当初は、 std::numeric_limitsの使用を検討したが、
// Windows.h 内でminとmaxがマクロ定義されているため、
// 競合を避けるためにlimits.hを直接使用することにした。
#include <limits.h>


HSSOUNDBASISLIB_FUNCEXPORT double  HSSBMath_UnNormalizedSinc( double v ) {
	if ( v == 0.0 ) {
		return 1.0;
	} else {
		return sin( v ) / v;
	}
}

HSSOUNDBASISLIB_FUNCEXPORT double  HSSBMath_NormalizedSinc( double v ) {
	return HSSBMath_UnNormalizedSinc( v * M_PI );
}


HSSOUNDBASISLIB_FUNCEXPORT bool  HSSBMathSupporter_CheckValidUInt64ToDouble( uint64_t value ) {
	double converted = static_cast<double>( value );
	uint64_t reverted = static_cast<uint64_t>( converted );
	return ( reverted == value );
}

HSSOUNDBASISLIB_FUNCEXPORT HRESULT HSSBMath64_CalculateSamplesBySeconds_Strict( uint64_t* pSamples, uint32_t sampling_frequency, double number_of_seconds, EHSSB_RoundMode  round_mode  ){

	// 引数チェック
	if ( pSamples == nullptr ) {
		return E_POINTER;
	}

	// 初期化
	*pSamples = 0;

	// サンプリング周波数が0、または秒数が負の値の場合、パラメータ不正としてエラーを返す
	if ( sampling_frequency == 0 || number_of_seconds < 0.0 ) {
		return E_INVALIDARG;
	}

	// 実際のサンプル数を計算
	double real_samples = number_of_seconds * sampling_frequency;

	// オーバーフローする場合、エラーを返す
	if ( real_samples > _UI64_MAX ) {
		return HRESULT_FROM_WIN32( ERROR_ARITHMETIC_OVERFLOW );
	}

	// 小数部分と整数部分に分離して丸め処理の必要性を確認する
	double fractional_part;
	double integer_part;
	fractional_part = modf( real_samples, &integer_part );

	// 小数部分が非常に小さい場合、小数部分は0とみなし、丸め処理は不要と判断する
	if ( fractional_part < DBL_EPSILON ) {
		*pSamples = static_cast<uint64_t>( integer_part );
		return S_OK;
	}

	// 丸め処理
	double adjusted_value;
	switch ( round_mode ) {
		case EHSSB_RoundMode::Up:
			// 切り上げ
			adjusted_value = integer_part + 1.0;
			break;
		case EHSSB_RoundMode::Down:
			// 切り捨て
			adjusted_value = integer_part;
			break;
		case EHSSB_RoundMode::Nearest:
			// 四捨五入
			if ( fractional_part < 0.5 ) {
				adjusted_value = integer_part;
			} else {
				adjusted_value = integer_part + 1.0;
			}
			break;
		default:
			// 不明なモードの場合、切り捨てと同じ扱いとする
			adjusted_value = integer_part;
			break;
	}

	// 調整結果がオーバーフローする場合もエラーを返す
	if ( adjusted_value > _UI64_MAX ) {
		return HRESULT_FROM_WIN32( ERROR_ARITHMETIC_OVERFLOW );
	}

	// 調整後の値を返す
	*pSamples = static_cast<uint64_t>( adjusted_value );

	return S_OK;
}

HSSOUNDBASISLIB_FUNCEXPORT uint64_t HSSBMath64_CalculateSamplesBySeconds( uint32_t sampling_frequency, double number_of_seconds, EHSSB_RoundMode round_mode ) {

	// 結果格納用変数
	uint64_t samples = 0;

	// 内部関数を呼び出して計算
	// 各引数の妥当性チェックは内部関数で行っているため、ここでは省略する
	HRESULT hr = HSSBMath64_CalculateSamplesBySeconds_Strict( &samples, sampling_frequency, number_of_seconds, round_mode );
	if ( FAILED( hr ) ) {
		// エラー発生時は0を返す
		return 0;
	}

	// 計算結果を返す
	return samples;
}

HSSOUNDBASISLIB_FUNCEXPORT HRESULT HSSBMath32_CalculateSamplesBySeconds_Strict( uint32_t* pSamples, uint32_t sampling_frequency, double number_of_seconds, EHSSB_RoundMode round_mode ) {

	uint64_t samples_64bit;


	// 64ビット版の関数を呼び出して計算
	// 各引数の妥当性チェックは内部関数で行っているため、ここでは省略する
	HRESULT hr = HSSBMath64_CalculateSamplesBySeconds_Strict( &samples_64bit, sampling_frequency, number_of_seconds, round_mode );

	if ( FAILED( hr ) ) {
		// エラー発生時はそのまま返す
		return hr;
	}

	// 32ビットに収まらない場合、オーバーフローエラーを返す
	if ( samples_64bit > _UI32_MAX ) {
		return HRESULT_FROM_WIN32( ERROR_ARITHMETIC_OVERFLOW );
	}

	// 計算結果を返す
	*pSamples = static_cast<uint32_t>( samples_64bit );
	return S_OK;
}

HSSOUNDBASISLIB_FUNCEXPORT uint32_t HSSBMath32_CalculateSamplesBySeconds( uint32_t sampling_frequency, double number_of_seconds, EHSSB_RoundMode round_mode ) {
	// 結果格納用変数
	uint32_t samples = 0;

	// 内部関数を呼び出して計算
	// 各引数の妥当性チェックは内部関数で行っているため、ここでは省略する
	HRESULT hr = HSSBMath32_CalculateSamplesBySeconds_Strict( &samples, sampling_frequency, number_of_seconds, round_mode );
	if ( FAILED( hr ) ) {
		// エラー発生時は0を返す
		return 0;
	}

	// 計算結果を返す
	return samples;
}


HSSOUNDBASISLIB_FUNCEXPORT HRESULT HSSBMath_CalculateBytesBySamples_Strict( size_t* pBytesSize, uint64_t samples, uint8_t channels, uint16_t bits_per_sample ) {

	// NULLポインターチェック
	if ( !pBytesSize ) {
		return E_POINTER;
	}

	// 入力値のチェック
	if ( samples == 0 || channels == 0 || bits_per_sample == 0 ) {
		return E_INVALIDARG;
	}

	// 量子化ビット数をバイト数に変換（切り上げ）
	// 計算結果は16ビットに収まるが、計算中のオーバーフロー対策のため、32ビット型を経由する
	uint16_t bytes_per_sample = static_cast<uint16_t> ( ( static_cast<uint32_t>( bits_per_sample ) + 7 ) / 8 );

	// 1サンプルあたりのブロックアラインを計算
	// 以下の観点でここでの計算はオーバーフローしないと考えられる
	//		まず、bytes_per_sample (uint16_t : 16ビット) * channels (uint8_t : 8ビット) = 最大24ビットである
	//		それに対し、size_tは環境によって変わるが32ビットまたは64ビットであるため十分な認識
	size_t blockalign = channels * bytes_per_sample;

	if ( samples <= ( static_cast<uint64_t>(SIZE_MAX) / blockalign ) ){

		// 一旦、64ビットで計算
		uint64_t total_bytes = samples * blockalign;

		// 安全のため、size_tに収まるか再チェック
		if ( total_bytes <= SIZE_MAX ) {
			*pBytesSize = static_cast<size_t>( total_bytes );
			return S_OK;
		} else {
			// オーバーフローする場合、エラーを返す
			return HRESULT_FROM_WIN32( ERROR_ARITHMETIC_OVERFLOW );
		}

	} else {
		// オーバーフローする場合、エラーを返す
		return HRESULT_FROM_WIN32( ERROR_ARITHMETIC_OVERFLOW );
	}

}

