//  内部で以下のファイルがインクルードされる
//      Windows.h
//      cstdint
//      cstdio
//      math.h (別途_USE_MATH_DEFINESを定義しているため、数学定数も有効化される)
//      cmath (modf関数を使用するため) 
//      (※このファイルが直接インクルードしているわけではなく、別のヘッダーファイルを介してインクルードされる)
#include "../headers/HSSBCalculator.hpp"
#include "../headers/HSSBUtility.hpp"

#include <limits>

HSSOUNDBASISLIB_FUNCEXPORT HRESULT HSSBCalculate64_SamplesBySeconds_Strict( uint64_t* pSamples, uint32_t sampling_frequency, double number_of_seconds, EHSSB_RoundMode  round_mode ) {

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

    // 指定された標本化周波数に基づき、最大秒数を計算
    uint64_t max_seconds = std::numeric_limits<uint64_t>::max( ) / sampling_frequency;

    // 秒数が計算上オーバーフローする場合、エラーを返す
    if ( number_of_seconds > static_cast<double>( max_seconds ) ) {
        return HRESULT_FROM_WIN32( ERROR_ARITHMETIC_OVERFLOW );
    }

    // 実際のサンプル数を計算
    double real_samples = number_of_seconds * sampling_frequency;

    // 許容誤差を取得
    double tolerance = HSSBUtility_GetTolerance( real_samples );

    // 小数部分と整数部分に分離して丸め処理の必要性を確認する
    double fractional_part;
    double integer_part;
    fractional_part = modf( real_samples, &integer_part );

    // 小数部分が非常に小さい場合、小数部分は0とみなし、丸め処理は不要と判断する
    if ( fractional_part < tolerance ) {
        *pSamples = static_cast<uint64_t>( integer_part );
        return S_OK;
    } else if ( ( 1 - fractional_part ) < tolerance ) {
        // 小数部分が1に非常に近い場合、整数部分を1増やして丸め処理は不要と判断する
        *pSamples = static_cast<uint64_t>( integer_part + 1.0 );
        return S_OK;
    }

    // 丸め処理
    // なお、切り捨てた値は integer_part に既に格納されているため、
    // integer_partの値を流用する。
    // (round関数などは不要と判断)
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
            // 小数部分はfractional_partに格納されているため、それを参照して判定する
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
    if ( adjusted_value > std::numeric_limits<uint64_t>::max( ) ) {
        return HRESULT_FROM_WIN32( ERROR_ARITHMETIC_OVERFLOW );
    }

    // 調整後の値を返す
    *pSamples = static_cast<uint64_t>( adjusted_value );

    return S_OK;
}

HSSOUNDBASISLIB_FUNCEXPORT uint64_t HSSBCalculate64_SamplesBySeconds( uint32_t sampling_frequency, double number_of_seconds, EHSSB_RoundMode round_mode ) {

    // 結果格納用変数
    uint64_t samples = 0;

    // 内部関数を呼び出して計算
    // 各引数の妥当性チェックは内部関数で行っているため、ここでは省略する
    HRESULT hr = HSSBCalculate64_SamplesBySeconds_Strict( &samples, sampling_frequency, number_of_seconds, round_mode );
    if ( FAILED( hr ) ) {
        // エラー発生時は0を返す
        return 0;
    }

    // 計算結果を返す
    return samples;
}

HSSOUNDBASISLIB_FUNCEXPORT HRESULT HSSBCalculate32_SamplesBySeconds_Strict( uint32_t* pSamples, uint32_t sampling_frequency, double number_of_seconds, EHSSB_RoundMode round_mode ) {

    uint64_t samples_64bit;


    // 64ビット版の関数を呼び出して計算
    // 各引数の妥当性チェックは内部関数で行っているため、ここでは省略する
    HRESULT hr = HSSBCalculate64_SamplesBySeconds_Strict( &samples_64bit, sampling_frequency, number_of_seconds, round_mode );

    if ( FAILED( hr ) ) {
        // エラー発生時はそのまま返す
        return hr;
    }

    // 32ビットに収まらない場合、オーバーフローエラーを返す
    if ( samples_64bit > std::numeric_limits<uint32_t>::max( ) ) {
        return HRESULT_FROM_WIN32( ERROR_ARITHMETIC_OVERFLOW );
    }

    // 計算結果を返す
    *pSamples = static_cast<uint32_t>( samples_64bit );
    return S_OK;
}

HSSOUNDBASISLIB_FUNCEXPORT uint32_t HSSBCalculate32_SamplesBySeconds( uint32_t sampling_frequency, double number_of_seconds, EHSSB_RoundMode round_mode ) {
    // 結果格納用変数
    uint32_t samples = 0;

    // 内部関数を呼び出して計算
    // 各引数の妥当性チェックは内部関数で行っているため、ここでは省略する
    HRESULT hr = HSSBCalculate32_SamplesBySeconds_Strict( &samples, sampling_frequency, number_of_seconds, round_mode );
    if ( FAILED( hr ) ) {
        // エラー発生時は0を返す
        return 0;
    }

    // 計算結果を返す
    return samples;
}


HSSOUNDBASISLIB_FUNCEXPORT HRESULT HSSBCalculate_BytesBySamples_Strict( size_t* pBytesSize, uint64_t samples, uint8_t channels, uint16_t bits_per_sample ) {

    // NULLポインターチェック
    if ( !pBytesSize ) {
        return E_POINTER;
    }

    // 入力値のチェック
    if (  channels == 0 || bits_per_sample == 0 ) {
        return E_INVALIDARG;
    }

    // サンプル数が0の場合、サイズは0バイト
    if ( samples == 0 ) {
        *pBytesSize = 0;
        return S_OK;
    }

    // size_tの最大値を取得
    constexpr size_t max_size_t = std::numeric_limits<size_t>::max( );

    // 量子化ビット数をバイト数に変換（切り上げ）
    // 計算結果は16ビットに収まるが、計算中のオーバーフロー対策のため、32ビット型を経由する
    uint16_t bytes_per_sample = static_cast<uint16_t> ( ( static_cast<uint32_t>( bits_per_sample ) + 7 ) / 8 );

    // 1サンプルあたりのブロックアラインを計算
    // 以下の観点でここでの計算はオーバーフローしないと考えられる
    //		まず、bytes_per_sample は uint16_t (16ビット)  ,  channels は uint8_t (8ビット) である
    //      16ビット * 8ビット = 24ビット = 16,711,425 (約1,677万) であり、32ビットの範囲内に収まる
    //      (16ビット：65535 (約6.5万) , 8ビット：255, 24ビット：約1,677万)
    //		それに対し、size_tは環境によって変わるが32ビットまたは64ビットであるため十分な認識
    size_t blockalign = static_cast<size_t>(channels) * static_cast<size_t>(bytes_per_sample);

    if ( samples <= ( static_cast<uint64_t>( max_size_t ) / blockalign ) ) {

        // 一旦、64ビットで計算
        uint64_t total_bytes = samples * blockalign;

        // 安全のため、size_tに収まるか再チェック
        if ( total_bytes <= max_size_t ) {
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

