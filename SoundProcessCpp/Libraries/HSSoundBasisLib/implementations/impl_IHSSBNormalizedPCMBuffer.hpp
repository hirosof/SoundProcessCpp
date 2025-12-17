#pragma once
#define HSSOUNDBASISLIB_EXPORT_DLL_INTERNAL
#include "../HSSoundBasisLib.hpp"
#include <atlbase.h>
#include <mutex>
#include <atomic>

class impl_IHSSBNormalizedPCMBuffer : public IHSSBNormalizedPCMBuffer {
private:

	mutable volatile LONG m_ref;

	CComPtr<IHSSBMemoryBuffer> m_MemoryBuffer;
	
    std::atomic<uint8_t> m_Channels;
    std::atomic<size_t> m_NumberOfSamples;
    mutable std::recursive_mutex m_Mutex;

	impl_IHSSBNormalizedPCMBuffer( );
	~impl_IHSSBNormalizedPCMBuffer( );

public:

	static HRESULT CreateInstance( IHSSBNormalizedPCMBuffer** ppBuffer);

	// IUnknown
	virtual HRESULT STDMETHODCALLTYPE QueryInterface( REFIID riid, _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject ) override;
	virtual ULONG STDMETHODCALLTYPE AddRef( void ) override;
	virtual ULONG STDMETHODCALLTYPE Release( void ) override;

	// IHSSBBase / Extra service
	virtual bool InquiryProvided( REFIID TargetIID ) const override;
	virtual bool InquiryProvidedExtraService( REFIID TargetIID ) const override;
	virtual HRESULT QueryExtraService( REFIID riid, void** ppvObject ) override;


	// IHSSBNormalizedPCMBuffer
	virtual size_t GetBytesSize( void ) const override;

	virtual  HRESULT Initialize( size_t number_of_samples, uint8_t number_of_channels = 1 ) override;
	virtual  HRESULT Initialize( uint32_t sampling_frequency, uint32_t number_of_seconds, uint8_t number_of_channels = 1 ) override;
	virtual  HRESULT Initialize( uint32_t sampling_frequency, double number_of_seconds, uint8_t number_of_channels = 1, EHSSB_RoundMode  round_mode = EHSSB_RoundMode::Down )override;
	virtual uint8_t GetNumberOfChannels( void ) const override;
	virtual size_t GetNumberOfSamples( void ) const override;

	virtual HRESULT GetBytesIndex( size_t* pOutBytesIndex, size_t sample_index, uint8_t channel_index ) const override;
	virtual HRESULT GetIndex( size_t* pOutIndex, size_t sample_index, uint8_t channel_index ) const override;
	virtual HRESULT GetValue( double* pOutValue, size_t sample_index, uint8_t channel_index ) const override ;
	virtual HRESULT SetValue( double value, size_t sample_index, uint8_t channel_index ) override;

	virtual HRESULT CreateEmptyChannelBuffer( IHSSBWritableMemoryBuffer** ppBuffer ) const override;
	virtual HRESULT CreateChannelBuffer( IHSSBWritableMemoryBuffer** ppBuffer, uint8_t channel_index ) const override;

	virtual HRESULT ExportChannelData( IHSSBWritableMemoryBuffer* pChannelBuffer, uint8_t channel_index ) const override;
	virtual HRESULT ExportChannelData( IHSSBWritableMemoryBuffer* pChannelBuffer, ExportImportSettings* pSettings, uint8_t channel_index ) const override;

	virtual HRESULT ImportChannelData( IHSSBReadOnlyMemoryBuffer* pChannelBuffer, uint8_t channel_index ) override;
	virtual HRESULT ImportChannelData( IHSSBReadOnlyMemoryBuffer* pChannelBuffer,  ExportImportSettings* pSettings, uint8_t channel_index ) override;

};
