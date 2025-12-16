/*
	自作ライブラリ『HSSoundBasisLib』によって提供している、メモリ管理系のサンプルプログラム
	(※単体テストのプログラムではありません)
*/

// Windows.h の min/max マクロ定義を無効化
#define NOMINMAX

#include <cstdio>
#include <cstdint>
#include <locale>
#include <atlbase.h>
#include <algorithm> // std::min を使用するために追加
#include <map>

// ※ HSSoundBasisLib.hppをインクルードすることによって、Windows.hもインクルードされます
// ※ ただし、HSSoundBasisLib.hppが直接Windows.hをインクルードしているわけではなく別のヘッダーによって読み込まれます
#include "../HSSoundBasisLib/HSSoundBasisLib.hpp"

void Sample_MemoryOwner( void );
void Sample_ReadOnlyMemoryBuffer( void );
void Sample_ReadOnlyMemoryBuffer_DumpData( IHSSBReadOnlyMemoryBuffer *pBuffer );


// サンプルモード列挙体
enum class SampleMode {
	MemoryOwner = 0,
	ReadOnlyMemoryBuffer,
	End		
};



int main( void ) {

	setlocale( LC_ALL, "Japanese" );
    SetConsoleOutputCP( 65001 ); // UTF-8コードページに変更


	std::map<SampleMode , const wchar_t*> sampleModeNames = {
		{ SampleMode::MemoryOwner, L"メモリオーナー" },
		{ SampleMode::ReadOnlyMemoryBuffer, L"読み取り専用メモリバッファ" }
	};


	wprintf( L"HSSoundBasisLib メモリ管理サンプルプログラム\n" );

	wprintf( L"=== サンプル一覧 ===\n" );

	for ( const auto& pair : sampleModeNames ) {
		if ( pair.first >= SampleMode::End ) {
			continue;
		}
		wprintf( L"\t%d: %s\n", static_cast<int>( pair.first ), pair.second );
	}

	wprintf( L"\n実行したいサンプルの番号を入力してください: " );

	int sampleNo = 0;
	(void)scanf_s( "%d", &sampleNo );

	if ( static_cast<int>( SampleMode::End ) <= sampleNo ) {
		wprintf( L"不正な番号が入力されました。プログラムを終了します。\n" );
		return 0;
	}

	// 選択されたサンプルモード
	SampleMode selectedMode = static_cast<SampleMode>( sampleNo );
	switch ( selectedMode ) {
		case SampleMode::MemoryOwner:
			Sample_MemoryOwner( );
			break;
		case SampleMode::ReadOnlyMemoryBuffer:
			Sample_ReadOnlyMemoryBuffer( );
			break;
		default:
			// 念のため不正な番号が来た場合の処理
			wprintf( L"不正な番号が入力されました。プログラムを終了します。\n" );
			return 0;
	}
	return 0;
}


void Sample_MemoryOwner( void ) {

	// IHSSBMemoryOwnerはIUnknownを実装しているため、CComPtrを使用できる
	CComPtr<IHSSBMemoryOwner> memoryOwner;

	wprintf( L"=== メモリオーナーのサンプル ===\n" );

	// メモリ確保サイズ
	size_t BufferAllocateSize = 512;

	// メモリを確保
	// ※ HeapAllocのメモリを移譲する場合、HeapハンドルはGetProcessHeapで取得する必要があります。
	void* pBuffer = HeapAlloc( GetProcessHeap( ), 0, BufferAllocateSize );

	// 確保に失敗した場合は終了
	if ( !pBuffer ) {
		wprintf( L"HeapAlloc 失敗\n" );
		return;
	}

	// pBufferにデータを書き込む
	// 値は0～255を順番に書き込む
	uint8_t* pByteBuffer = static_cast<uint8_t*>( pBuffer );
	for ( size_t i = 0; i < BufferAllocateSize; ++i ) {
		pByteBuffer[i] = static_cast<uint8_t>( i & 0xFF );
	}


	HRESULT hr;

	// HSSBCreateReadOnlyMemoryBufferに渡す誤ったサイズを用意
	// ここでは、BufferAllocateSizeの3倍を通知することとします。
	size_t OverSizedNotifySize = BufferAllocateSize * 3;

	// ヒープメモリの所有権を持つメモリオーナーを作成
	hr = HSSBCreateMemoryOwner( &memoryOwner,
		// 移譲するバッファ
		pBuffer,
		// 意図的に誤ったサイズを通知
		OverSizedNotifySize,
		// 管理を移譲し、また、該当のメモリはHeapAllocで確保したものであることを通知します
		EHSSBMemoryOwnershipType::WithHeapFreeOwnership_HeapAlloced
	);

	// 作成に失敗した場合
	if ( FAILED( hr ) ) {
		wprintf( L"HSSBCreateMemoryOwner 失敗. hr=0x%08X\n", hr );
		// 所有権を持たない場合は自分で解放する必要がある
		// これは、IHSSBMemoryOwnerへの管理移譲が完了していないためです
		HeapFree( GetProcessHeap( ), 0, pBuffer );
		return;
	}


	// 先ほど、サイズ指定を多く通知したためS_OKは返らないはずである。これは、HeapAllocで確保したものを移譲しようとした際、
	// HSSBCreateMemoryOwner内部でHeapSizeで実際のサイズを確認し、
	// 実際のサイズが指定されたサイズよりも小さい場合、 それをもとに実装クラス内で管理するサイズを調整し、
	// HSSB_S_OK_BUT_MANAGED_SIZE_ADJUSTEDを返すためである。
	if ( hr != S_OK ) {
		wprintf( L"HSSBCreateMemoryOwner Result hr = 0x%08X\n", hr );
		wprintf( L"HSSB_S_OK_BUT_MANAGED_SIZE_ADJUSTED = 0x%08X\n\n", HSSB_S_OK_BUT_MANAGED_SIZE_ADJUSTED );
		if ( hr == HSSB_S_OK_BUT_MANAGED_SIZE_ADJUSTED ) {
			wprintf( L"\tこのように、実際のサイズより大きいサイズを指定した場合、\n" );
			wprintf( L"\tHSSB_S_BUT_MANAGED_SIZE_ADJUSTEDが返ります。 \n" );
			wprintf( L"\t(HeapAllocで確保したものを移譲した場合のみ)\n\n" );
		} else {
			wprintf( L"※ ライブラリ側のバグの可能性があります ※\n" );
		}
		wprintf( L"具体的に、以下のようにサイズ調整がされます。\n" );
		wprintf( L"\tHSSBCreateMemoryOwnerに渡したバッファの実際の確保サイズ：%zu\n", BufferAllocateSize );
		wprintf( L"\tHSSBCreateMemoryOwnerに渡したサイズ：%zu\n", OverSizedNotifySize );
		wprintf( L"\tHSSBCreateMemoryOwnerが認識しているサイズ：%zu\n\n", memoryOwner->GetSize( ) );
	} else {
		wprintf( L"※ ライブラリ側のバグの可能性があります ※\n" );
	}

	wprintf( L"\n" );

	// Detachでバッファを取り出す
	void* pDetachedBuffer = nullptr;
	hr = memoryOwner->Detach( &pDetachedBuffer );

	if ( FAILED( hr ) ) {
		wprintf( L"Detach 失敗. hr=0x%08X\n", hr );
		return;
	}

	wprintf( L"Detach 成功. 取り出したバッファーポインタ: %p\n", pDetachedBuffer );
	
	// この時、当初に移譲したバッファポインタと同じであることを確認
	if ( pDetachedBuffer != pBuffer ) {

		wprintf( L"※ ライブラリ側のバグの可能性があります ※\n" );
		return;
	}

	wprintf( L"取り出したバッファーポインタは、当初移譲したバッファーポインタと同じです。\n" );

	// 今度はAttachで再度バッファをアタッチする
	// 今度は、正しいサイズを通知します

	hr = memoryOwner->Attach( pDetachedBuffer, BufferAllocateSize,
		EHSSBMemoryOwnershipType::WithHeapFreeOwnership_HeapAlloced
	);

	if ( FAILED( hr ) ) {
		wprintf( L"Attach 失敗. hr=0x%08X\n", hr );
		// 所有権を持たない場合は自分で解放する必要がある
		HeapFree( GetProcessHeap( ), 0, pDetachedBuffer );
		return;
	}


	wprintf( L"Attach 成功.\n" );

	// 正しいサイズを通知したため、S_OKが返るはずである
	if ( hr == S_OK ) {
		wprintf( L"通知したサイズで正しくアタッチされました。\n" );
	} else {
		wprintf( L"※ ライブラリ側のバグの可能性があります ※\n" );
	}

	// CComPtrによって、Releaseが自動で呼ばれるため、解放不要
	// 今回移譲したバッファは、HeapAllocで確保したものであることを通知していたため、
	// 参照カウンタが0になり、IHSSBMemoryOwnerが解放される際に
	// HeapFreeが呼ばれ、移譲したバッファが自動で解放されます。
}


void Sample_ReadOnlyMemoryBuffer( void ) {


	// IHSSBReadOnlyMemoryBufferはIUnknownを実装しているため、CComPtrを使用できる
	CComPtr<IHSSBReadOnlyMemoryBuffer> readOnlyBuffer;

	wprintf( L"=== 読み取り専用メモリバッファのサンプル ===\n" );

	// メモリ確保サイズ
	size_t BufferAllocateSize = 512;

	// メモリを確保
	// ※ HeapAllocのメモリを移譲する場合、HeapハンドルはGetProcessHeapで取得する必要があります。
	void* pBuffer = HeapAlloc( GetProcessHeap( ), 0, BufferAllocateSize );

	// 確保に失敗した場合は終了
	if ( !pBuffer ) {
		wprintf( L"HeapAlloc 失敗\n" );
		return;
	}

	// pBufferにデータを書き込む
	// 値は0～255を順番に書き込む
	uint8_t* pByteBuffer = static_cast<uint8_t*>( pBuffer );
	for ( size_t i = 0; i < BufferAllocateSize; ++i ) {
		pByteBuffer[i] = static_cast<uint8_t>( i & 0xFF );
	}


	HRESULT hr;

	// HSSBCreateReadOnlyMemoryBufferに渡す誤ったサイズを用意
	// ここでは、BufferAllocateSizeの倍を通知することとします。
	size_t OverSizedNotifySize = BufferAllocateSize * 2;

	// ヒープメモリの所有権を持つ読み取り専用メモリバッファを作成
	hr = HSSBCreateReadOnlyMemoryBuffer( &readOnlyBuffer,

		// 移譲するバッファ
		pBuffer,

		// 意図的に誤ったサイズを通知
		OverSizedNotifySize,

		// 管理を移譲し、また、該当のメモリはHeapAllocで確保したものであることを通知します
		EHSSBMemoryOwnershipType::WithHeapFreeOwnership_HeapAlloced
	);

	// 作成に失敗した場合
	if ( FAILED( hr ) ) {
		wprintf( L"HSSBCreateReadOnlyMemoryBuffer 失敗. hr=0x%08X\n", hr );
		// 所有権を持たない場合は自分で解放する必要がある
		// これは、IHSSBReadOnlyMemoryBufferへの管理移譲が完了していないためです
		HeapFree( GetProcessHeap( ), 0, pBuffer );
		return;
	}

	// 先ほど、サイズ指定を多く通知したためS_OKは返らないはずである。これは、HeapAllocで確保したものを移譲しようとした際、
	// HSSBCreateReadOnlyMemoryBuffer内部でHeapSizeで実際のサイズを確認し、
	// 実際のサイズが指定されたサイズよりも小さい場合、 それをもとに実装クラス内で管理するサイズを調整し、
	// HSSB_S_OK_BUT_MANAGED_SIZE_ADJUSTEDを返すためである。
	if ( hr != S_OK ) {
		wprintf( L"HSSBCreateReadOnlyMemoryBuffer Result hr = 0x%08X\n", hr );
		wprintf( L"HSSB_S_OK_BUT_MANAGED_SIZE_ADJUSTED = 0x%08X\n\n", HSSB_S_OK_BUT_MANAGED_SIZE_ADJUSTED );

		if ( hr == HSSB_S_OK_BUT_MANAGED_SIZE_ADJUSTED ) {
			wprintf( L"\tこのように、実際のサイズより大きいサイズを指定した場合、\n" );
			wprintf( L"\tHSSB_S_BUT_MANAGED_SIZE_ADJUSTEDが返ります。 \n" );
			wprintf( L"\t(HeapAllocで確保したものを移譲した場合のみ)\n\n" );
		} else {
			wprintf( L"※ ライブラリ側のバグの可能性があります ※\n" );
		}

		wprintf( L"具体的に、以下のようにサイズ調整がされます。\n" );

		wprintf( L"\tHSSBCreateReadOnlyMemoryBufferに渡したバッファの実際の確保サイズ：%zu\n", BufferAllocateSize );
		wprintf( L"\tHSSBCreateReadOnlyMemoryBufferに渡したサイズ：%zu\n", OverSizedNotifySize );
		wprintf( L"\tHSSBCreateReadOnlyMemoryBufferが認識しているサイズ：%zu\n\n", readOnlyBuffer->GetSize( ) );

	} else {
		wprintf( L"※ ライブラリ側のバグの可能性があります ※\n" );
	}

	wprintf( L"\n" );

	// IHSSBReadOnlyMemoryBufferは、内部でサイズ情報を保有しており、
	// 呼び出し先で確認できるため、安全に渡せます。
	// ここでは、例として、読み取り専用メモリバッファの内容をダンプする関数を呼びます
	Sample_ReadOnlyMemoryBuffer_DumpData( readOnlyBuffer );

	wprintf( L"\n" );


	// CComPtrによって、Releaseが自動で呼ばれるため、解放不要
	// 今回移譲したバッファは、HeapAllocで確保したものであることを通知していたため、
	// 参照カウンタが0になり、IHSSBReadOnlyMemoryBufferが解放される際に
	// HeapFreeが呼ばれ、移譲したバッファが自動で解放されます。
}

void Sample_ReadOnlyMemoryBuffer_DumpData( IHSSBReadOnlyMemoryBuffer* pBuffer ) {

	if ( !pBuffer ) {
		wprintf( L"pBufferはnullptrです\n" );
		return;
	}

	// バッファサイズを取得
	const size_t BufferSize = pBuffer->GetSize( );
	wprintf( L"読み取り専用メモリバッファのサイズ: %zu バイト\n", BufferSize );
	if ( BufferSize == 0 ) {
		// サイズが0の場合はダンプしない。
		return;
	}

	// IsValidElementNumberを利用して、インデックスが有効かを確認できます。
	if ( pBuffer->IsValidElementNumber( 0 ) ) {

		// 先頭のバッファポインタをconst uint8_t* で取得
		// GetConstBufferPointerTypeが実装されており、別途型キャストが不要
		// なお、インデックスが有効であることを先に、チェック済みのためnullptrチェックは無くてよい
		// 内部実装上、IsValidElementNumberが内部で呼び出されており、有効であれば、
		// 有効なバッファが返ります。
		// ただし、IsValidElementNumberがfalseを返すケースの場合、nullptrが返るので、
		// IsValidElementNumberで事前確認をしない場合はnullptrチェックを推奨します。
		const uint8_t* pData = pBuffer->GetConstBufferPointerType<uint8_t>( 0 );


		// 事前にインデックスの有効性を確認してからバッファポインタを取得したため、nullptrチェックは省略

		// データ内容のダンプを実施
		const size_t MaxDumpSize = 2048;
		const size_t DumpBytesSizeByLine = 16;

		const size_t RealDumpSize = std::min( BufferSize, MaxDumpSize );
		wprintf( L"データ内容 (最大ダンプサイズ：%zuバイト)\n", MaxDumpSize );
		wprintf( L"\t" );
		for ( size_t i = 0; i < RealDumpSize; ++i ) {
			wprintf( L"%02X ", pData[i] );
			if ( ( ( i + 1 ) % DumpBytesSizeByLine ) == 0 ) {
				wprintf( L"\n\t" );
			}
		}
	}
	wprintf( L"\n" );
}
