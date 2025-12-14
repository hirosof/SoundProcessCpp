#include <cstdio>
#include <cstdint>
#include <locale>
#include <random>
#include <atlbase.h>

#include "../HSSoundBasisLib/HSSoundBasisLib.hpp"

struct W24 {
	uint8_t s[3];
};


void Sample_MemoryBuffer( void );
void Sample_ReadOnlyMemoryBuffer( void );
void Sample_WritableMemoryBuffer( void );
void Sample_MemoryStream_InternalBuffer( void );
void Sample_MemoryStream_ExternalBuffer( void );

int main( void ) {

	setlocale( LC_ALL, "Japanese" );

	CComPtr<IHSSBNormalizedPCMBuffer> pPCMBuf = nullptr;
	
	HRESULT hr = HSSBCreateNormalizedPCMBuffer( &pPCMBuf );
	if ( FAILED( hr ) ) {
		printf( "HSSBCreateNormalizedPCMBuffer failed : 0x%08X\n", hr );
		return -1;
	}

	// ƒTƒ“ƒvƒŠƒ“ƒOŽü”g”: 32Hz , •b”: 1•b , ƒ`ƒƒƒ“ƒlƒ‹”: 2ch , Ø‚èã‚°
	hr = pPCMBuf->Initialize( 32, 1, 2, EHSSB_RoundMode::Up );

	if ( FAILED( hr ) ) {
		printf( "IHSSBNormalizedPCMBuffer::Initialize failed : 0x%08X\n", hr );
		return -1;
	}

	printf( "Buffer Size : %zu\n", pPCMBuf->GetBytesSize( ) )	;

	CComPtr< IHSSBWritableMemoryBuffer> pLeftChannelBuffer;
	hr = pPCMBuf->CreateEmptyChannelBuffer( &pLeftChannelBuffer );

	if ( FAILED( hr ) ) {
		printf( "IHSSBNormalizedPCMBuffer::CreateEmptyChannelBuffer failed : 0x%08X\n", hr );
		return -1;
	}

	printf( "Empty Buffer Size : %zu\n", pLeftChannelBuffer->GetSize( ) );



	return 0;
}

void Sample_ReadOnlyMemoryBuffer( void ) {
	int8_t  val[256];
	for ( int i = 0; i < 256; i++ ) {
		val[i] = 127 - i;
	}
	CComPtr<IHSSBReadOnlyMemoryBuffer> pBuffer;
	if ( SUCCEEDED( HSSBCreateReadOnlyMemoryBufferType( &pBuffer, val ) ) ) {
		size_t i = 0;
		const int8_t* pVal;
		while ( ( pVal = pBuffer->GetConstBufferPointerType<int8_t>( i ) ) != nullptr ) {
			printf( "%d\n", *pVal );
			i++;
		}
	}
}

void Sample_WritableMemoryBuffer( void ) {
	CComPtr<IHSSBWritableMemoryBuffer> pWritableBuffer;
	int8_t  val[256];
	if ( SUCCEEDED( HSSBCreateWritableMemoryBuffer( &pWritableBuffer, val ) ) ) {
		size_t i = 0;
		int8_t* pVal;
		while ( ( pVal = pWritableBuffer->GetBufferPointerType<int8_t>( i ) ) != nullptr ) {
			*pVal = static_cast<int8_t>( -128 + i );
			i++;
		}
		for ( i = 0; i < 256; i++ ) {
			printf( "val[%3zu] = %d\n", i, val[i] );
		}
	}
}


void Sample_MemoryBuffer( void ) {

#if 0
	CComPtr< IHSSBMemoryBuffer> pMemoryBuffer;

	HRESULT hr = HSSBCreateMemoryBuffer( &pMemoryBuffer );

	if ( FAILED( hr ) ) return;

	size_t s = 128;

	if ( SUCCEEDED( pMemoryBuffer->PrepareElements<double>( s ) ) ) {
		printf( "allocated size : %zu\n", pMemoryBuffer->GetAllocatedSize( ) );
		printf( "allocated size type : %zu\n\n", pMemoryBuffer->GetAllocatedElements<double>( ) );
		for ( size_t i = 0; i <= s; i++ ) {
			printf( "%4zu (%04zu): %s\n", i, i * sizeof( double ),
				( pMemoryBuffer->IsValidOffsetType<double>( i ) ) ? "—LŒø" : "–³Œø" );
		}
	}
#endif

}

void Sample_MemoryStream_InternalBuffer( void ) {

#if 0

	CComPtr<IHSSBMemoryStream> pStream;

	HRESULT hr = HSSBCreateMemoryStream( &pStream );

	if ( FAILED( hr ) ) return;

	hr = pStream->Initialize(8);

	if ( FAILED( hr ) ) return;

	uint8_t v[] = { 0x2f , 0x3a , 0xab , 0x0c };

	printf( "Write Before StreamSize = %zu\n", pStream->GetSize( ) );
	pStream->SeekTo( 0, EHSSB_AddressOrigin::End );
	pStream->Write( v, sizeof( v )  , EHSSB_WriteProcessModeForInsufficient::AutoResize);
	printf( "Write After StreamSize = %zu\n", pStream->GetSize( ) );

	CComPtr<IHSSBReadOnlyMemoryBuffer> pBuffer;
	if ( SUCCEEDED( pStream.QueryInterface( &pBuffer ) ) ) {
		const uint8_t* pBufferPointer = pBuffer->GetConstBufferPointerType<uint8_t>( );
		size_t size = pBuffer->GetAllocatedSize( );
		for ( size_t i = 0; i < size; i++ ) {
			printf( "%zu : %3u (0x%02X) \n", i, *( pBufferPointer + i ), *( pBufferPointer + i ) );
		}
	}
#endif 
}


void Sample_MemoryStream_ExternalBuffer( void ) {

#if 0
	CComPtr<IHSSBMemoryBuffer> pBuffer;

	HRESULT hr;


	hr = HSSBCreateMemoryBuffer( &pBuffer );
	if ( FAILED( hr ) ) return;


	hr = pBuffer->AllocateElements<uint32_t>( 4 );
	if ( FAILED( hr ) ) return;


	std::random_device rd;
	std::default_random_engine dre( rd( ) );

	size_t idx = 0;
	printf( "[prepare]\n" );
	while ( pBuffer->IsValidOffsetType<uint32_t>( idx ) ) {
		uint32_t* pCurrent= pBuffer->GetBufferOffsetType<uint32_t>( idx );
		if ( pCurrent ) {
			*pCurrent = dre( ); 

			printf( "%zu : %u\n",idx,  *pCurrent );
		}
		idx++;
	}


	CComPtr<IHSSBMemoryStream> pStream;
	hr = HSSBCreateMemoryStream( &pStream );
	if ( FAILED( hr ) ) return;

	hr = pStream->Initialize( pBuffer );
	if ( FAILED( hr ) ) return;


	uint32_t b;
	size_t count = 0 , pos , rest;
	printf( "\n[read]\n" );

	CComPtr<IHSSBMemoryBuffer> buf;

	HSSBCreateMemoryBuffer( &buf );

	if ( SUCCEEDED( buf->Allocate( 1 ) ) ) {


		while ( pStream->IsReachEndOffset( ) ) {

			pos = pStream->GetCurrentOffsets( );
			rest = pStream->GetCurrentOffsetsFromReachEnd( );

			if ( pStream->Read( buf ) == 1 ) {
				printf( "%zu (%zu %zu %zu) : %u\n", count,  pos ,rest , pStream->GetCurrentOffsetsFromReachEnd(), *buf->GetConstBufferPointerType<uint8_t>( ) );
			}

			count++;
		}
	}

	/*
	while ( pStream->CanAccessElements<uint32_t>( 1 ) ) {
		b = 0;

		pos = pStream->GetCurrentOffsets( );

		if ( pStream->ReadElements<uint32_t>( &b, 1 ) == 1 ) {
			printf( "%zu (%zu) : %u\n",count , pos, b );
		}
		count++;
	}
	*/

#endif


}