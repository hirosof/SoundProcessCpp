#pragma once

#define HSSOUNDBASISLIB_EXPORT_DLL_INTERNAL
#include "../HSSoundBasisLib.hpp"

class impl_IHSSBMemoryOwner : public IHSSBMemoryOwner {

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

	// クリティカルセクション（スレッドセーフ対応用）
	CRITICAL_SECTION m_CriticalSection;

	// コンストラクタは private にして、CreateInstance 経由でしかインスタンス生成できないようにする
	impl_IHSSBMemoryOwner( );

	// デストラクタも private にして、Release 経由でしか削除できないようにする
	~impl_IHSSBMemoryOwner( );


	// new[] で確保されたバッファーを解放するための専用関数
	void FreeForNewAllocatedBuffer( void );

	// new[] で確保されたバッファーを解放するための型別テンプレート関数
	// (FreeForNewAllocatedBufferから呼び出される)
	template <typename T> void  FreeForNewAllocatedBufferInternal( void ) {
		T* pArray = reinterpret_cast<T*>( m_pBuffer );
		delete[] pArray;
	}

public:
	// インスタンス生成

	// デフォルト（空のメモリオーナー）インスタンスを作成
	static HRESULT CreateInstance( IHSSBMemoryOwner** ppInstance );

	// 指定されたバッファーを所有するメモリオーナーインスタンスを作成
	static HRESULT CreateInstance( IHSSBMemoryOwner** ppInstance,
		void* pBuffer,
		size_t size,
		EHSSBMemoryOwnershipType owner = EHSSBMemoryOwnershipType::NoOwnership,
		EHSSBMemoryNewAllocatedTypeInfo owner_type_info = EHSSBMemoryNewAllocatedTypeInfo::None
	);
	
	// コピー/ムーブを禁止して誤った複製を防止
	impl_IHSSBMemoryOwner( const impl_IHSSBMemoryOwner& ) = delete;
	impl_IHSSBMemoryOwner& operator=( const impl_IHSSBMemoryOwner& ) = delete;
	impl_IHSSBMemoryOwner( impl_IHSSBMemoryOwner&& ) = delete;
	impl_IHSSBMemoryOwner& operator=( impl_IHSSBMemoryOwner&& ) = delete;


	// IHSSBMemoryOwner / 基底インターフェイスの実装（override を付与）
	virtual bool InquiryProvided( REFIID TargetIID ) const override;
	virtual HRESULT STDMETHODCALLTYPE QueryInterface( REFIID riid, _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject ) override;
	virtual ULONG STDMETHODCALLTYPE AddRef( void ) override;
	virtual ULONG STDMETHODCALLTYPE Release( void ) override;


	virtual bool InquiryProvidedExtraService( REFIID TargetIID ) const override;
	virtual HRESULT STDMETHODCALLTYPE QueryExtraService( REFIID riid, void** ppvObject ) override;


	// IHSSBMemoryOwner の実装（override を付与）
	
	// メモリバッファーをアタッチ
	virtual HRESULT Attach( void* pBuffer, size_t size,
		EHSSBMemoryOwnershipType owner = EHSSBMemoryOwnershipType::NoOwnership,
		EHSSBMemoryNewAllocatedTypeInfo owner_type_info = EHSSBMemoryNewAllocatedTypeInfo::None
	) override;

	// メモリバッファーをデタッチ
	virtual HRESULT Detach( void** ppOutBuffer,
		size_t* pOutSize = nullptr,
		EHSSBMemoryOwnershipType* pOutOwner = nullptr,
		EHSSBMemoryNewAllocatedTypeInfo* pOutOwnerTypeInfo = nullptr
	) override;

	// 所有しているメモリを解放
	virtual HRESULT Free( void ) override;

	// バッファーポインタとサイズを取得
	virtual void* GetBufferPointer( void ) const override;

	// バッファーサイズを取得
	virtual size_t GetSize( void ) const override;

	// 所有権タイプを取得
	virtual EHSSBMemoryOwnershipType GetOwnershipType( void ) const override;

	// 所有権タイプ情報を取得
	virtual EHSSBMemoryNewAllocatedTypeInfo GetOwnershipTypeInfo( void ) const override;

};
