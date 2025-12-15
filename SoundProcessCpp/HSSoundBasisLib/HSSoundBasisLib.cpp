#include <Windows.h>
#include "HSSoundBasisLib.hpp"
#include "impl_IHSSBReadOnlyMemoryBuffer.hpp"
#include "impl_IHSSBWritableMemoryBuffer.hpp"
#include "impl_IHSSBMemoryBuffer.hpp"
#include "impl_IHSSBNormalizedPCMBuffer.hpp"
#include "impl_IHSSBMemoryOwner.hpp"


BOOL WINAPI DllMain( HINSTANCE hDLL, DWORD dwReason, LPVOID lpReserved ) {
	switch ( dwReason ) {
	case DLL_PROCESS_ATTACH:
		// スレッドごとの DLL_THREAD_ATTACH/DLL_THREAD_DETACH を受け取らないようにする（パフォーマンス向上）
		DisableThreadLibraryCalls( hDLL );
		break;
	case DLL_PROCESS_DETACH:
		// 必要ならクリーンアップ
		break;
	default:
		break;
	}
	return TRUE;
}

// 各種IID 定義
const IID IID_IHSSBBase = __uuidof( IHSSBBase );
const IID IID_IHSSBMemoryProvider = __uuidof( IHSSBMemoryProvider );

const IID IID_IHSSBMemoryBufferBase = __uuidof( IHSSBMemoryBufferBase );
const IID IID_IHSSBReadOnlyMemoryBuffer = __uuidof( IHSSBReadOnlyMemoryBuffer );
const IID IID_IHSSBWritableMemoryBuffer = __uuidof( IHSSBWritableMemoryBuffer );
const IID IID_IHSSBMemoryBuffer = __uuidof( IHSSBMemoryBuffer );


const IID IID_IHSSBMemoryStreamBase = __uuidof( IHSSBMemoryStreamBase );
const IID IID_IHSSBMemoryStream = __uuidof( IHSSBMemoryStream );

const IID IID_IHSSBMemoryReader = __uuidof( IHSSBMemoryReader );
const IID IID_IHSSBMemoryWriter = __uuidof( IHSSBMemoryWriter );

const IID IID_IHSSBNormalizedPCMBuffer = __uuidof( IHSSBNormalizedPCMBuffer );

const IID IID_IHSSBMemoryOwner = __uuidof( IHSSBMemoryOwner );


// カスタム HRESULT 作成マクロ
#define HSSB_MAKE_CUSTOM_HRESULT(sev , code) MAKE_HRESULT( sev, FACILITY_ITF, 0x2000 +(code))

// カスタム HRESULT 定義

// 初期化されていないことを表すカスタムHRESULTエラーコード
const HRESULT HSSB_E_NOT_INITIALIZED = HSSB_MAKE_CUSTOM_HRESULT( SEVERITY_ERROR, 0 );

//部分的成功を表すカスタムHRESULTコード
const HRESULT HSSB_S_PARTIAL = HSSB_MAKE_CUSTOM_HRESULT( SEVERITY_SUCCESS, 1 );

// 部分的成功：入力側に要因があった場合
const HRESULT HSSB_S_PARTIAL_INPUT_SIDE_FACTOR = HSSB_MAKE_CUSTOM_HRESULT( SEVERITY_SUCCESS, 2 );

// 部分的成功：出力側に要因があった場合
const HRESULT HSSB_S_PARTIAL_OUTPUT_SIDE_FACTOR = HSSB_MAKE_CUSTOM_HRESULT( SEVERITY_SUCCESS, 3 );

// バグ起因とみられる処理エラー表すカスタムHRESULTエラーコード
const HRESULT HSSB_E_PROCESS_ERROR_BY_BUG_FACTOR = HSSB_MAKE_CUSTOM_HRESULT( SEVERITY_ERROR, 4 );

// 処理には成功したが、管理サイズが調整されたことを表すカスタムHRESULTコード
const HRESULT HSSB_S_OK_BUT_MANAGED_SIZE_ADJUSTED = HSSB_MAKE_CUSTOM_HRESULT( SEVERITY_SUCCESS, 5 );


HSSOUNDBASISLIB_FUNCEXPORT HRESULT HSSBCreateMemoryOwner( IHSSBMemoryOwner** ppInstance ) {
	if ( !ppInstance ) return E_INVALIDARG;
	*ppInstance = nullptr;
	return impl_IHSSBMemoryOwner::CreateInstance( ppInstance );
}

HSSOUNDBASISLIB_FUNCEXPORT HRESULT HSSBCreateMemoryOwner( IHSSBMemoryOwner** ppInstance, void* pBuffer, size_t size, EHSSBMemoryOwnershipType owner, EHSSBMemoryNewAllocatedTypeInfo owner_type_info ) {
	if ( !ppInstance ) return E_INVALIDARG;
	*ppInstance = nullptr;
	// pBuffer が NULL の場合、サイズが 0 であることを期待する（要件に応じて調整）
	if ( size != 0 && !pBuffer ) return E_INVALIDARG;
	return impl_IHSSBMemoryOwner::CreateInstance( ppInstance, pBuffer, size, owner, owner_type_info );
}

HSSOUNDBASISLIB_FUNCEXPORT HRESULT HSSBCreateReadOnlyMemoryBuffer( IHSSBReadOnlyMemoryBuffer** ppInstance, void* pBuffer, size_t size, EHSSBMemoryOwnershipType owner, EHSSBMemoryNewAllocatedTypeInfo owner_type_info ) {

	if ( !ppInstance ) return E_INVALIDARG;
	*ppInstance = nullptr;
	// pBuffer が NULL の場合、サイズが 0 であることを期待する（要件に応じて調整）
	if ( size != 0 && !pBuffer ) return E_INVALIDARG;
	return impl_IHSSBReadOnlyMemoryBuffer::CreateInstance( ppInstance, pBuffer, size, owner, owner_type_info );
}

HSSOUNDBASISLIB_FUNCEXPORT HRESULT HSSBCreateWritableMemoryBuffer( IHSSBWritableMemoryBuffer** ppBuffer, void* pTargetBuffer, size_t TargetBufferSize ) {
	if ( !ppBuffer ) return E_INVALIDARG;
	*ppBuffer = nullptr;

	// pTargetBuffer が NULL の場合、サイズが 0 であることを期待する（要件に応じて調整）
	if ( TargetBufferSize != 0 && !pTargetBuffer ) return E_INVALIDARG;

	return impl_IHSSBWritableMemoryBuffer::CreateInstance( ppBuffer, pTargetBuffer, TargetBufferSize );
}


HSSOUNDBASISLIB_FUNCEXPORT HRESULT HSSBCreateMemoryBuffer( IHSSBMemoryBuffer** ppBuffer ) {

	if ( !ppBuffer ) return E_INVALIDARG;

	*ppBuffer = nullptr;

	return impl_IHSSBMemoryBuffer::CreateInstance( ppBuffer );
}


HSSOUNDBASISLIB_FUNCEXPORT HRESULT HSSBCreateMemoryStream( IHSSBMemoryStream** ppBuffer ) {
	if ( !ppBuffer ) return E_INVALIDARG;
	*ppBuffer = nullptr;
	// 未実装: 実装されていないことを明示する
	return E_NOTIMPL;
}

HSSOUNDBASISLIB_FUNCEXPORT HRESULT HSSBCreateNormalizedPCMBuffer( IHSSBNormalizedPCMBuffer** ppBuffer ) {
	if ( !ppBuffer ) return E_INVALIDARG;
	
	*ppBuffer = nullptr;

	return impl_IHSSBNormalizedPCMBuffer::CreateInstance( ppBuffer );
}