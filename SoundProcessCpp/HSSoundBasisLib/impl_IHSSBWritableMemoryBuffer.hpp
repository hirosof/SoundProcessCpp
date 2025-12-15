#pragma once

#define HSSOUNDBASISLIB_EXPORT_DLL_INTERNAL
#include "HSSoundBasisLib.hpp"

/// impl_IHSSBWritableMemoryBuffer
/// 所有権ポリシー:
/// - デフォルト: 本クラスは渡された `pBuffer` の所有権を持ちません（非所有）。
///   呼び出し側がバッファの割当・解放の責任を負います。
/// - 所有権を移譲する API（例: ownsBuffer フラグや専用ファクトリ）を導入する場合は、
///   その場合の割当/解放方法（`new[]` / `free` 等）をヘッダに明記してください。
class impl_IHSSBWritableMemoryBuffer : public IHSSBWritableMemoryBuffer {

private:

	volatile LONG m_ref;
	uint8_t* m_pBuffer;
	size_t m_BufferSize;

	impl_IHSSBWritableMemoryBuffer( void* pBuffer, size_t size );
	~impl_IHSSBWritableMemoryBuffer( );

public:

	static HRESULT CreateInstance( IHSSBWritableMemoryBuffer** ppInstance, void* pBuffer, size_t size );

	virtual bool InquiryProvided( REFIID TargetIID ) const;
	virtual HRESULT STDMETHODCALLTYPE QueryInterface( REFIID riid, _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject );
	virtual ULONG STDMETHODCALLTYPE  AddRef( void );
	virtual ULONG STDMETHODCALLTYPE Release( void );

	virtual bool InquiryProvidedExtraService( REFIID TargetIID ) const;
	virtual HRESULT QueryExtraService( REFIID riid, void** ppvObject );

	virtual size_t GetSize( void ) const;
	virtual bool IsValidElementNumber( size_t offset ) const;
	virtual const void* GetConstBufferPointer( size_t offset = 0 )const;
	virtual void* GetBufferPointer( size_t offset = 0 )const;


	virtual HRESULT CheckValidElementNumberRange( size_t offset, size_t length ) const override;
	virtual HRESULT CheckValidElementNumberRangeOffset( size_t start_offset, size_t end_offset ) const override;

};