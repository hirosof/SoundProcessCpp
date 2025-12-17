#include "../headers/HSSBUtility.hpp"


HSSOUNDBASISLIB_FUNCEXPORT bool  HSSBUtility_CheckValidUInt64ToDouble( uint64_t value ) {
    double converted = static_cast<double>( value );
    uint64_t reverted = static_cast<uint64_t>( converted );
    return ( reverted == value );
}

