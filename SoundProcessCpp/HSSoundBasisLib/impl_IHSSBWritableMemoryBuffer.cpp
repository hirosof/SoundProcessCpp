#include "impl_IHSSBWritableMemoryBuffer.hpp"
#include <new> // std::nothrow

impl_IHSSBWritableMemoryBuffer::impl_IHSSBWritableMemoryBuffer( void* pBuffer, size_t size )
	: m_ref(1)
	, m_pBuffer( static_cast<uint8_t*>( pBuffer ) )
	, m_BufferSize( size )
{
}

impl_IHSSBWritableMemoryBuffer::~impl_IHSSBWritableMemoryBuffer( ) {
}

HRESULT impl_IHSSBWritableMemoryBuffer::CreateInstance( IHSSBWritableMemoryBuffer** ppInstance, void* pBuffer, size_t size ) {
	if ( !ppInstance ) return E_POINTER;
	if ( !pBuffer ) return E_POINTER;
	if ( size == 0 ) return E_INVALIDARG;

	*ppInstance = nullptr;

	auto instance = new (std::nothrow) impl_IHSSBWritableMemoryBuffer( pBuffer, size );
	if ( instance == nullptr ) {
		return E_OUTOFMEMORY;
	}

	*ppInstance = instance;
	return S_OK;
}

bool impl_IHSSBWritableMemoryBuffer::InquiryProvided( REFIID TargetIID ) const {
	IID provided_iids[] = {
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

HRESULT __stdcall impl_IHSSBWritableMemoryBuffer::QueryInterface( REFIID riid, _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject ) {
	if ( !ppvObject ) return E_POINTER;
	*ppvObject = nullptr;

	// 標準 COM パターン: 要求された IID ごとに適切なインターフェースポインタを返す
	if ( IsEqualIID( riid, IID_IHSSBWritableMemoryBuffer ) ) {
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

ULONG __stdcall impl_IHSSBWritableMemoryBuffer::AddRef( void ) {
	return static_cast<ULONG>( InterlockedIncrement( &m_ref ) );
}

ULONG __stdcall impl_IHSSBWritableMemoryBuffer::Release( void ) {
	LONG newVal = InterlockedDecrement( &m_ref );
	if ( newVal == 0 ) {
		delete this;
		return 0;
	}
	return static_cast<ULONG>( newVal );
}

bool impl_IHSSBWritableMemoryBuffer::InquiryProvidedExtraService( REFIID TargetIID ) const {
	return false;
}

HRESULT impl_IHSSBWritableMemoryBuffer::QueryExtraService( REFIID riid, void** ppvObject ) {
	if ( ppvObject ) *ppvObject = nullptr;
	return E_NOTIMPL;
}

size_t impl_IHSSBWritableMemoryBuffer::GetSize( void ) const {
	return this->m_BufferSize;
}

bool impl_IHSSBWritableMemoryBuffer::IsValidElementNumber( size_t offset ) const {
	return ( offset < this->m_BufferSize );
}

const void* impl_IHSSBWritableMemoryBuffer::GetConstBufferPointer( size_t offset ) const {
	return this->GetBufferPointer( offset );
}

void* impl_IHSSBWritableMemoryBuffer::GetBufferPointer( size_t offset ) const {
	if ( this->IsValidElementNumber( offset ) ) {
		return this->m_pBuffer + offset;
	}
	return nullptr;
}

HRESULT impl_IHSSBWritableMemoryBuffer::CheckValidElementNumberRange( size_t offset, size_t length ) const {
	return E_NOTIMPL;
}

HRESULT impl_IHSSBWritableMemoryBuffer::CheckValidElementNumberRangeOffset( size_t start_offset, size_t end_offset ) const {
	return E_NOTIMPL;
}