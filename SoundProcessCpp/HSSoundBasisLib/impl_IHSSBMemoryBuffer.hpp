#pragma once
#define HSSOUNDBASISLIB_EXPORT_DLL_INTERNAL
#include "HSSoundBasisLib.hpp"

class impl_IHSSBMemoryBuffer : public IHSSBMemoryBuffer {
private:

	volatile LONG m_ref;
	uint8_t* m_pBuffer;
	size_t m_BufferSize;

	impl_IHSSBMemoryBuffer( );
	~impl_IHSSBMemoryBuffer( );

public:

	static HRESULT CreateInstance( IHSSBMemoryBuffer** ppInstance );

	virtual bool InquiryProvided( REFIID TargetIID ) const;
	virtual HRESULT STDMETHODCALLTYPE QueryInterface( REFIID riid, _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject );
	virtual ULONG STDMETHODCALLTYPE  AddRef( void );
	virtual ULONG STDMETHODCALLTYPE Release( void );

	virtual bool InquiryProvidedExtraService( REFIID TargetIID ) const;
	virtual HRESULT QueryExtraService( REFIID riid, void** ppvObject );

	virtual bool IsAllocated( void ) const;

	virtual size_t GetSize( void ) const;
	virtual bool IsValidElementNumber( size_t offset ) const;
	virtual const void* GetConstBufferPointer( size_t offset = 0 )const;
	virtual void* GetBufferPointer( size_t offset = 0 )const;

	virtual HRESULT Allocate( size_t size );
	virtual HRESULT Free( void );
	virtual HRESULT ReAllocate( size_t new_size );
	virtual HRESULT Prepare( size_t size, bool enable_reduce_allocate = false );
};