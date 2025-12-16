#pragma once

#include <Windows.h>
#include <cstdint>
#include <cstdio>
#include <string>

#define _USE_MATH_DEFINES
#include <math.h>
#include <cmath>


#ifdef HSSOUNDBASISLIB_EXPORTS


	#ifdef  HSSOUNDBASISLIB_EXPORT_DLL_INTERNAL

		#define HSSOUNDBASISLIB_VAREXPORT extern
		#define HSSOUNDBASISLIB_FUNCEXPORT extern 

	#else

		#define HSSOUNDBASISLIB_VAREXPORT extern __declspec (dllexport) 
		#define HSSOUNDBASISLIB_FUNCEXPORT extern __declspec (dllexport) 

	#endif

#else

	#define HSSOUNDBASISLIB_FUNCEXPORT extern __declspec (dllimport) 
	#define HSSOUNDBASISLIB_VAREXPORT extern __declspec(dllimport) 

#endif


enum struct EHSSB_AddressOrigin {
	//先頭
	Benign = 0,

	// 現在位置
	Current,

	// 終端
	End
};


enum struct EHSSB_RoundMode {

	// 切り捨て
	Down = 0,

	// 切り上げ
	Up,

	// 四捨五入
	Nearest

};


// メモリ所有権タイプ
enum struct EHSSBMemoryOwnershipType {
	// 所有権なし
	NoOwnership = 0,

	//所有権あり、該当のメモリはnew[]で確保されたものである
	//   → delete[]で解放されるべき
	WithDeleteArrayOwnership_NewAllocated,

	//所有権あり、該当のメモリはmallocで確保されたものである
	//   → freeで解放されるべき
	WithFreeOwnership_Malloced,

	//所有権あり、該当のメモリはHeapAlloc(GetProcessHeap(), ...)で確保されたものである
	//   → HeapFree(GetProcessHeap(), 0, ...)で解放されるべき
	WithHeapFreeOwnership_HeapAlloced
};


// new で確保されたメモリの型情報
enum struct EHSSBMemoryNewAllocatedTypeInfo {

	// 未指定
	None = 0,

	// char 配列
	char_array,

	// wchar_t 配列
	wchar_t_array,

	// float 配列
	float_array,
	
	// double 配列
	double_array,

	// int8_t 配列
	int8_t_array,

	// int16_t 配列
	int16_t_array,

	// int32_t 配列
	int32_t_array,

	// int64_t 配列
	int64_t_array,

	// uint8_t 配列
	uint8_t_array,

	// uint16_t 配列
	uint16_t_array,

	// uint32_t 配列
	uint32_t_array,

	// uint64_t 配列
	uint64_t_array
};


//未初期化時を表すカスタムHRESULTエラーコード
HSSOUNDBASISLIB_VAREXPORT const HRESULT HSSB_E_NOT_INITIALIZED;

//部分的成功を表すカスタムHRESULTコード
HSSOUNDBASISLIB_VAREXPORT const HRESULT HSSB_S_PARTIAL;

// 部分的成功：入力側に要因があった場合
HSSOUNDBASISLIB_VAREXPORT const HRESULT HSSB_S_PARTIAL_INPUT_SIDE_FACTOR;

// 部分的成功：出力側に要因があった場合
HSSOUNDBASISLIB_VAREXPORT const HRESULT HSSB_S_PARTIAL_OUTPUT_SIDE_FACTOR;

// バグ起因とみられる処理エラー表すカスタムHRESULTエラーコード
HSSOUNDBASISLIB_VAREXPORT const HRESULT HSSB_E_PROCESS_ERROR_BY_BUG_FACTOR;

// 処理には成功したが、管理サイズが調整されたことを表すカスタムHRESULTコード
HSSOUNDBASISLIB_VAREXPORT const HRESULT HSSB_S_OK_BUT_MANAGED_SIZE_ADJUSTED;

// 本ライブラリ上における基本インターフェース
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