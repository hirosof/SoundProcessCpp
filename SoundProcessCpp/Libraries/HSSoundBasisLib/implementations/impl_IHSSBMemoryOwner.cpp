#include "impl_IHSSBMemoryOwner.hpp"
#include <new>
#include <atlbase.h>  // For CComPtr


impl_IHSSBMemoryOwner::impl_IHSSBMemoryOwner( ) : m_ref(1)  {
	// 初期化
	this->m_BufferSize = 0;
	this->m_pBuffer = nullptr;
	this->m_OwnershipType = EHSSBMemoryOwnershipType::NoOwnership;
	this->m_OwnershipTypeInfo = EHSSBMemoryNewAllocatedTypeInfo::None;

}

impl_IHSSBMemoryOwner::~impl_IHSSBMemoryOwner( ) {
	this->Free( );

}

void impl_IHSSBMemoryOwner::FreeForNewAllocatedBuffer( void ) {

	// Freeから呼び出されることを想定しているため、チェックは最小限に留める
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

HRESULT impl_IHSSBMemoryOwner::CreateInstance( IHSSBMemoryOwner** ppInstance ) {

	if ( !ppInstance ) {
		return E_POINTER;
	}

	*ppInstance = new ( std::nothrow ) impl_IHSSBMemoryOwner( );

	if ( !*ppInstance ) {
		return E_OUTOFMEMORY;
	}

	return S_OK;
}


HRESULT impl_IHSSBMemoryOwner::CreateInstance( IHSSBMemoryOwner** ppInstance, void* pBuffer, size_t size, EHSSBMemoryOwnershipType owner, EHSSBMemoryNewAllocatedTypeInfo owner_type_info ) {

	if ( !ppInstance ) {
		return E_POINTER;
	}

	CComPtr<IHSSBMemoryOwner> spInstance;
	
	// もう一つの CreateInstance を呼び出してインスタンスを作成
	HRESULT hr = impl_IHSSBMemoryOwner::CreateInstance( &spInstance );

	//インスタンス作成失敗時はエラーを返す
	if ( FAILED( hr ) ) {
		return hr;
	}

	// バッファーをアタッチ (パラメータチェックは Attach 内で行われる)
	HRESULT hr_attach = spInstance->Attach( pBuffer, size, owner, owner_type_info );
	if ( FAILED( hr_attach ) ) {
		return hr_attach;
	}

    // 成功したら出力ポインタに設定
    hr = spInstance.QueryInterface( ppInstance );
    if ( FAILED( hr ) ) {
        return hr;
    }

    return hr_attach;
}

bool impl_IHSSBMemoryOwner::InquiryProvided( REFIID TargetIID ) const {
	// 提供しているインターフェイスの IID 一覧

	IID provided_iids[] = {
		IID_IHSSBMemoryOwner,
		IID_IHSSBMemoryProvider,
		IID_IHSSBBase,
		IID_IUnknown
	};


	for ( const IID& iid : provided_iids ) {
		if ( IsEqualIID( iid, TargetIID ) ) {
			return true;
		}
	}

    return false;
}

HRESULT __stdcall impl_IHSSBMemoryOwner::QueryInterface( REFIID riid, _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject ) {

	// パラメータチェック
	if ( !ppvObject ) {
		return E_POINTER;
	}

	// 初期化
	*ppvObject = nullptr;

	// InquiryProvided を使ってインターフェイスが提供されているか確認
	if ( InquiryProvided( riid ) == false ) {
		return E_NOINTERFACE;
	}

	// 提供されている場合は、IUnknown 経由でポインタを取得し、AddRef する
	*ppvObject = static_cast<IUnknown*>( static_cast<IHSSBMemoryOwner*>( this ) );
	
	// AddRef を呼び出す
	this->AddRef( );

	// 成功
	return S_OK;

}

ULONG __stdcall impl_IHSSBMemoryOwner::AddRef( void ) {
	// 参照カウントをインクリメントして新しい値を返す
	return static_cast<ULONG>( InterlockedIncrement( &m_ref ) );
}

ULONG __stdcall impl_IHSSBMemoryOwner::Release( void ) {

	// 参照カウントをデクリメント
	LONG newCount = InterlockedDecrement( &m_ref );
	if ( newCount == 0 ) {
		// 参照カウントが 0 になった場合は自身を削除
		delete this;
		return 0;
	}
	// 新しい参照カウントを返す
	return static_cast<ULONG>( newCount );
}

bool impl_IHSSBMemoryOwner::InquiryProvidedExtraService( REFIID TargetIID ) const {
	// このクラスは追加サービスを提供していないため、常に false を返す
	return false;
}

HRESULT __stdcall impl_IHSSBMemoryOwner::QueryExtraService( REFIID riid, void** ppvObject ) {
	// このクラスは追加サービスを提供していないため、常に E_NOTIMPL を返す
	return E_NOTIMPL;
}

HRESULT impl_IHSSBMemoryOwner::Attach( void* pBuffer, size_t size, EHSSBMemoryOwnershipType owner, EHSSBMemoryNewAllocatedTypeInfo owner_type_info ) {

    std::lock_guard<std::mutex> lock( m_mutex );

	if ( this->IsAttached()) {
		// すでにバッファーがアタッチされている場合はエラー
		return E_FAIL;
	}

	if ( !pBuffer ) {
		// バッファーポインタが nullptr の場合はエラー
		return E_POINTER;
	}

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

        if ( heap_size == (SIZE_T) ( -1 ) ) {
            // 実際のサイズの取得に失敗した場合はエラー
            return E_INVALIDARG;
        }

		if ( heap_size == 0 ) {
			// 実際のサイズが 0 の場合はエラー
			return E_INVALIDARG;
		}

		// 実際のサイズの取得に成功した場合、サイズのチェックが可能なので、
		// 取得したサイズを使ってサイズ調整の必要があるか確認する

		// size が 0 または 実際のサイズより大きい場合、調整を行う
		// size が 実際のサイズ以下の場合は調整不要 (意図したサイズとして扱う)
		if ( ( size == 0 ) || ( size > heap_size ) ) {

			// 実際のサイズを管理サイズに設定して続行する
			size = static_cast<size_t>( heap_size );

			// サイズ調整が発生したため、期待される成功コードを変更する
			Expect_hr_for_Success = HSSB_S_OK_BUT_MANAGED_SIZE_ADJUSTED;
		} else {
			// size が 実際のサイズ以下の場合は調整不要 (意図したサイズとして扱う)
		}


    } else {
		// HeapAlloc 以外の場合、サイズが 0 は無効
		if ( size == 0 ) {
			return E_INVALIDARG;
		}
	}

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

	// アタッチ処理
	this->m_pBuffer = reinterpret_cast<uint8_t*>( pBuffer );
	this->m_BufferSize = size;
	this->m_OwnershipType = owner;
	this->m_OwnershipTypeInfo = owner_type_info;

	// 成功時の期待される HRESULT を返す
	return Expect_hr_for_Success;
}

HRESULT impl_IHSSBMemoryOwner::Detach( void** ppOutBuffer, size_t* pOutSize, EHSSBMemoryOwnershipType* pOutOwner, EHSSBMemoryNewAllocatedTypeInfo* pOutOwnerTypeInfo ) {

    std::lock_guard<std::mutex> lock( m_mutex );


    if ( !this->IsAttached( ) ) {
        // バッファーがアタッチされていない場合はエラー
        return E_FAIL;
    }

    if ( !ppOutBuffer ) {
        // 出力ポインタが nullptr の場合はエラー
        return E_POINTER;
    }

	// 出力ポインタに値を設定
	*ppOutBuffer = reinterpret_cast<void*>( this->m_pBuffer );

	// サイズ、所有権タイプ、所有権タイプ情報も必要に応じて設定
	// 各種、ポインタが 有効な 項目にのみ設定

	// サイズ
	if ( pOutSize ) {
		*pOutSize = this->m_BufferSize;
	}

	// 所有権タイプ
	if ( pOutOwner ) {
		*pOutOwner = this->m_OwnershipType;
	}

	// 所有権タイプ情報
	if ( pOutOwnerTypeInfo ) {
		*pOutOwnerTypeInfo = this->m_OwnershipTypeInfo;
	}

	// 内部状態をクリア
	this->m_pBuffer = nullptr;
	this->m_BufferSize = 0;
	this->m_OwnershipType = EHSSBMemoryOwnershipType::NoOwnership;
	this->m_OwnershipTypeInfo = EHSSBMemoryNewAllocatedTypeInfo::None;
	return S_OK;
}

HRESULT impl_IHSSBMemoryOwner::Free( void ) {

    std::lock_guard<std::mutex> lock( m_mutex );


	if (!this->IsAttached() ) {
		// バッファーがアタッチされていない場合は何もしないで成功を返す
		return S_OK;
	}

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
			if(!HeapFree( GetProcessHeap( ), 0, m_pBuffer )) {
                // HeapFree に失敗した場合はエラーを返す
                return HRESULT_FROM_WIN32(GetLastError());
            }
			break;
		case EHSSBMemoryOwnershipType::NoOwnership:
			// 所有権なしの場合は何もしない
			break;
		default:
			// 不明な所有権タイプの場合も何もしない (所有権なしと同様に扱う)
			break;
	}

	// 内部状態をクリア
	this->m_pBuffer = nullptr;
	this->m_BufferSize = 0;
	this->m_OwnershipType = EHSSBMemoryOwnershipType::NoOwnership;
	this->m_OwnershipTypeInfo = EHSSBMemoryNewAllocatedTypeInfo::None;
	return S_OK;
}

bool impl_IHSSBMemoryOwner::IsAttached( void ) const {
    return  ( this->m_BufferSize != 0 ) && ( this->m_pBuffer != nullptr );
}

void* impl_IHSSBMemoryOwner::GetBufferPointer( void ) const {
    if ( !this->IsAttached( ) ) {
        return nullptr;
    }
	return this->m_pBuffer;
}

size_t impl_IHSSBMemoryOwner::GetSize( void ) const {
    if ( !this->IsAttached( ) ) {
        return 0;
    }
    return this->m_BufferSize;
}

EHSSBMemoryOwnershipType impl_IHSSBMemoryOwner::GetOwnershipType( void ) const {
    if ( !this->IsAttached( ) ) {
        return EHSSBMemoryOwnershipType::NoOwnership;
    }

	return this->m_OwnershipType;
}

EHSSBMemoryNewAllocatedTypeInfo impl_IHSSBMemoryOwner::GetOwnershipTypeInfo( void ) const {
    if ( !this->IsAttached( ) ) {
        return EHSSBMemoryNewAllocatedTypeInfo::None;
    }
    return this->m_OwnershipTypeInfo;
}
