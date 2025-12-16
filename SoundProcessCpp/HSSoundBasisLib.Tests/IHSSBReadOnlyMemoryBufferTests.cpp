#include "pch.h"
#include "../HSSoundBasisLib/HSSoundBasisLib.hpp"
#include <cstdint>
#include <vector>
#include <cstring>

TEST( HSSB_ReadOnlyBuffer, CreateReadOnlyMemoryBuffer_RawBytes_Succeeds ) {
    std::vector<uint8_t> data = { 10, 20, 30, 40 };
    IHSSBReadOnlyMemoryBuffer* pBuf = nullptr;

    HRESULT hr = HSSBCreateReadOnlyMemoryBuffer( &pBuf, data.data( ), data.size( ) );
    ASSERT_EQ( hr, S_OK );
    ASSERT_NE( pBuf, nullptr );

    EXPECT_EQ( pBuf->GetSize( ), data.size( ) );

    const uint8_t* ptr = pBuf->GetConstBufferPointerType<uint8_t>( 0 );
    ASSERT_NE( ptr, nullptr );
    EXPECT_EQ( 0, std::memcmp( ptr, data.data( ), data.size( ) ) );

    EXPECT_TRUE( pBuf->IsValidElementNumber( 0 ) );
    EXPECT_TRUE( pBuf->IsValidElementNumber( data.size( ) - 1 ) );
    EXPECT_FALSE( pBuf->IsValidElementNumber( data.size( ) ) );

    pBuf->Release( );
}

TEST( HSSB_ReadOnlyBuffer, CreateReadOnlyMemoryBufferType_ArrayAndCount_Succeeds ) {
    uint16_t arr[] = { 0x1111, 0x2222, 0x3333 };
    IHSSBReadOnlyMemoryBuffer* pBuf = nullptr;

    HRESULT hr = HSSBCreateReadOnlyMemoryBufferType<uint16_t>( &pBuf, arr, 3 );
    ASSERT_EQ( hr, S_OK );
    ASSERT_NE( pBuf, nullptr );

    EXPECT_EQ( pBuf->GetSize( ), sizeof( arr ) );

    const uint16_t* ptr = pBuf->GetConstBufferPointerType<uint16_t>( 0 );
    ASSERT_NE( ptr, nullptr );
    EXPECT_EQ( 0, std::memcmp( ptr, arr, sizeof( arr ) ) );

    EXPECT_TRUE( pBuf->IsValidElementNumberType<uint16_t>( 0 ) );
    EXPECT_TRUE( pBuf->IsValidElementNumberType<uint16_t>( 2 ) );
    EXPECT_FALSE( pBuf->IsValidElementNumberType<uint16_t>( 3 ) );

    pBuf->Release( );
}

TEST( HSSB_ReadOnlyBuffer, CreateReadOnlyMemoryBufferType_ArrayReference_Succeeds ) {
    int arr[] = { 1, 2, 3, 4, 5 };
    IHSSBReadOnlyMemoryBuffer* pBuf = nullptr;

    HRESULT hr = HSSBCreateReadOnlyMemoryBufferType( &pBuf, arr );
    ASSERT_EQ( hr, S_OK );
    ASSERT_NE( pBuf, nullptr );

    EXPECT_EQ( pBuf->GetSize( ), sizeof( arr ) );

    const int* ptr = pBuf->GetConstBufferPointerType<int>( 0 );
    ASSERT_NE( ptr, nullptr );
    EXPECT_EQ( 0, std::memcmp( ptr, arr, sizeof( arr ) ) );

    pBuf->Release( );
}

TEST( HSSB_ReadOnlyBuffer, CreateReadOnlyMemoryBuffer_NullOutputParameter_Fails ) {
    uint8_t data[] = { 0xAA };
    // 出力引数が nullptr の場合は実装依存だが、HRESULT が失敗コードであることを期待する
    HRESULT hr = HSSBCreateReadOnlyMemoryBuffer( nullptr, data, sizeof( data ) );
    EXPECT_TRUE( FAILED( hr ) );
}

TEST( HSSB_ReadOnlyBuffer, InquiryProvided_And_QueryInterface_Behavior ) {
    uint8_t data[] = { 1, 2, 3 };
    IHSSBReadOnlyMemoryBuffer* pBuf = nullptr;
    ASSERT_EQ( HSSBCreateReadOnlyMemoryBuffer( &pBuf, data, sizeof( data ) ), S_OK );
    ASSERT_NE( pBuf, nullptr );

    // ReadOnly インターフェイスは提供されているはず
    EXPECT_TRUE( pBuf->InquiryProvided( IID_IHSSBReadOnlyMemoryBuffer ) );
    // 継承元のバッファ基底インターフェイスも提供されるはず
    EXPECT_TRUE( pBuf->InquiryProvided( IID_IHSSBMemoryBufferBase ) );
    // 書き込み用インターフェイスは提供されないはず（読み取り専用）
    EXPECT_FALSE( pBuf->InquiryProvided( IID_IHSSBWritableMemoryBuffer ) );

    // IUnknown（COM）の QueryInterface が成功してポインタが戻ること
    void* pUnknown = nullptr;
    HRESULT hr = pBuf->QueryInterface( __uuidof( IUnknown ), &pUnknown );
    EXPECT_EQ( hr, S_OK );
    ASSERT_NE( pUnknown, nullptr );
    IUnknown* pUnk = reinterpret_cast<IUnknown*>( pUnknown );
    // QueryInterface は参照カウントを増やすはずなので Release しておく
    pUnk->Release();

    // 無効な/未実装の IID に対する QueryInterface は失敗であること
    void* pOut = nullptr;
    hr = pBuf->QueryInterface( IID_IHSSBWritableMemoryBuffer, &pOut );
    // 実装に拠るが、E_NOINTERFACE か失敗コードであることを期待
    EXPECT_TRUE( FAILED( hr ) );
    EXPECT_EQ( pOut, nullptr );

    pBuf->Release();
}

TEST( HSSB_ReadOnlyBuffer, AddRef_Release_BasicChecks ) {
    uint8_t data[] = { 9, 8, 7 };
    IHSSBReadOnlyMemoryBuffer* pBuf = nullptr;
    ASSERT_EQ( HSSBCreateReadOnlyMemoryBuffer( &pBuf, data, sizeof( data ) ), S_OK );
    ASSERT_NE( pBuf, nullptr );

    // AddRef/Release は 0 以外の値を返すこと（COM 的な戻り値チェック）
    ULONG newCount = pBuf->AddRef();
    EXPECT_GT( newCount, 0u );

    ULONG afterRelease = pBuf->Release();
    EXPECT_GT( afterRelease, 0u );

    // 最終的に作成時の参照を解放してオブジェクトを破棄
    pBuf->Release();
}


TEST( HSSB_ReadOnlyBuffer, CreateReadOnlyMemoryBuffer_NullDataPointer_Fails ) {
    IHSSBReadOnlyMemoryBuffer* pBuf = nullptr;
    // データポインタが nullptr の場合は多くの実装で失敗を期待する
    HRESULT hr = HSSBCreateReadOnlyMemoryBuffer( &pBuf, nullptr, 4 );
    EXPECT_TRUE( FAILED( hr ) );
    // 実装によっては pBuf が null のままであるはず
    EXPECT_EQ( pBuf, nullptr );
}

TEST( HSSB_ReadOnlyBuffer, CreateReadOnlyMemoryBuffer_ZeroSize_Behavior ) {
    uint8_t dummy = 0;
    IHSSBReadOnlyMemoryBuffer* pBuf = nullptr;
    HRESULT hr = HSSBCreateReadOnlyMemoryBuffer( &pBuf, &dummy, 0 );
    // 実装に依るが、成功してサイズ 0 のオブジェクトを返すことが望ましい
    ASSERT_EQ( hr, E_INVALIDARG );
    ASSERT_EQ( pBuf, nullptr );
}

TEST( HSSB_ReadOnlyBuffer, GetConstBufferPointer_WithOffset_ReturnsCorrectPointer ) {
    std::vector<uint8_t> data = { 11, 22, 33, 44, 55 };
    IHSSBReadOnlyMemoryBuffer* pBuf = nullptr;
    ASSERT_EQ( HSSBCreateReadOnlyMemoryBuffer( &pBuf, data.data( ), data.size( ) ), S_OK );
    ASSERT_NE( pBuf, nullptr );

    if ( pBuf ) {
        // オフセット 2 の先頭を指すこと
        const uint8_t* ptr = pBuf->GetConstBufferPointerType<uint8_t>( 2 );
        ASSERT_NE( ptr, nullptr );
        EXPECT_EQ( 0, std::memcmp( ptr, data.data( ) + 2, data.size( ) - 2 ) );

        // 範囲外アクセスは nullptr を返すこと
        const uint8_t* ptrOut = pBuf->GetConstBufferPointerType<uint8_t>( data.size( ) );
        EXPECT_EQ( ptrOut, nullptr );

        pBuf->Release( );
    }
}

TEST( HSSB_ReadOnlyBuffer, IsValidElementNumberType_MismatchedSize ) {
    // バイト長が uint32_t の倍数ではないケース
    uint8_t raw[] = { 1, 2, 3 }; // 3 bytes
    IHSSBReadOnlyMemoryBuffer* pBuf = nullptr;
    ASSERT_EQ( HSSBCreateReadOnlyMemoryBuffer( &pBuf, raw, sizeof( raw ) ), S_OK );
    ASSERT_NE( pBuf, nullptr );

    // uint32_t の要素数としては 0 個（部分的に足りない）は無効
    EXPECT_FALSE( pBuf->IsValidElementNumberType<uint32_t>( 0 ) );

    // ただしバイト単位の IsValidElementNumber は 0 は有効（サイズ=3 > 0）
    EXPECT_TRUE( pBuf->IsValidElementNumber( 0 ) );

    pBuf->Release( );
}

TEST( HSSB_ReadOnlyBuffer, QueryInterface_NullOutParam_Fails ) {
    uint8_t data[] = { 9, 9, 9 };
    IHSSBReadOnlyMemoryBuffer* pBuf = nullptr;
    ASSERT_EQ( HSSBCreateReadOnlyMemoryBuffer( &pBuf, data, sizeof( data ) ), S_OK );
    ASSERT_NE( pBuf, nullptr );

    // 出力ポインタが nullptr の場合は失敗（安全性）のことが多い

    if ( pBuf ) {

        HRESULT hr = pBuf->QueryInterface( __uuidof( IUnknown ), nullptr );
        EXPECT_TRUE( FAILED( hr ) );

        pBuf->Release( );
    }
}