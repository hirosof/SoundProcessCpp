#include "impl_IHSSBReadOnlyMemoryBuffer.hpp"
#include <new>

impl_IHSSBReadOnlyMemoryBuffer::impl_IHSSBReadOnlyMemoryBuffer(const void* pBuffer, size_t size ) {
	m_ref = 1;
	m_pBuffer = static_cast<const uint8_t*>(pBuffer);
	m_BufferSize = size;
}

impl_IHSSBReadOnlyMemoryBuffer::~impl_IHSSBReadOnlyMemoryBuffer( ) {
}

HRESULT impl_IHSSBReadOnlyMemoryBuffer::CreateInstance(IHSSBReadOnlyMemoryBuffer** ppInstance, const void* pBuffer, size_t size ) {
	if ( !ppInstance ) return E_POINTER;
	if ( !pBuffer ) return E_POINTER;
	if ( size == 0 ) return E_INVALIDARG;

	*ppInstance = nullptr;

	// 非例外版 new（nothrow）を使用してエラーハンドリング
	impl_IHSSBReadOnlyMemoryBuffer* inst = new (std::nothrow) impl_IHSSBReadOnlyMemoryBuffer( pBuffer, size );
	if ( !inst ) {
		return E_OUTOFMEMORY;
	}

	*ppInstance = inst;
	return S_OK;
}

bool impl_IHSSBReadOnlyMemoryBuffer::InquiryProvided( REFIID TargetIID ) const {
	IID provided_iids[] = { 
		IID_IHSSBReadOnlyMemoryBuffer,
		IID_IHSSBMemoryBufferBase,
		IID_IHSSBMemoryProvider,
		IID_IHSSBBase,
	};

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

	// IID ごとに正しいインターフェイスポインタを返す（ポインタ調整のため static_cast を利用）
	if ( IsEqualIID(riid, IID_IHSSBReadOnlyMemoryBuffer) ) {
		*ppvObject = static_cast<IHSSBReadOnlyMemoryBuffer*>(this);
	}
	else if ( IsEqualIID(riid, IID_IHSSBMemoryBufferBase) ) {
		*ppvObject = static_cast<IHSSBMemoryBufferBase*>(this);
	}
	else if ( IsEqualIID(riid, IID_IHSSBMemoryProvider) ) {
		*ppvObject = static_cast<IHSSBMemoryProvider*>(this);
	}
	else if ( IsEqualIID(riid, IID_IHSSBBase) ) {
		*ppvObject = static_cast<IHSSBBase*>(this);
	}
	else if ( IsEqualIID(riid, IID_IUnknown) ) {
		*ppvObject = static_cast<IUnknown*>(this);
	}
	else {
		return E_NOINTERFACE;
	}

	this->AddRef();
	return S_OK;
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
	return false;
}

HRESULT impl_IHSSBReadOnlyMemoryBuffer::QueryExtraService( REFIID riid, void** ppvObject ) {
	if ( ppvObject ) *ppvObject = nullptr;
	return E_NOTIMPL;
}

size_t impl_IHSSBReadOnlyMemoryBuffer::GetSize( void ) const {
	return this->m_BufferSize;
}

bool impl_IHSSBReadOnlyMemoryBuffer::IsValidElementNumber( size_t offset ) const {
	return ( offset < this->m_BufferSize);
}

const void* impl_IHSSBReadOnlyMemoryBuffer::GetConstBufferPointer( size_t offset ) const {
	if ( this->IsValidElementNumber( offset ) ) {
		return this->m_pBuffer + offset;
	}
	return nullptr;
}

