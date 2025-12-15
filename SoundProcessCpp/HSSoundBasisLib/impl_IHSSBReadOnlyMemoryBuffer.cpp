#include "impl_IHSSBReadOnlyMemoryBuffer.hpp"
#include <new>

impl_IHSSBReadOnlyMemoryBuffer::impl_IHSSBReadOnlyMemoryBuffer(void* pBuffer, size_t size ) {
	m_ref = 1;
	m_pBuffer = static_cast<uint8_t*>(pBuffer);
	m_BufferSize = size;
	m_OwnershipType = EHSSBMemoryOwnershipType::NoOwnership;
	m_OwnershipTypeInfo = EHSSBMemoryNewAllocatedTypeInfo::None;
}

impl_IHSSBReadOnlyMemoryBuffer::~impl_IHSSBReadOnlyMemoryBuffer( ) {

	// 所有権がある場合はメモリを解放
	switch ( m_OwnershipType ) {
	case EHSSBMemoryOwnershipType::WithDeleteArrayOwnership_NewAllocated:
		// new[] で確保されたメモリの場合、専用の解放関数を呼び出す
		this->FreeForNewAllocatedBuffer( );
		break;
	case EHSSBMemoryOwnershipType::WithFreeOwnership_Malloced:
		// malloc で確保されたメモリの場合、 free を呼び出す
		free( m_pBuffer );
		break;
	case EHSSBMemoryOwnershipType::WithHeapFreeOwnership_HeapAlloced:
		// HeapAlloc で確保されたメモリの場合、 HeapFree を呼び出す
		HeapFree( GetProcessHeap( ), 0, m_pBuffer );
		break;
	case EHSSBMemoryOwnershipType::NoOwnership:
		// 所有権なしの場合は何もしない
		break;
	default:
		// 不明な所有権タイプの場合も何もしない (所有権なしと同様に扱う)
		break;
	}

}

void impl_IHSSBReadOnlyMemoryBuffer::FreeForNewAllocatedBuffer( void ) {
	
	// デストラクタから呼び出されることを想定しているため、チェックは最小限に留める

	if ( m_OwnershipType != EHSSBMemoryOwnershipType::WithDeleteArrayOwnership_NewAllocated ) {
		// 所有権タイプが new[] で確保されたメモリではない場合は何もしない
		return;
	}

	// 型情報に基づいて適切な型で解放を行う
	switch ( m_OwnershipTypeInfo ) {
		case EHSSBMemoryNewAllocatedTypeInfo::char_array:
			this->FreeForNewAllocatedBufferInternal<char>( );
			break;
		case EHSSBMemoryNewAllocatedTypeInfo::wchar_t_array:
			this->FreeForNewAllocatedBufferInternal<wchar_t>( );
			break;
		case EHSSBMemoryNewAllocatedTypeInfo::float_array:
			this->FreeForNewAllocatedBufferInternal<float>( );
			break;
		case EHSSBMemoryNewAllocatedTypeInfo::double_array:
			this->FreeForNewAllocatedBufferInternal<double>( );
			break;
		case EHSSBMemoryNewAllocatedTypeInfo::int8_t_array:
			this->FreeForNewAllocatedBufferInternal<int8_t>( );
			break;
		case EHSSBMemoryNewAllocatedTypeInfo::int16_t_array:
			this->FreeForNewAllocatedBufferInternal<int16_t>( );
			break;
		case EHSSBMemoryNewAllocatedTypeInfo::int32_t_array:
			this->FreeForNewAllocatedBufferInternal<int32_t>( );
			break;
		case EHSSBMemoryNewAllocatedTypeInfo::int64_t_array:
			this->FreeForNewAllocatedBufferInternal<int64_t>( );
			break;
		case EHSSBMemoryNewAllocatedTypeInfo::uint8_t_array:
			this->FreeForNewAllocatedBufferInternal<uint8_t>( );
			break;
		case EHSSBMemoryNewAllocatedTypeInfo::uint16_t_array:
			this->FreeForNewAllocatedBufferInternal<uint16_t>( );
			break;
		case EHSSBMemoryNewAllocatedTypeInfo::uint32_t_array:
			this->FreeForNewAllocatedBufferInternal<uint32_t>( );
			break;
		case EHSSBMemoryNewAllocatedTypeInfo::uint64_t_array:
			this->FreeForNewAllocatedBufferInternal<uint64_t>( );
			break;
		default:
			// 不明な型情報の場合は何もしない
			break;
	}

}

HRESULT impl_IHSSBReadOnlyMemoryBuffer::CreateInstance(IHSSBReadOnlyMemoryBuffer** ppInstance, void* pBuffer, size_t size ) {
	//もう一つのCreateInstanceを呼び出すだけ
	// 所有権なしで作成 ( 所有権ありで作成する場合は、そちらのCreateInstanceを直接呼び出すこと)
	// パラメータチェックはそちらで行われる
	return impl_IHSSBReadOnlyMemoryBuffer::CreateInstance( ppInstance, pBuffer, size, EHSSBMemoryOwnershipType::NoOwnership );	
}

HRESULT impl_IHSSBReadOnlyMemoryBuffer::CreateInstance( IHSSBReadOnlyMemoryBuffer** ppInstance, void* pBuffer, size_t size, EHSSBMemoryOwnershipType owner, EHSSBMemoryNewAllocatedTypeInfo owner_type_info ) {

	// パラメータチェック
	if ( !ppInstance ) return E_POINTER;
	if ( !pBuffer ) return E_POINTER;

	// owner の有効性をチェック
	switch ( owner ) {
		case EHSSBMemoryOwnershipType::NoOwnership:
		case EHSSBMemoryOwnershipType::WithDeleteArrayOwnership_NewAllocated:
		case EHSSBMemoryOwnershipType::WithFreeOwnership_Malloced:
		case EHSSBMemoryOwnershipType::WithHeapFreeOwnership_HeapAlloced:
			// 有効な値
			break;
		default:
			// 無効な値
			return E_INVALIDARG;
	}

	// 成功時における期待されるHRESULTを設定
	HRESULT Expect_hr_for_Success = S_OK;

	// 所有権タイプに応じたサイズチェックと調整
	if ( owner == EHSSBMemoryOwnershipType::WithHeapFreeOwnership_HeapAlloced ) {

		// HeapAlloc で確保されたメモリの場合、実際のサイズの取得を試行する
		SIZE_T heap_size = HeapSize( GetProcessHeap( ), 0, pBuffer );

		if ( ( size == 0 ) && ( heap_size == (SIZE_T) ( -1 ) ) ) {
			// サイズの取得に失敗した場合はエラー (指定サイズが 0 の場合)
			return E_INVALIDARG;
		}

		if ( heap_size == 0 ) {
			// 実際のサイズが 0 の場合はエラー
			return E_INVALIDARG;
		}

		// 実際のサイズの取得に成功した場合、サイズのチェックが可能なので、
		// 取得したサイズを使ってサイズ調整の必要があるか確認する
		if ( heap_size != (SIZE_T) ( -1 ) ) {

			// サイズ調整の必要があるか確認
			// size が 0 または 実際のサイズより大きい場合、調整を行う
			// size が 実際のサイズ以下の場合は調整不要 (意図したサイズとして扱う)
			if ( ( size == 0 ) || ( size > heap_size ) ) {

				// 実際のサイズの取得に成功したため、サイズを設定して続行する
				size = static_cast<size_t>( heap_size );

				// サイズ調整が発生したため、期待される成功コードを変更する
				Expect_hr_for_Success = HSSB_S_OK_BUT_MANAGED_SIZE_ADJUSTED;
			}
		}

	} else {
		// HeapAlloc 以外の場合、サイズが 0 は無効
		if ( size == 0 ) {
			return E_INVALIDARG;
		}
	}

	// 出力ポインタを nullptr に初期化
	*ppInstance = nullptr;

	if ( owner == EHSSBMemoryOwnershipType::WithDeleteArrayOwnership_NewAllocated ) {
		// 所有権タイプが new[] で確保されたメモリの場合、型情報が指定されていることを確認
		if ( owner_type_info == EHSSBMemoryNewAllocatedTypeInfo::None ) {
			// 型情報が指定されていない場合はエラー
			return E_INVALIDARG;
		}
	} else {
		// 所有権タイプが new[] 以外の場合、型情報は None に設定する
		owner_type_info = EHSSBMemoryNewAllocatedTypeInfo::None;
	}

	// 非例外版 new（nothrow）を使用してエラーハンドリング
	impl_IHSSBReadOnlyMemoryBuffer* inst = new ( std::nothrow ) impl_IHSSBReadOnlyMemoryBuffer( pBuffer, size );
	if ( !inst ) {
		// インスタンス作成失敗 (メモリ確保失敗)
		return E_OUTOFMEMORY;
	}

	// 成功したら出力ポインタに設定
	*ppInstance = inst;

	// 所有権タイプを設定
	inst->m_OwnershipType = owner;

	// 所有権タイプ情報を設定
	inst->m_OwnershipTypeInfo = owner_type_info;

	return Expect_hr_for_Success;
}

bool impl_IHSSBReadOnlyMemoryBuffer::InquiryProvided( REFIID TargetIID ) const {

	// 提供しているインターフェイスの IID 一覧
	IID provided_iids[] = { 
		IID_IHSSBReadOnlyMemoryBuffer,
		IID_IHSSBMemoryBufferBase,
		IID_IHSSBMemoryProvider,
		IID_IHSSBBase,
		IID_IUnknown	
	};

	// TargetIID が提供している IID 一覧に含まれているか確認
	for ( const IID& current : provided_iids ) {
		if ( IsEqualIID(current, TargetIID) ) {
			return true;
		}
	}
	return  false;
}

HRESULT __stdcall impl_IHSSBReadOnlyMemoryBuffer::QueryInterface( REFIID riid, _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject ) {
	if ( !ppvObject )  return E_POINTER;

	*ppvObject = nullptr;

	// InquiryProvided を使ってインターフェイスが提供されているか確認
	if ( this->InquiryProvided( riid ) ) {
		// 提供されている場合は、IUnknown 経由でポインタを取得し、AddRef する
		// このキャストは、多重継承階層の基点となる IUnknown を示すために安全です
		*ppvObject = static_cast<IUnknown*>(static_cast<IHSSBReadOnlyMemoryBuffer*>(this));
		this->AddRef();
		return S_OK;
	}

	return E_NOINTERFACE;
}

ULONG __stdcall impl_IHSSBReadOnlyMemoryBuffer::AddRef( void ) {
	return static_cast<ULONG>( InterlockedIncrement( &m_ref ) );
}

ULONG __stdcall impl_IHSSBReadOnlyMemoryBuffer::Release( void ) {
	LONG newCount = InterlockedDecrement( &m_ref );
	if ( newCount == 0 ) {
		delete this;
		return 0;
	}
	return static_cast<ULONG>( newCount );
}

bool impl_IHSSBReadOnlyMemoryBuffer::InquiryProvidedExtraService( REFIID TargetIID ) const {
	// 追加サービスは提供していない
	return false;
}

HRESULT impl_IHSSBReadOnlyMemoryBuffer::QueryExtraService( REFIID riid, void** ppvObject ) {
	// 追加サービスは提供していない
	if ( ppvObject ) *ppvObject = nullptr;
	return E_NOTIMPL;
}

size_t impl_IHSSBReadOnlyMemoryBuffer::GetSize( void ) const {
	// バッファーサイズを返す
	return this->m_BufferSize;
}

bool impl_IHSSBReadOnlyMemoryBuffer::IsValidElementNumber( size_t offset ) const {
	// オフセットがバッファーサイズ未満であれば有効
	return ( offset < this->m_BufferSize);
}

const void* impl_IHSSBReadOnlyMemoryBuffer::GetConstBufferPointer( size_t offset ) const {
	// オフセットが有効ならポインタを返す
	if ( this->IsValidElementNumber( offset ) ) {
		return this->m_pBuffer + offset;
	}
	return nullptr;
}

HRESULT impl_IHSSBReadOnlyMemoryBuffer::CheckValidElementNumberRange( size_t offset, size_t length ) const {
	if ( length == 0 ) {
		// 長さが0の場合は無効
		return E_INVALIDARG;
	}

	if ( length > SIZE_MAX - offset ) {
		// オフセットと長さの和がサイズ上限を超える場合は無効
		return E_INVALIDARG;
	}

	// 範囲の終端オフセットを計算して、範囲チェック用関数を呼び出す
	return this->CheckValidElementNumberRangeOffset(offset , offset + length -1);
}

HRESULT impl_IHSSBReadOnlyMemoryBuffer::CheckValidElementNumberRangeOffset( size_t start_offset, size_t end_offset ) const {


	if ( start_offset > end_offset ) {
		// 開始オフセットが終端オフセットより大きい場合は無効
		return E_INVALIDARG;
	}

	if(this->IsValidElementNumber( start_offset ) && this->IsValidElementNumber( end_offset ) ) {
		// 両端のオフセットが有効なら成功
		return S_OK;
	}

	// いずれかのオフセットが無効ならエラー
	return E_INVALIDARG;
}
