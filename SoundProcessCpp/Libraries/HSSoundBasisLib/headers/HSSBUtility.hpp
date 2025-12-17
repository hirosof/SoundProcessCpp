#pragma once
#include "HSSBCommon.hpp"

#include <limits>

HSSOUNDBASISLIB_FUNCEXPORT bool  HSSBUtility_CheckValidUInt64ToDouble( uint64_t value );


template <typename T> T HSSBUtility_GetTolerance( T real_value ) {
    // 絶対値を取得
    T abs_value = ( real_value >= static_cast<T>( 0 ) ) ? real_value : -real_value;
    // 絶対値が1.0未満の場合、許容誤差は機械イプシロンとする
    if ( abs_value < static_cast<T>( 1.0 ) ) {
        return std::numeric_limits<T>::epsilon( );
    }
    // 絶対値が1.0以上の場合、許容誤差は絶対値に機械イプシロンを乗じた値とする
    return abs_value * std::numeric_limits<T>::epsilon( );
}
