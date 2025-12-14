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

