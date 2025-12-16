#pragma once

#define HSSOUNDBASISLIB_EXPORT_DLL_INTERNAL
#include "HSSoundBasisLib.hpp"

class impl_IHSSBReadOnlyMemoryBuffer : public IHSSBReadOnlyMemoryBuffer {

private:

	// 参照カウント
	volatile LONG m_ref;

	// バッファーポインタ (バイト単位で扱うため uint8_t* とする)
	uint8_t* m_pBuffer;

	// メモリ所有権タイプ
	EHSSBMemoryOwnershipType m_OwnershipType;

	// メモリ新規確保タイプ情報 (new[]で確保された配列用)
	EHSSBMemoryNewAllocatedTypeInfo m_OwnershipTypeInfo;

	// バッファーサイズ（バイト単位）
	size_t m_BufferSize;

	// コンストラクタは private にして、CreateInstance 経由でしかインスタンス生成できないようにする
	impl_IHSSBReadOnlyMemoryBuffer( void* pBuffer, size_t size );

	// デストラクタも private にして、Release 経由でしか削除できないようにする
	~impl_IHSSBReadOnlyMemoryBuffer( );

	// new[] で確保されたバッファーを解放するための専用関数
	void FreeForNewAllocatedBuffer(void );

	// new[] で確保されたバッファーを解放するための型別テンプレート関数
	// (FreeForNewAllocatedBufferから呼び出される)
	template <typename T> void  FreeForNewAllocatedBufferInternal( void ) {
		T* pArray = reinterpret_cast<T*>( m_pBuffer );
		delete[] pArray;
	}

public:

	// インスタンス生成
	static HRESULT CreateInstance( IHSSBReadOnlyMemoryBuffer** ppInstance, void* pBuffer, size_t size );
	static HRESULT CreateInstance( IHSSBReadOnlyMemoryBuffer** ppInstance, 
		void* pBuffer, 
		size_t size , 
		EHSSBMemoryOwnershipType owner,
		EHSSBMemoryNewAllocatedTypeInfo owner_type_info = EHSSBMemoryNewAllocatedTypeInfo::None
	);

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

	//バッファサイズ取得
	virtual size_t GetSize( void ) const override;

	// 要素番号の妥当性確認
	virtual bool IsValidElementNumber( size_t offset ) const override;

	// バッファーポインタ取得(読み取り専用)
	virtual const void* GetConstBufferPointer( size_t offset = 0 ) const override;

	virtual HRESULT CheckValidElementNumberRange( size_t offset, size_t length ) const override;
	virtual HRESULT CheckValidElementNumberRangeOffset( size_t start_offset, size_t end_offset ) const override;

};