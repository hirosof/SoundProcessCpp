#include <cstdio>
#include <cstdint>
#include <iostream>
#include <locale>
#include <Windows.h>
#include <vector>
#include <string>
#include <atlbase.h>
#include <atlstr.h>

#pragma comment(lib,"Rpcrt4.lib")

struct GenItem {
	std::string class_name;
	UUID iid;

	std::string iid_str;
	std::string iid_str_macro_name;
	std::string iidval_variable_name;

	std::string code_iid_str_macro_declare;

	std::string code_iidval_declare;
	std::string code_class_define;

	std::string code_iidval_value;


	GenItem( ) :class_name( "" ), iid( IID_NULL ) {

		iid_str_macro_name.clear( );
		code_iidval_declare.clear( );
		code_iidval_value.clear( );
		code_class_define.clear( );
		code_iid_str_macro_declare.clear( );
		iidval_variable_name.clear( );

	}
};

std::string UuidToStdStringA( const UUID uuid );

int main( int argc, char* argv[] ){

	setlocale( LC_ALL, "Japanese" );
    SetConsoleOutputCP( 65001 ); // UTF-8コードページに変更

	std::vector<GenItem> itemVector;

	if ( argc == 1 ) {

        wprintf( L"コマンドライン引数でクラス名が指定されませんでした。入力してください。\n" );

		std::string namestr;

		GenItem item;

		while ( 1 ) {

			namestr.clear( );
			std::getline( std::cin, namestr );

			if ( namestr.empty( ) ) break;

			item.class_name = namestr;
			itemVector.push_back( item );

			wprintf( L"\n追加のクラスがある場合はクラス名を入力してください。\n終了する場合は文字列を入力せずにそのままEnterキーを押してください。\n" );

		}

		if ( itemVector.empty( ) ) {
			wprintf( L"クラス名が一つも入力されませんでした。終了します。\n" );
			return 0;
		}

	} else {
		for ( int i = 1; i < argc; i++ ) {
			GenItem item;
			item.class_name = argv[i];
			itemVector.push_back( item );
		}
	}

	wprintf( L"\n" );

	CAtlStringA strgen;

	for ( GenItem& item : itemVector ) {
		(void)UuidCreate( &item.iid );
		if ( item.iid == IID_NULL ) {
			wprintf( L"UUIDの作成に失敗しました。終了します。\n" );
			return 0;
		}


		item.iid_str = UuidToStdStringA( item.iid );

		item.iid_str_macro_name = "IIDSTR_";
		item.iid_str_macro_name.append( item.class_name );

		item.iidval_variable_name = "IID_";
		item.iidval_variable_name.append( item.class_name ); 


		item.code_iid_str_macro_declare = "#define ";
		item.code_iid_str_macro_declare.append( item.iid_str_macro_name );
		item.code_iid_str_macro_declare.push_back( ' ' );
		item.code_iid_str_macro_declare.push_back( '\"' );
		item.code_iid_str_macro_declare.append( item.iid_str );
		item.code_iid_str_macro_declare.push_back( '\"' );


		item.code_iidval_declare = "const IID";
		item.code_iidval_declare.push_back( ' ' );
		item.code_iidval_declare.append( item.iidval_variable_name );

		strgen.Format( "MIDL_INTERFACE( %s ) %s : public IHSSBBase {\n\n};", item.iid_str_macro_name.c_str( ), item.class_name.c_str( ) );
		item.code_class_define = strgen.GetString( );


	}

	wprintf( L"/*\n\t.hpp File Code\n*/\n\n" );

	for (const GenItem& item : itemVector ) {

    	wprintf( L"%S\n", item.code_iid_str_macro_declare.c_str( ) );
		wprintf( L"HSSOUNDBASISLIB_VAREXPORT %S;\n", item.code_iidval_declare.c_str( ) );
		wprintf( L"%S\n\n", item.code_class_define.c_str( ) );
	}


	wprintf( L"\n/*\n\t.cpp File Code\n*/\n\n" );

	for (const GenItem& item : itemVector ) {
		wprintf( L"%S = __uuidof(%S);\n", item.code_iidval_declare.c_str( ) , item.class_name.c_str() );
	}

	wprintf( L"\n" );
	system( "pause" );

	return 0;
}


std::string UuidToStdStringA( const UUID uuid ) {
	CAtlStringA str;
	str.AppendFormat( "%08X", uuid.Data1 );
	str.AppendFormat( "-%04X", uuid.Data2 );
	str.AppendFormat( "-%04X", uuid.Data3 );
	str.AppendFormat( "-%02X%02X-", uuid.Data4[0], uuid.Data4[1]);

	for ( int i = 2; i < 8; i++ ) {
		str.AppendFormat( "%02X", uuid.Data4[i] );
	}

	return std::string(str.GetString( ));
}
