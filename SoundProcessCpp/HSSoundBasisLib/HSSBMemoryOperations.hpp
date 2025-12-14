#pragma once

#include "HSSBCommon.hpp"


#define IIDSTR_IHSSBBase  "DE8D1C85-0807-455A-BA0E-2641F02E2DE2"
HSSOUNDBASISLIB_VAREXPORT const IID IID_IHSSBBase;
MIDL_INTERFACE( IIDSTR_IHSSBBase ) IHSSBBase : public IUnknown {


	virtual bool InquiryProvided( REFIID TargetIID ) const = 0;
	virtual bool InquiryProvidedExtraService( REFIID TargetIID ) const = 0;

	virtual HRESULT QueryExtraService( REFIID riid, void** ppvObject ) = 0;
	template <typename T> HRESULT QueryExtraService( T** ppvObject ) {
		return this->QueryExtraService( __uuidof( T ), reinterpret_cast<void**>( ppvObject ) );
	}

};



#define IIDSTR_IHSSBMemoryProvider "F9BC45C9-C7CC-456A-BDF7-A918BC5AA076"
HSSOUNDBASISLIB_VAREXPORT const IID IID_IHSSBMemoryProvider;
MIDL_INTERFACE( IIDSTR_IHSSBMemoryProvider ) IHSSBMemoryProvider : public IHSSBBase {

};



#define IIDSTR_IHSSBMemoryBufferBase "D24733D0-E5D4-4752-95F6-5A7952AD363D"
HSSOUNDBASISLIB_VAREXPORT const IID IID_IHSSBMemoryBufferBase;
MIDL_INTERFACE( IIDSTR_IHSSBMemoryBufferBase ) IHSSBMemoryBufferBase : public IHSSBMemoryProvider {


};



#define IIDSTR_IHSSBReadOnlyMemoryBuffer "F8EA8507-D00E-4F69-8399-0BD8D113A4C1"
HSSOUNDBASISLIB_VAREXPORT const IID IID_IHSSBReadOnlyMemoryBuffer;
MIDL_INTERFACE( IIDSTR_IHSSBReadOnlyMemoryBuffer ) IHSSBReadOnlyMemoryBuffer : public IHSSBMemoryBufferBase {

	virtual size_t GetSize( void ) const = 0;


	template <typename T> size_t GetSizeElements( void ) const {
		return this->GetSize( ) / sizeof( T );
	}

	template <typename T> bool  IsSizeByTypeSizeUnit( void )const {
		return ( ( this->GetSize( ) % sizeof( T ) ) == 0 );
	}

	virtual bool IsValidElementNumber( size_t offset ) const = 0;

	template <typename T> bool IsValidElementNumberType( size_t offsetElements ) const {
		return this->IsValidElementNumber( sizeof( T ) * ( offsetElements + 1 ) - 1 );
	}

	virtual const void* GetConstBufferPointer( size_t offset = 0 )const = 0;

	template <typename T> const T* GetConstBufferPointerType( size_t offset = 0 ) const {
		return reinterpret_cast<const T*>( this->GetConstBufferPointer( sizeof( T ) * offset ) );
	}

};



HSSOUNDBASISLIB_FUNCEXPORT HRESULT HSSBCreateReadOnlyMemoryBuffer( IHSSBReadOnlyMemoryBuffer** ppBuffer, const void* pTargetBuffer, size_t TargetBufferSize );

template <typename T> HRESULT HSSBCreateReadOnlyMemoryBufferType( IHSSBReadOnlyMemoryBuffer** ppBuffer, const T* pTargetBuffer, size_t TargetBufferElements ) {
	return HSSBCreateReadOnlyMemoryBuffer( ppBuffer, static_cast<const void*>( pTargetBuffer ), sizeof( T ) * TargetBufferElements );
}
template <typename T, size_t S> HRESULT  HSSBCreateReadOnlyMemoryBufferType( IHSSBReadOnlyMemoryBuffer** ppBuffer, const T( &targetBuffer )[S] ) {
	return HSSBCreateReadOnlyMemoryBufferType<T>( ppBuffer, targetBuffer, S );
}




#define IIDSTR_IHSSBWritableMemoryBuffer "621F6CB1-102A-41D2-93AD-8290EFDC3F37"
HSSOUNDBASISLIB_VAREXPORT const IID IID_IHSSBWritableMemoryBuffer;
MIDL_INTERFACE( IIDSTR_IHSSBWritableMemoryBuffer ) IHSSBWritableMemoryBuffer : public IHSSBReadOnlyMemoryBuffer {
	virtual void* GetBufferPointer( size_t offset = 0 )const = 0;
	template <typename T> T* GetBufferPointerType( size_t offset = 0 ) const {
		return reinterpret_cast<T*>( this->GetBufferPointer( sizeof( T ) * offset ) );
	}

};



HSSOUNDBASISLIB_FUNCEXPORT HRESULT HSSBCreateWritableMemoryBuffer( IHSSBWritableMemoryBuffer** ppBuffer, void* pTargetBuffer, size_t TargetBufferSize );
template <typename T> HRESULT HSSBCreateWritableMemoryBuffer( IHSSBWritableMemoryBuffer** ppBuffer, T* pTargetBuffer, size_t TargetBufferElemets ) {
	return HSSBCreateWritableMemoryBuffer( ppBuffer, static_cast<void*>( pTargetBuffer ), sizeof( T ) * TargetBufferElemets );
}
template <typename T, size_t S> HRESULT  HSSBCreateWritableMemoryBuffer( IHSSBWritableMemoryBuffer** ppBuffer, T( &targetBuffer )[S] ) {
	return HSSBCreateWritableMemoryBuffer<T>( ppBuffer, targetBuffer, S );
}


#define IIDSTR_IHSSBMemoryBuffer "4C5E9E72-2834-43FC-9F84-4795009C2B0B"
HSSOUNDBASISLIB_VAREXPORT const IID IID_IHSSBMemoryBuffer;
MIDL_INTERFACE( IIDSTR_IHSSBMemoryBuffer ) IHSSBMemoryBuffer : public IHSSBWritableMemoryBuffer {

	virtual bool IsAllocated( void ) const = 0;
	virtual HRESULT Allocate( size_t size ) = 0;
	virtual HRESULT Free( void ) = 0;
	virtual HRESULT ReAllocate( size_t new_size ) = 0;
	virtual HRESULT Prepare( size_t size, bool enable_reduce_allocate = false ) = 0;

	template <typename T> HRESULT AllocateElements( size_t elements ) {
		return this->Allocate( sizeof( T ) * elements );
	}
	template <typename T> HRESULT ReAllocateElements( size_t elements ) {
		return this->ReAllocate( sizeof( T ) * elements );
	}
	template <typename T> HRESULT PrepareElements( size_t elements, bool enable_reduce_allocate = false ) {
		return this->Prepare( sizeof( T ) * elements  , enable_reduce_allocate );
	}
};



HSSOUNDBASISLIB_FUNCEXPORT HRESULT HSSBCreateMemoryBuffer( IHSSBMemoryBuffer** ppBuffer );






#define IIDSTR_IHSSBMemoryStreamBase "BB453B42-1E08-413B-8A0C-0575EF6832C5"
HSSOUNDBASISLIB_VAREXPORT const IID IID_IHSSBMemoryStreamBase;
MIDL_INTERFACE( IIDSTR_IHSSBMemoryStreamBase ) IHSSBMemoryStreamBase : public IHSSBMemoryProvider {


};




#define IIDSTR_IHSSBMemoryReader "021E6D9E-335C-4172-8E5C-746D2B959156"
HSSOUNDBASISLIB_VAREXPORT const IID IID_IHSSBMemoryReader;


MIDL_INTERFACE( IIDSTR_IHSSBMemoryReader ) IHSSBMemoryReader : public IHSSBMemoryStreamBase {

	/*
	virtual char ReadChar( void ) = 0;
	virtual unsigned char ReadUnsignedChar( void ) = 0;
	virtual wchar_t ReadWChar( void ) = 0;

	virtual std::string ReadString( const char split = '\0' ) = 0;
	virtual std::wstring ReadWString( const wchar_t split = '\0' ) = 0;

	virtual std::string ReadLine( void ) = 0;
	virtual std::wstring ReadWLine( void ) = 0;

	virtual int8_t ReadInt8( void ) = 0;
	virtual uint8_t ReadUInt8( void ) = 0;

	virtual int16_t ReadInt16( void ) = 0;
	virtual uint16_t ReadUInt16( void ) = 0;

	virtual int32_t ReadInt32( void ) = 0;
	virtual uint32_t ReadUInt32( void ) = 0;

	virtual int64_t ReadInt64( void ) = 0;
	virtual uint64_t ReadUInt64( void ) = 0;

	virtual float ReadFloat( void ) = 0;
	virtual double ReadDouble( void ) = 0;
	*/


};



#define IIDSTR_IHSSBMemoryWriter "28284326-79D4-483D-9FFF-CA76A39C8949"
HSSOUNDBASISLIB_VAREXPORT const IID IID_IHSSBMemoryWriter;
MIDL_INTERFACE( IIDSTR_IHSSBMemoryWriter ) IHSSBMemoryWriter : public IHSSBMemoryStreamBase {

};




#define IIDSTR_IHSSBMemoryStream "866EC631-8F4C-40A8-9FB2-BF2381DF557B"
HSSOUNDBASISLIB_VAREXPORT const IID IID_IHSSBMemoryStream;
MIDL_INTERFACE( IIDSTR_IHSSBMemoryStream ) IHSSBMemoryStream : public IHSSBMemoryStreamBase {



};

HSSOUNDBASISLIB_FUNCEXPORT HRESULT HSSBCreateMemoryStream( IHSSBMemoryStream** ppBuffer );



#define IIDSTR_IHSSBNormalizedPCMBuffer "47F97C40-9D49-4D4B-8B06-C7AA04870CD9"
HSSOUNDBASISLIB_VAREXPORT const IID IID_IHSSBNormalizedPCMBuffer;
MIDL_INTERFACE( IIDSTR_IHSSBNormalizedPCMBuffer ) IHSSBNormalizedPCMBuffer : public IHSSBMemoryBufferBase {


	virtual  HRESULT Initialize( size_t number_of_samples, uint8_t number_of_channels = 1 ) = 0;
	virtual  HRESULT Initialize( uint32_t sampling_frequency, uint32_t number_of_seconds, uint8_t number_of_channels = 1 ) = 0;
	virtual  HRESULT Initialize( uint32_t sampling_frequency, double number_of_seconds, uint8_t number_of_channels = 1 , EHSSB_RoundMode  round_mode = EHSSB_RoundMode::Down ) = 0;


	virtual size_t GetBytesSize( void ) const = 0;

	virtual uint8_t GetNumberOfChannels( void ) const = 0;
	virtual size_t GetNumberOfSamples( void ) const = 0;

	virtual HRESULT GetBytesIndex( size_t* pOutBytesIndex, size_t sample_index, uint8_t channel_index ) const = 0;
	virtual HRESULT GetIndex( size_t* pOutIndex, size_t sample_index, uint8_t channel_index ) const = 0;

	virtual HRESULT GetValue( double* pOutValue, size_t sample_index, uint8_t channel_index ) const = 0;
	virtual HRESULT SetValue( double value, size_t sample_index, uint8_t channel_index ) = 0;

	virtual HRESULT CreateEmptyChannelBuffer( IHSSBWritableMemoryBuffer** ppBuffer ) const = 0;
	virtual HRESULT CreateChannelBuffer( IHSSBWritableMemoryBuffer** ppBuffer , uint8_t channel_index ) const = 0;

	
	// エクスポート/インポート時の設定構造体
	struct ExportImportSettings {
		size_t sample_length;
		size_t from_side_sample_start_index;
		size_t to_side_sample_start_index;
		size_t result_processed_sample_length;

		ExportImportSettings( )
			: sample_length( 0 )
			, from_side_sample_start_index( 0 )
			, to_side_sample_start_index( 0 ) 
			, result_processed_sample_length( 0 ){
		}

		explicit ExportImportSettings( size_t length, size_t from_index, size_t to_index )
			: sample_length( length )
			, from_side_sample_start_index( from_index )
			, to_side_sample_start_index( to_index )
			, result_processed_sample_length(0){
		
		}

	};


	virtual HRESULT ExportChannelData( IHSSBWritableMemoryBuffer* pChannelBuffer, uint8_t channel_index ) const = 0;
	virtual HRESULT ExportChannelData( IHSSBWritableMemoryBuffer* pChannelBuffer, ExportImportSettings *pSettings , uint8_t channel_index ) const = 0;

	virtual HRESULT ImportChannelData( IHSSBReadOnlyMemoryBuffer* pChannelBuffer, uint8_t channel_index ) = 0;
	virtual HRESULT ImportChannelData( IHSSBReadOnlyMemoryBuffer* pChannelBuffer,ExportImportSettings* pSettings, uint8_t channel_index ) = 0;



};

HSSOUNDBASISLIB_FUNCEXPORT HRESULT HSSBCreateNormalizedPCMBuffer( IHSSBNormalizedPCMBuffer** ppBuffer );
