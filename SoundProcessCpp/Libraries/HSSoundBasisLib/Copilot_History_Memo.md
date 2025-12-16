```
現在のファイルで宣言されているimpl_IHSSBNormalizedPCMBufferの実装のうち以下のメンバ関数については、
既存の実装を参考に実装したい。

	virtual HRESULT STDMETHODCALLTYPE QueryInterface( REFIID riid, _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject ) override;
	virtual ULONG STDMETHODCALLTYPE AddRef( void ) override;
	virtual ULONG STDMETHODCALLTYPE Release( void ) override;

	virtual bool InquiryProvided( REFIID TargetIID ) const override;
	virtual bool InquiryProvidedExtraService( REFIID TargetIID ) const override;
	virtual HRESULT QueryExtraService( REFIID riid, void** ppvObject ) override;


参考となる実装は、IHSSBWritableMemoryBufferの実装となるimpl_IHSSBWritableMemoryBufferクラスとなり、
以下のファイルで実装されている

	impl_IHSSBWritableMemoryBuffer.hpp
	impl_IHSSBWritableMemoryBuffer.cpp

なお、先に記載したメンバ関数以外の実装支援は不要である。
```


```
以下のファイルで定義されている impl_IHSSBNormalizedPCMBufferクラスのInitializeについてレビューをお願いしたい。
	impl_IHSSBNormalizedPCMBuffer.hpp
	impl_IHSSBNormalizedPCMBuffer.cpp

なお、IHSSBMemoryBufferの実装は以下で行われているため、参考にしてほしい。
	impl_IHSSBMemoryBuffer.hpp
	impl_IHSSBMemoryBuffer.cpp

以下は参考になる可能性があるファイルです
	HSSBMemoryOperations.hpp
```
