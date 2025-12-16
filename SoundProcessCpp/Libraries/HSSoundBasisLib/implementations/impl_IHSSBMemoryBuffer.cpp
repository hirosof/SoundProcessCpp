#include "impl_IHSSBMemoryBuffer.hpp"
#include <new>      // std::nothrow
#include <Windows.h>

impl_IHSSBMemoryBuffer::impl_IHSSBMemoryBuffer( )
	: m_ref(1)
	, m_pBuffer( nullptr )
	, m_BufferSize( 0 )
{
}

impl_IHSSBMemoryBuffer::~impl_IHSSBMemoryBuffer( ) {
	if ( this->m_pBuffer ) {
		HeapFree( GetProcessHeap(), 0, this->m_pBuffer );
		this->m_pBuffer = nullptr;
		this->m_BufferSize = 0;
	}
}

HRESULT impl_IHSSBMemoryBuffer::CreateInstance( IHSSBMemoryBuffer** ppInstance ) {
	if ( !ppInstance ) return E_POINTER;
	*ppInstance = nullptr;

	auto instance = new (std::nothrow) impl_IHSSBMemoryBuffer( );
	if ( instance == nullptr ) {
		return E_OUTOFMEMORY;
	}

	*ppInstance = instance;
	return S_OK;
}

bool impl_IHSSBMemoryBuffer::InquiryProvided( REFIID TargetIID ) const {
	IID provided_iids[] = {
		IID_IHSSBMemoryBuffer,
		IID_IHSSBWritableMemoryBuffer,
		IID_IHSSBReadOnlyMemoryBuffer,
		IID_IHSSBMemoryBufferBase,
		IID_IHSSBMemoryProvider,
		IID_IHSSBBase,
	};

	for ( const IID& current : provided_iids ) {
		if ( IsEqualIID( current, TargetIID ) ) {
			return true;
		}
	}
	return false;
}

HRESULT __stdcall impl_IHSSBMemoryBuffer::QueryInterface( REFIID riid, _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject ) {
	if ( !ppvObject ) return E_POINTER;
	*ppvObject = nullptr;

	if ( IsEqualIID( riid, IID_IHSSBMemoryBuffer ) ) {
		*ppvObject = static_cast<IHSSBMemoryBuffer*>( this );
	} else if ( IsEqualIID( riid, IID_IHSSBWritableMemoryBuffer ) ) {
		*ppvObject = static_cast<IHSSBWritableMemoryBuffer*>( this );
	} else if ( IsEqualIID( riid, IID_IHSSBReadOnlyMemoryBuffer ) ) {
		*ppvObject = static_cast<IHSSBReadOnlyMemoryBuffer*>( this );
	} else if ( IsEqualIID( riid, IID_IHSSBMemoryBufferBase ) ) {
		*ppvObject = static_cast<IHSSBMemoryBufferBase*>( this );
	} else if ( IsEqualIID( riid, IID_IHSSBMemoryProvider ) ) {
		*ppvObject = static_cast<IHSSBMemoryProvider*>( this );
	} else if ( IsEqualIID( riid, IID_IHSSBBase ) ) {
		*ppvObject = static_cast<IHSSBBase*>( this );
	} else if ( IsEqualIID( riid, IID_IUnknown ) ) {
		*ppvObject = static_cast<IUnknown*>( this );
	} else {
		return E_NOINTERFACE;
	}

	this->AddRef();
	return S_OK;
}

ULONG __stdcall impl_IHSSBMemoryBuffer::AddRef( void ) {
	return static_cast<ULONG>( InterlockedIncrement( &m_ref ) );
}

ULONG __stdcall impl_IHSSBMemoryBuffer::Release( void ) {
	LONG newVal = InterlockedDecrement( &m_ref );
	if ( newVal == 0 ) {
		delete this;
		return 0;
	}
	return static_cast<ULONG>( newVal );
}

bool impl_IHSSBMemoryBuffer::InquiryProvidedExtraService( REFIID TargetIID ) const {
	return false;
}

HRESULT impl_IHSSBMemoryBuffer::QueryExtraService( REFIID riid, void** ppvObject ) {
	if ( ppvObject ) *ppvObject = nullptr;
	return E_NOTIMPL;
}

bool impl_IHSSBMemoryBuffer::IsAllocated( void ) const {
	return (this->m_pBuffer != nullptr);
}

size_t impl_IHSSBMemoryBuffer::GetSize( void ) const {
	return this->m_BufferSize;
}

bool impl_IHSSBMemoryBuffer::IsValidElementNumber( size_t offset ) const {
	return ( offset < this->m_BufferSize );
}

const void* impl_IHSSBMemoryBuffer::GetConstBufferPointer( size_t offset ) const {
	return this->GetBufferPointer( offset );
}

void* impl_IHSSBMemoryBuffer::GetBufferPointer( size_t offset ) const {
	if ( this->IsValidElementNumber( offset ) ) {
		return this->m_pBuffer + offset;
	}
	return nullptr;
}

HRESULT impl_IHSSBMemoryBuffer::Allocate( size_t size ) {
	if ( size == 0 ) return E_INVALIDARG;

	// 既存バッファがあれば解放してから割当て
	if ( this->m_pBuffer ) {
		HeapFree( GetProcessHeap(), 0, this->m_pBuffer );
		this->m_pBuffer = nullptr;
		this->m_BufferSize = 0;
	}

	void* p = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, size );
	if ( p == nullptr ) {
		return E_OUTOFMEMORY;
	}

	this->m_pBuffer = static_cast<uint8_t*>( p );
	this->m_BufferSize = size;
	return S_OK;
}

HRESULT impl_IHSSBMemoryBuffer::Free( void ) {
	if ( this->m_pBuffer ) {
		if ( !HeapFree( GetProcessHeap(), 0, this->m_pBuffer ) ) {
			// HeapFree が失敗した場合でもポインタは無効とする
			this->m_pBuffer = nullptr;
			this->m_BufferSize = 0;
			return HRESULT_FROM_WIN32( GetLastError() );
		}
		this->m_pBuffer = nullptr;
		this->m_BufferSize = 0;
	}
	return S_OK;
}

HRESULT impl_IHSSBMemoryBuffer::ReAllocate( size_t new_size ) {
	if ( new_size == 0 ) return E_INVALIDARG;

	// 未割当てなら Allocate と同じ
	if ( this->m_pBuffer == nullptr ) {
		return this->Allocate( new_size );
	}

	void* p = HeapReAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, this->m_pBuffer, new_size );
	if ( p == nullptr ) {
		// 再割当てに失敗した場合、既存バッファはそのまま残る（HeapReAlloc の仕様）
		return E_OUTOFMEMORY;
	}

	this->m_pBuffer = static_cast<uint8_t*>( p );
	this->m_BufferSize = new_size;
	return S_OK;
}

HRESULT impl_IHSSBMemoryBuffer::Prepare( size_t size, bool enable_reduce_allocate ) {
	if ( size == 0 ) return E_INVALIDARG;

	if ( this->m_BufferSize >= size ) {
		if ( enable_reduce_allocate && this->m_BufferSize != size ) {
			return this->ReAllocate( size );
		}
		return S_FALSE;
	}

	return this->ReAllocate( size );
}

HRESULT impl_IHSSBMemoryBuffer::CheckValidElementNumberRange( size_t offset, size_t length ) const {
	return E_NOTIMPL;
}

HRESULT impl_IHSSBMemoryBuffer::CheckValidElementNumberRangeOffset( size_t start_offset, size_t end_offset ) const {
	return E_NOTIMPL;
}