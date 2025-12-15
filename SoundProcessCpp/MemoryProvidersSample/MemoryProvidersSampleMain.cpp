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

// ※ HSSoundBasisLib.hppをインクルードすることによって、Windows.hもインクルードされます
// ※ ただし、HSSoundBasisLib.hppが直接Windows.hをインクルードしているわけではなく別のヘッダーによって読み込まれます
#include "../HSSoundBasisLib/HSSoundBasisLib.hpp"

void Sample_ReadOnlyMemoryBuffer( void );
void Sample_ReadOnlyMemoryBuffer_DumpData( IHSSBReadOnlyMemoryBuffer *pBuffer );

int main( void ) {

	setlocale( LC_ALL, "Japanese" );

	// 読み取り専用メモリバッファのサンプル実行
	Sample_ReadOnlyMemoryBuffer( );

	return 0;
}

void Sample_ReadOnlyMemoryBuffer( void ) {


	// IHSSBReadOnlyMemoryBufferはIUnknownを実装しているため、CComPtrを使用できる
	CComPtr<IHSSBReadOnlyMemoryBuffer> readOnlyBuffer;

	printf( "=== 読み取り専用メモリバッファのサンプル ===\n" );

	// メモリ確保サイズ
	size_t BufferAllocateSize = 512;

	// メモリを確保
	// ※ HeapAllocのメモリを移譲する場合、HeapハンドルはGetProcessHeapで取得する必要があります。
	void* pBuffer = HeapAlloc( GetProcessHeap( ), 0, BufferAllocateSize );

	// 確保に失敗した場合は終了
	if ( !pBuffer ) {
		printf( "HeapAlloc 失敗\n" );
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
		printf( "HSSBCreateReadOnlyMemoryBuffer 失敗. hr=0x%08X\n", hr );
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
		printf( "HSSBCreateReadOnlyMemoryBuffer Result hr = 0x%08X\n", hr );
		printf( "HSSB_S_OK_BUT_MANAGED_SIZE_ADJUSTED = 0x%08X\n\n", HSSB_S_OK_BUT_MANAGED_SIZE_ADJUSTED );

		if ( hr == HSSB_S_OK_BUT_MANAGED_SIZE_ADJUSTED ) {
			printf( "\tこのように、実際のサイズより大きいサイズを指定した場合、\n" );
			printf( "\tHSSB_S_BUT_MANAGED_SIZE_ADJUSTEDが返ります。 \n" );
			printf( "\t(HeapAllocで確保したものを移譲した場合のみ)\n\n" );
		} else {
			printf( "※ ライブラリ側のバグの可能性があります ※\n" );
		}

		printf( "具体的に、以下のようにサイズ調整がされます。\n" );

		printf( "\tHSSBCreateReadOnlyMemoryBufferに渡したバッファの実際の確保サイズ：%zu\n", BufferAllocateSize );
		printf( "\tHSSBCreateReadOnlyMemoryBufferに渡したサイズ：%zu\n", OverSizedNotifySize );
		printf( "\tHSSBCreateReadOnlyMemoryBufferが認識しているサイズ：%zu\n\n", readOnlyBuffer->GetSize( ) );

	} else {
		printf( "※ ライブラリ側のバグの可能性があります ※\n" );
	}

	printf( "\n" );

	// IHSSBReadOnlyMemoryBufferは、内部でサイズ情報を保有しており、
	// 呼び出し先で確認できるため、安全に渡せます。
	// ここでは、例として、読み取り専用メモリバッファの内容をダンプする関数を呼びます
	Sample_ReadOnlyMemoryBuffer_DumpData( readOnlyBuffer );

	printf( "\n" );


	// CComPtrによって、Releaseが自動で呼ばれるため、解放不要
	// 今回移譲したバッファは、HeapAllocで確保したものであることを通知していたため、
	// 参照カウンタが0になり、IHSSBReadOnlyMemoryBufferが解放される際に
	// HeapFreeが呼ばれ、移譲したバッファが自動で解放されます。
}

void Sample_ReadOnlyMemoryBuffer_DumpData( IHSSBReadOnlyMemoryBuffer* pBuffer ) {

	if ( !pBuffer ) {
		printf( "pBufferはnullptrです\n" );
		return;
	}

	// バッファサイズを取得
	const size_t BufferSize = pBuffer->GetSize( );
	printf( "読み取り専用メモリバッファのサイズ: %zu バイト\n", BufferSize );
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
		printf( "データ内容 (最大ダンプサイズ：%zuバイト)\n", MaxDumpSize );
		printf( "\t" );
		for ( size_t i = 0; i < RealDumpSize; ++i ) {
			printf( "%02X ", pData[i] );
			if ( ( ( i + 1 ) % DumpBytesSizeByLine ) == 0 ) {
				printf( "\n\t" );
			}
		}
	}
	printf( "\n" );
}

