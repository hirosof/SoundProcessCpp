#pragma once

#define HSSOUNDBASISLIB_EXPORT_DLL_INTERNAL
#include "HSSoundBasisLib.hpp"

class impl_IHSSBReadOnlyMemoryBuffer : public IHSSBReadOnlyMemoryBuffer {

private:

	// Interlocked 系 API と整合の取れた型に変更（符号付き LONG）
	volatile LONG m_ref;
	const uint8_t* m_pBuffer;
	size_t m_BufferSize;

	impl_IHSSBReadOnlyMemoryBuffer( const void* pBuffer, size_t size );
	~impl_IHSSBReadOnlyMemoryBuffer( );

public:

	// インスタンス生成
	static HRESULT CreateInstance( IHSSBReadOnlyMemoryBuffer** ppInstance, const void* pBuffer, size_t size );

	// コピー/ムーブを禁止して誤った複製を防止
	impl_IHSSBReadOnlyMemoryBuffer( const impl_IHSSBReadOnlyMemoryBuffer& ) = delete;
	impl_IHSSBReadOnlyMemoryBuffer& operator=( const impl_IHSSBReadOnlyMemoryBuffer& ) = delete;
	impl_IHSSBReadOnlyMemoryBuffer( impl_IHSSBReadOnlyMemoryBuffer&& ) = delete;
	impl_IHSSBReadOnlyMemoryBuffer& operator=( impl_IHSSBReadOnlyMemoryBuffer&& ) = delete;

	// IHSSBReadOnlyMemoryBuffer / 基底インターフェイスの実装（override を付与）
	virtual bool InquiryProvided( REFIID TargetIID ) const override;
	virtual HRESULT STDMETHODCALLTYPE QueryInterface( REFIID riid, _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject ) override;
	virtual ULONG STDMETHODCALLTYPE AddRef( void ) override;
	virtual ULONG STDMETHODCALLTYPE Release( void ) override;

	virtual bool InquiryProvidedExtraService( REFIID TargetIID ) const override;
	virtual HRESULT STDMETHODCALLTYPE QueryExtraService( REFIID riid, void** ppvObject ) override;

	virtual size_t GetSize( void ) const override;
	virtual bool IsValidElementNumber( size_t offset ) const override;
	virtual const void* GetConstBufferPointer( size_t offset = 0 ) const override;

};