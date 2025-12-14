#include "impl_IHSSBNormalizedPCMBuffer.hpp"
#include <new> // std::nothrow

impl_IHSSBNormalizedPCMBuffer::impl_IHSSBNormalizedPCMBuffer( ) : m_ref(1),m_Channels(1), m_NumberOfSamples (1){

}

impl_IHSSBNormalizedPCMBuffer::~impl_IHSSBNormalizedPCMBuffer( ) {
}

HRESULT impl_IHSSBNormalizedPCMBuffer::CreateInstance( IHSSBNormalizedPCMBuffer** ppBuffer ) {
	if ( !ppBuffer ) return E_POINTER;
	*ppBuffer = nullptr;

	auto instance = new (std::nothrow) impl_IHSSBNormalizedPCMBuffer();
	if ( instance == nullptr ) {
		return E_OUTOFMEMORY;
	}

	*ppBuffer = instance;
	return S_OK;
}

HRESULT __stdcall impl_IHSSBNormalizedPCMBuffer::QueryInterface( REFIID riid, _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject ) {
	if ( !ppvObject ) return E_POINTER;
	*ppvObject = nullptr;

	if ( IsEqualIID( riid, IID_IHSSBNormalizedPCMBuffer ) ) {
		*ppvObject = static_cast<IHSSBNormalizedPCMBuffer*>( this );
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

ULONG __stdcall impl_IHSSBNormalizedPCMBuffer::AddRef( void ) {
	return static_cast<ULONG>( InterlockedIncrement( &m_ref ) );
}

ULONG __stdcall impl_IHSSBNormalizedPCMBuffer::Release( void ) {
	LONG newVal = InterlockedDecrement( &m_ref );
	if ( newVal == 0 ) {
		delete this;
		return 0;
	}
	return static_cast<ULONG>( newVal );
}

bool impl_IHSSBNormalizedPCMBuffer::InquiryProvided( REFIID TargetIID ) const {

	IID provided_iids[] = {
		IID_IHSSBNormalizedPCMBuffer,
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

bool impl_IHSSBNormalizedPCMBuffer::InquiryProvidedExtraService( REFIID TargetIID ) const {

	// IHSSBMemoryBufferの提供は管理外でのメモリサイズが変更される可能性があるため未サポート扱いとする
	if ( IsEqualIID( TargetIID, IID_IHSSBMemoryBuffer ) ) {
		return false;
	}

	if ( this->m_MemoryBuffer.p ) {
		return this->m_MemoryBuffer->InquiryProvided(TargetIID);
	}

	return false;
}

HRESULT impl_IHSSBNormalizedPCMBuffer::QueryExtraService( REFIID riid, void** ppvObject ) {

	// IHSSBMemoryBufferの提供は管理外でのメモリサイズが変更される可能性があるため未サポート扱いとする
	if ( IsEqualIID( riid, IID_IHSSBMemoryBuffer ) ) {
		return  HRESULT_FROM_WIN32( ERROR_NOT_SUPPORTED );
	}

	if ( this->m_MemoryBuffer.p ) {
		return this->m_MemoryBuffer->QueryInterface( riid, ppvObject );
	}

	if ( ppvObject ) *ppvObject = nullptr;
	return E_NOTIMPL;
}


HRESULT impl_IHSSBNormalizedPCMBuffer::Initialize( size_t number_of_samples, uint8_t number_of_channels ) {

	// すでに初期化済みなら再初期化は許可しない
	if ( this->m_MemoryBuffer.p ) {
		return HRESULT_FROM_WIN32( ERROR_ALREADY_INITIALIZED);
	}

	size_t total_size;
	HRESULT hr;

	// バイト数を計算
	//		備考：number_of_samplesやnumber_of_channelsの値は0チェックを含めて、この関数の内部でチェックしている
	hr = HSSBMath_CalculateBytesBySamples_StrictType<double>( &total_size, 
		static_cast<uint64_t>( number_of_samples ),
		number_of_channels );

	if ( FAILED( hr ) ) {
		return hr;
	}

	// メモリバッファオブジェクトを作成
	hr = HSSBCreateMemoryBuffer( &this->m_MemoryBuffer );
	if ( FAILED( hr ) ) {
		return hr;
	}

	// メモリバッファを確保
	hr = this->m_MemoryBuffer->Prepare( total_size );
	if ( FAILED(hr) ) {
		// Prepare に失敗した場合、作成したメモリバッファを解放して再試行可能な状態に戻す
		this->m_MemoryBuffer.Release();
		return hr;
	}

	// 成功したら内部状態を設定
	this->m_Channels = number_of_channels;
	this->m_NumberOfSamples = number_of_samples;

	return S_OK;
}

HRESULT impl_IHSSBNormalizedPCMBuffer::Initialize( uint32_t sampling_frequency, uint32_t number_of_seconds, uint8_t number_of_channels ) {
	// 内部でdouble版の関数を呼び出す
	// なお、number_of_secondsはuint32_tなので変換による情報損失は発生しない
	// 値のチェックは内部で行うためここでは省略する
	return this->Initialize( sampling_frequency, static_cast<double>( number_of_seconds ), number_of_channels  , EHSSB_RoundMode::Down);
}

HRESULT impl_IHSSBNormalizedPCMBuffer::Initialize( uint32_t sampling_frequency, double number_of_seconds, uint8_t number_of_channels  , EHSSB_RoundMode  round_mode ) {

	size_t size_of_size_t = sizeof( size_t );
	HRESULT hr;

	// size_t のサイズに応じて処理を分岐 (これにより、オーバーフローは防止できる)
	if ( size_of_size_t == 4 ) {

		uint32_t calculated_samples;
		hr = HSSBMath32_CalculateSamplesBySeconds_Strict( &calculated_samples, sampling_frequency, number_of_seconds, round_mode );
		if ( FAILED( hr ) ) {
			return hr;
		}

		// HSSBMath32_CalculateSamplesBySeconds_Strictの仕様上0は返らないはずだが念のためチェック
		if ( calculated_samples == 0 ) {
			return E_INVALIDARG;
		}

		return this->Initialize( static_cast<size_t>( calculated_samples ), number_of_channels );

	} else if ( size_of_size_t == 8 ) {
		uint64_t calculated_samples;
		hr = HSSBMath64_CalculateSamplesBySeconds_Strict( &calculated_samples, sampling_frequency, number_of_seconds, round_mode );
		if ( FAILED( hr ) ) {
			return hr;
		}

		// HSSBMath64_CalculateSamplesBySeconds_Strictの仕様上0は返らないはずだが念のためチェック
		if ( calculated_samples == 0 ) {
			return E_INVALIDARG;
		}

		return this->Initialize( static_cast<size_t>( calculated_samples ), number_of_channels );
	} else {
		// size_t が32ビットでも64ビットでもない環境はサポートしない
		return E_NOTIMPL;
	}

}

uint8_t impl_IHSSBNormalizedPCMBuffer::GetNumberOfChannels( void ) const {
	return this->m_Channels;
}

size_t impl_IHSSBNormalizedPCMBuffer::GetNumberOfSamples( void ) const {
	return this->m_NumberOfSamples;
}

HRESULT impl_IHSSBNormalizedPCMBuffer::GetBytesIndex( size_t* pOutBytesIndex, size_t sample_index, uint8_t channel_index ) const {


	if ( !this->m_MemoryBuffer.p ) return HSSB_E_NOT_INITIALIZED;
	if ( !pOutBytesIndex ) return E_POINTER;

	size_t element_index;

	// サンプルインデックスとチャンネルインデックスから要素インデックスを取得
	HRESULT hr = this->GetIndex( &element_index, sample_index, channel_index );

	if ( FAILED( hr ) ) {
		return hr;
	}

	// バイトインデックスを計算して返す
	*pOutBytesIndex = element_index * sizeof( double );

	return S_OK;
}

HRESULT impl_IHSSBNormalizedPCMBuffer::GetIndex( size_t* pOutIndex, size_t sample_index, uint8_t channel_index ) const {

	if ( !this->m_MemoryBuffer.p ) return HSSB_E_NOT_INITIALIZED;

	if ( !pOutIndex ) return E_POINTER;

	if ( sample_index >= this->m_NumberOfSamples ) return E_INVALIDARG;
	if ( channel_index >= this->m_Channels ) return E_INVALIDARG;

	size_t index = sample_index * this->m_Channels + channel_index;

	*pOutIndex = index;
	return S_OK;
}

HRESULT impl_IHSSBNormalizedPCMBuffer::GetValue( double* pOutValue, size_t sample_index, uint8_t channel_index ) const {

	size_t index;
	if ( !this->m_MemoryBuffer.p ) return HSSB_E_NOT_INITIALIZED;
	if ( !pOutValue ) return E_POINTER;

	HRESULT hr = this->GetIndex( &index, sample_index, channel_index );
	if ( FAILED( hr ) ) {
		return hr;
	}

	// バッファから値を取得
	const double *pBuffer =	this->m_MemoryBuffer->GetConstBufferPointerType<double>( index );
	if ( !pBuffer ) {
		return E_FAIL;
	}

	*pOutValue = *pBuffer;

	return S_OK;
}

HRESULT impl_IHSSBNormalizedPCMBuffer::SetValue( double value, size_t sample_index, uint8_t channel_index ) {

	size_t index;
	if ( !this->m_MemoryBuffer.p ) return HSSB_E_NOT_INITIALIZED;

	HRESULT hr = this->GetIndex( &index, sample_index, channel_index );
	if ( FAILED( hr ) ) {
		return hr;
	}


	double* pBuffer = this->m_MemoryBuffer->GetBufferPointerType<double>( index );
	if ( !pBuffer ) {
		return E_FAIL;
	}

	*pBuffer = value;
	return S_OK;

}

HRESULT impl_IHSSBNormalizedPCMBuffer::CreateEmptyChannelBuffer( IHSSBWritableMemoryBuffer** ppBuffer ) const {
	if ( !this->m_MemoryBuffer.p ) return HSSB_E_NOT_INITIALIZED;

	if ( !ppBuffer ) return E_POINTER;
	*ppBuffer = nullptr;

	CComPtr<IHSSBMemoryBuffer> channel_buffer;

	HRESULT hr = HSSBCreateMemoryBuffer( &channel_buffer );
	if ( FAILED( hr ) ) {
		return hr;
	}

	// チャンネルあたりのバイト数を計算
	size_t channel_bytes;
	hr = HSSBMath_CalculateBytesBySamples_StrictType<double>( &channel_bytes,
		static_cast<uint64_t>( this->m_NumberOfSamples ),
		1 );

	if ( FAILED( hr ) ) {
		return hr;
	}

	// メモリバッファを確保
	hr = channel_buffer->Prepare( channel_bytes );
	if ( FAILED( hr ) ) {
		return hr;
	}

	// 成功したら出力
	return channel_buffer->QueryInterface( IID_IHSSBWritableMemoryBuffer, reinterpret_cast<void**>( ppBuffer ) );
}

HRESULT impl_IHSSBNormalizedPCMBuffer::CreateChannelBuffer( IHSSBWritableMemoryBuffer** ppBuffer, uint8_t channel_index ) const {
	if ( !this->m_MemoryBuffer.p ) return HSSB_E_NOT_INITIALIZED;

	if ( !ppBuffer ) return E_POINTER;
	*ppBuffer = nullptr;

	if ( channel_index >= this->m_Channels ) return E_INVALIDARG;

	CComPtr<IHSSBWritableMemoryBuffer> channel_buffer;

	// 空のチャンネルバッファを作成
	HRESULT hr = this->CreateEmptyChannelBuffer( &channel_buffer );
	if ( FAILED( hr ) ) {
		return hr;
	}

	// 元のバッファからチャンネルデータをコピー
	hr = this->ExportChannelData( channel_buffer, channel_index );

	if ( FAILED( hr ) ) {
		return hr;
	}

	// 成功したら出力
	return channel_buffer->QueryInterface( IID_IHSSBWritableMemoryBuffer, reinterpret_cast<void**>( ppBuffer ) );
}

HRESULT impl_IHSSBNormalizedPCMBuffer::ExportChannelData( IHSSBWritableMemoryBuffer* pChannelBuffer, uint8_t channel_index ) const {

	// 初期化されているかチェック
	if ( !this->m_MemoryBuffer.p ) return HSSB_E_NOT_INITIALIZED;

	// エクスポート設定を作成
	ExportImportSettings settings;

	// 各メンバーの値を設定
	// このセット内容により、全サンプルをエクスポートする設定になる

	// 全サンプルをエクスポートする設定にする
	settings.sample_length = this->m_NumberOfSamples;

	// エクスポート開始インデックスは0にする
	settings.from_side_sample_start_index = 0;

	// エクスポート先の開始インデックスも0にする
	settings.to_side_sample_start_index = 0;

	// 内部の詳細実装関数を呼び出す
	// なお、settingsの内容は上記の通りに設定されている
	// pChannelBuffer が有効かどうかのチェックは詳細実装関数内で行う
	// channel_index のチェックも同様に詳細実装関数内で行う
	return this->ExportChannelData( pChannelBuffer, &settings, channel_index );
}

HRESULT impl_IHSSBNormalizedPCMBuffer::ExportChannelData( IHSSBWritableMemoryBuffer* pChannelBuffer, ExportImportSettings* pSettings, uint8_t channel_index ) const {
	// 初期化されているかチェック
	if ( !this->m_MemoryBuffer.p ) return HSSB_E_NOT_INITIALIZED;

	if ( pChannelBuffer == nullptr || pSettings == nullptr ) return E_POINTER;

	// チャンネルインデックスのチェック
	if ( channel_index >= this->m_Channels ) return E_INVALIDARG;

	// チャンネルバッファのサイズチェック
	if ( pChannelBuffer->GetSize( ) == 0 )	 return E_INVALIDARG;

	//  double 型のサイズ単位でチェック
	if ( !pChannelBuffer->IsSizeByTypeSizeUnit<double>( ) ) return E_INVALIDARG;


	// 入力元となる本クラス側のサンプル数よりも、エクスポート開始インデックスが大きい場合はエラー
	if ( this->m_NumberOfSamples <= pSettings->from_side_sample_start_index ) {
		return E_INVALIDARG;
	}

	// エクスポート可能なサンプル数を計算
	size_t available_samples = this->m_NumberOfSamples - pSettings->from_side_sample_start_index;

	// 成功時における期待されるHRESULTを設定
	HRESULT Expect_hr_for_Success = S_OK;

	// エクスポートするサンプル数を決定
	size_t export_sample_count = pSettings->sample_length;
	if ( available_samples < export_sample_count ) {
		// 利用可能なサンプル数よりも多い場合は利用可能なサンプル数に制限
		export_sample_count = available_samples;

		// この場合、本クラス側のデータ量に合わせてエクスポートすることになり、
		// 指定されたサンプル数に満たないため、部分的成功を示すHRESULTを設定
		// 本クラスはINPUT側となるため、HSSB_S_PARTIAL_INPUT_SIDE_FACTORを設定する
		Expect_hr_for_Success = HSSB_S_PARTIAL_INPUT_SIDE_FACTOR;
	}


	// チャンネルバッファの要素数を取得
	size_t channel_buffer_elements = pChannelBuffer->GetSizeElements<double>( );

	// チャンネルバッファの開始インデックスが要素数以上の場合はエラー
	if( channel_buffer_elements <= pSettings->to_side_sample_start_index ) {
		return E_INVALIDARG;
	}

	// チャンネルバッファの利用可能な要素数を計算
	size_t available_channel_buffer_samples = channel_buffer_elements - pSettings->to_side_sample_start_index;


	// チャンネルバッファに書き込めるサンプル数を決定
	if ( export_sample_count > available_channel_buffer_samples ) {
		// チャンネルバッファに書き込めるサンプル数の方が少ない場合は、そちらに合わせる
		export_sample_count = available_channel_buffer_samples;

		// この場合、チャンネルバッファ側のデータ量に合わせてエクスポートすることになり、
		// 指定されたサンプル数に満たないため、部分的成功を示すHRESULTを設定
		// チャンネルバッファはOUTPUT側となるため、HSSB_S_PARTIAL_OUTPUT_SIDE_FACTORを設定する
		Expect_hr_for_Success = HSSB_S_PARTIAL_OUTPUT_SIDE_FACTOR;
	}


	// メモリに直接アクセスすることになるため念のため、それぞれ終端アクセス位置の再チェックを行う
	if(( pSettings->from_side_sample_start_index + export_sample_count ) > this->m_NumberOfSamples ) {
		// 本クラス側の終端を超える場合は、前述の計算にバグがあると考えられるためエラー
#ifdef _DEBUG
		// デバッグビルド時は詳細なエラーを返す
		return HSSB_E_PROCESS_ERROR_BY_BUG_FACTOR;
#else
		// リリースビルド時は一般的なエラーを返す
		return E_ABORT;
#endif // _DEBUG
	}

	if ( ( pSettings->to_side_sample_start_index + export_sample_count ) > channel_buffer_elements ) {
		// チャンネルバッファ側の終端を超える場合は、前述の計算にバグがあると考えられるためエラー
#ifdef _DEBUG
		// デバッグビルド時は詳細なエラーを返す
		return HSSB_E_PROCESS_ERROR_BY_BUG_FACTOR;
#else
		// リリースビルド時は一般的なエラーを返す
		return E_ABORT;
#endif // _DEBUG
	
	}

	size_t self_side_index;
	// 元のバッファの開始インデックスを取得(また、チャンネルのオフセットも加味される)
	if ( this->GetIndex( &self_side_index, pSettings->from_side_sample_start_index, channel_index ) != S_OK ) {
		// バイトインデックスの取得に失敗した場合は、前述の計算にバグがあると考えられるためエラー
#ifdef _DEBUG
		// デバッグビルド時は詳細なエラーを返す
		return HSSB_E_PROCESS_ERROR_BY_BUG_FACTOR;
#else
		// リリースビルド時は一般的なエラーを返す
		return E_ABORT;
#endif // _DEBUG

	}

	// 元のバッファと先のバッファのポインタを取得
	const double* pSrcBuffer = this->m_MemoryBuffer->GetConstBufferPointerType<double>( self_side_index );
	double* pDestBuffer = pChannelBuffer->GetBufferPointerType<double>( pSettings->to_side_sample_start_index );

	if ( !pSrcBuffer || !pDestBuffer ) {
		return E_FAIL;
	}

	// 元のバッファからチャンネルデータをコピー
	const size_t number_of_channels = static_cast<size_t>( this->m_Channels );
	for ( size_t i = 0; i < export_sample_count; ++i ) {

		// 本来、pSrcBufferのオフセット計算にはchannel_indexを加算する必要があるが、
		//	すでに、pSrcBufferは指定されたチャンネルのデータを示しているため、channel_indexの加算は不要
		//		（self_side_indexの計算時にchannel_indexを加算しているため）
		pDestBuffer[i] = pSrcBuffer[i * number_of_channels];
	}

	// 処理したサンプル数を設定
	// これにより、呼び出し元で実際に処理したサンプル数を知ることができる
	pSettings->result_processed_sample_length = export_sample_count;

	return Expect_hr_for_Success;
}

HRESULT impl_IHSSBNormalizedPCMBuffer::ImportChannelData( IHSSBReadOnlyMemoryBuffer* pChannelBuffer, uint8_t channel_index ) {
	// 初期化されているかチェック
	if ( !this->m_MemoryBuffer.p ) return HSSB_E_NOT_INITIALIZED;

	// インポート設定を作成
	ExportImportSettings settings;

	// 各メンバーの値を設定
	// このセット内容により、全サンプルをインポートする設定になる

	// 全サンプルをインポートする設定にする
	settings.sample_length = this->m_NumberOfSamples;

	// インポート元の開始インデックスは0にする
	settings.from_side_sample_start_index = 0;

	// インポート先の開始インデックスも0にする
	settings.to_side_sample_start_index = 0;

	// 内部の詳細実装関数を呼び出す
	// なお、settingsの内容は上記の通りに設定されている
	// pChannelBuffer が有効かどうかのチェックは詳細実装関数内で行う
	// channel_index のチェックも同様に詳細実装関数内で行う
	return this->ImportChannelData( pChannelBuffer, &settings, channel_index );
}

HRESULT impl_IHSSBNormalizedPCMBuffer::ImportChannelData( IHSSBReadOnlyMemoryBuffer* pChannelBuffer, ExportImportSettings* pSettings, uint8_t channel_index ) {
	// 初期化されているかチェック
	if ( !this->m_MemoryBuffer.p ) return HSSB_E_NOT_INITIALIZED;
	
	if ( pChannelBuffer == nullptr || pSettings == nullptr ) return E_POINTER;

	// チャンネルインデックスのチェック
	if ( channel_index >= this->m_Channels ) return E_INVALIDARG;

	// チャンネルバッファのサイズチェック
	if ( pChannelBuffer->GetSize( ) == 0 )	 return E_INVALIDARG;


	//  double 型のサイズ単位でチェック
	if ( !pChannelBuffer->IsSizeByTypeSizeUnit<double>( ) ) return E_INVALIDARG;

	// 入力元となるチャンネルバッファ側のサンプル数よりも、インポート開始インデックスが大きい場合はエラー
	if ( pChannelBuffer->GetSizeElements<double>( ) <= pSettings->from_side_sample_start_index ) {
		return E_INVALIDARG;
	}

	// インポート可能なサンプル数を計算
	size_t available_samples = pChannelBuffer->GetSizeElements<double>( ) - pSettings->from_side_sample_start_index;

	// 成功時における期待されるHRESULTを設定
	HRESULT Expect_hr_for_Success = S_OK;

	// インポートするサンプル数を決定
	size_t import_sample_count = pSettings->sample_length;

	if ( available_samples < import_sample_count ) {
		// 利用可能なサンプル数よりも多い場合は利用可能なサンプル数に制限
		import_sample_count = available_samples;

		// この場合、チャンネルバッファ側のデータ量に合わせてインポートすることになり、
		// 指定されたサンプル数に満たないため、部分的成功を示すHRESULTを設定
		// チャンネルバッファはINPUT側となるため、HSSB_S_PARTIAL_INPUT_SIDE_FACTORを設定する
		Expect_hr_for_Success = HSSB_S_PARTIAL_INPUT_SIDE_FACTOR;
	}

	// 本クラス側のサンプル数よりも、インポート先の開始インデックスが大きい場合はエラー
	if ( this->m_NumberOfSamples <= pSettings->to_side_sample_start_index ) {
		return E_INVALIDARG;
	}


	// 本クラス側の利用可能なサンプル数を計算
	size_t available_self_samples = this->m_NumberOfSamples - pSettings->to_side_sample_start_index;

	// 本クラス側に書き込めるサンプル数を決定
	if ( import_sample_count > available_self_samples ) {
		
		// 本クラス側に書き込めるサンプル数の方が少ない場合は、そちらに合わせる
		import_sample_count = available_self_samples;

		// この場合、本クラス側のデータ量に合わせてインポートすることになり、
		// 指定されたサンプル数に満たないため、部分的成功を示すHRESULTを設定
		// 本クラスはOUTPUT側となるため、HSSB_S_PARTIAL_OUTPUT_SIDE_FACTORを設定する
		Expect_hr_for_Success = HSSB_S_PARTIAL_OUTPUT_SIDE_FACTOR;
	}

	// メモリに直接アクセスすることになるため念のため、それぞれ終端アクセス位置の再チェックを行う
	if ( ( pSettings->from_side_sample_start_index + import_sample_count ) > pChannelBuffer->GetSizeElements<double>( ) ) {
		// チャンネルバッファ側の終端を超える場合は、前述の計算にバグがあると考えられるためエラー
#ifdef _DEBUG
		// デバッグビルド時は詳細なエラーを返す
		return HSSB_E_PROCESS_ERROR_BY_BUG_FACTOR;
#else
		// リリースビルド時は一般的なエラーを返す
		return E_ABORT;
#endif // _DEBUG
	}

	if ( ( pSettings->to_side_sample_start_index + import_sample_count ) > this->m_NumberOfSamples ) {
		// 本クラス側の終端を超える場合は、前述の計算にバグがあると考えられるためエラー
#ifdef _DEBUG
		// デバッグビルド時は詳細なエラーを返す
		return HSSB_E_PROCESS_ERROR_BY_BUG_FACTOR;
#else

		// リリースビルド時は一般的なエラーを返す
		return E_ABORT;
#endif // _DEBUG

	}

	size_t self_side_index;
	// 本クラス側の開始インデックスを取得(また、チャンネルのオフセットも加味される)
	if ( this->GetIndex( &self_side_index, pSettings->to_side_sample_start_index, channel_index ) != S_OK ) {
		// バイトインデックスの取得に失敗した場合は、前述の計算にバグがあると考えられるためエラー
#ifdef _DEBUG
		// デバッグビルド時は詳細なエラーを返す
		return HSSB_E_PROCESS_ERROR_BY_BUG_FACTOR;
#else
		// リリースビルド時は一般的なエラーを返す
		return E_ABORT;
#endif // _DEBUG
	}

	// 元のバッファと先のバッファのポインタを取得
	const double* pSrcBuffer = pChannelBuffer->GetConstBufferPointerType<double>( pSettings->from_side_sample_start_index );
	double* pDestBuffer = this->m_MemoryBuffer->GetBufferPointerType<double>( self_side_index );
	if ( !pSrcBuffer || !pDestBuffer ) {
		return E_FAIL;
	}

	// チャンネルバッファから元のバッファへデータをコピー
	const size_t number_of_channels = static_cast<size_t>( this->m_Channels );
	for ( size_t i = 0; i < import_sample_count; ++i ) {
		// 本来、pDestBufferのオフセット計算にはchannel_indexを加算する必要があるが、
		//	すでに、pDestBufferは指定されたチャンネルのデータを示しているため、channel_indexの加算は不要
		//		（self_side_indexの計算時にchannel_indexを加算しているため）
		pDestBuffer[i * number_of_channels] = pSrcBuffer[i];
	}
	// 処理したサンプル数を設定
	pSettings->result_processed_sample_length = import_sample_count;
	return Expect_hr_for_Success;
}

size_t impl_IHSSBNormalizedPCMBuffer::GetBytesSize( void ) const {
	if ( !this->m_MemoryBuffer.p ) return 0;
	return this->m_MemoryBuffer->GetSize( );
}

