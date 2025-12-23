#define NOMINMAX
#include <windows.h>
#include <Shlobj.h>
#include <locale>
#include <string>

// ハンドルのクローズ漏れがないように、CHandleAutoCloserクラスを実装
class CHandleAutoCloser {
private:

	HANDLE m_Handle = NULL;

public:
	CHandleAutoCloser( HANDLE handle ) {
		this->m_Handle = handle;
	}

	~CHandleAutoCloser( ) {
		if ( ( this->m_Handle != NULL ) && ( this->m_Handle != INVALID_HANDLE_VALUE ) ) {
			CloseHandle( this->m_Handle );
		}
	}

	// 複製を防止するため、コピーコンストラクタと代入演算子を削除
	CHandleAutoCloser( const CHandleAutoCloser& ) = delete;
	CHandleAutoCloser& operator=( const CHandleAutoCloser& ) = delete;

};


// エントリーポイント
int main( ) {

	// 日本語ロケールに設定
	setlocale( LC_ALL, "ja_JP.Utf-8" );

	// UTF-8コードページに変更
	SetConsoleOutputCP( CP_UTF8 );

	std::wstring ws_title;
	if ( IsUserAnAdmin( ) ) {
		ws_title.append( L"[管理者] " );
	}
	ws_title.append( L"二重起動検出方法サンプル" );
	SetConsoleTitleW( ws_title.c_str( ) );

	wprintf( L"[二重起動検出方法サンプル]\n\n" );
	wprintf( L"二重起動チェックを行うためにミューテックスを取得します。\n" );


	// ミューテックスの作成準備を実施
	std::wstring mutexName;  //作成するミューテックス名
	bool mutexGlobalMode = false;
	if ( MessageBoxW( GetConsoleWindow( ), L"作成するミューテックスをグローバルにしますか？",
		L"二重起動検出方法サンプル", MB_YESNO | MB_ICONQUESTION ) == IDYES ) {
		mutexName.append( L"Global\\" );
		mutexGlobalMode = true;
		wprintf( L"作成するミューテックスはグローバルです。\n" );
	} else {
		wprintf( L"作成するミューテックスはローカルです。\n" );
	}
	mutexName.append( L"DualRunPreventSample" );

	BOOL	mutexTakeOwner = FALSE;
	if ( MessageBoxW( GetConsoleWindow( ), L"作成するミューテックスの所有権を要求しますか？",
		L"二重起動検出方法サンプル", MB_YESNO | MB_ICONQUESTION) == IDYES ) {
		mutexTakeOwner = TRUE;
		wprintf( L"作成するミューテックスの所有権を要求します。\n" );
	} else {
		wprintf( L"作成するミューテックスの所有権を要求しません。\n" );
	}

	// ミューテックスを作成する
	//
	// 第1パラメータは継承のためのセキュリティ記述子を指定するものだが、使用しないのでNULLで問題なし
	// 第2パラメータをTRUEにすることで、初回作成時の時、所有権を取得
	// 第3パラメータでコンピュータ上で一位となるように、名前を指定する
	//		なお、名前の前に『Global\』をつけることで、セッション(ユーザー)をまたいで、1つにできる
	HANDLE hMutex = CreateMutexW( NULL, mutexTakeOwner, mutexName.c_str( ) );

	// CeateMutex関数は、指定した名前に一致したミューテックスが存在した場合、
	// 存在したミューテックスを示すハンドルを返し、成功します。
	//
	// すでに、ミューテックスが存在しているかを確かめるには、GetLastError関数を呼んで拡張エラー情報の取得が必要となる
	// なお、他のAPIを呼んだ場合、拡張エラー情報がその関数のものに上書きされる可能性があるため、
	// CreateMutex関数を呼んだ直後に取得すること。
	DWORD dwMutexCreatedError = GetLastError( );


	if ( hMutex == NULL ) {
		// hMutexがNULLの場合ミューテックスの作成に失敗したことを示す。
		wprintf( L"ミューテックスの作成に失敗しました。\n" );
		wprintf( L"Win32エラーコード：0x%08X (%u)\n", dwMutexCreatedError, dwMutexCreatedError );

		//コンソールアプリケーションの場合、すぐに終了してしまうので、メッセージボックスを表示する。
		MessageBoxW( GetConsoleWindow( ), L"OKを押すとプログラムを終了します。", L"二重起動検出方法サンプル", MB_OK );
		return 0;
	}

	// 以後、ハンドルのクローズはCHandleAutoCloserに委任 
	CHandleAutoCloser mutexHandleAutoCloser( hMutex );

	if ( dwMutexCreatedError == ERROR_ALREADY_EXISTS ) {

		// dwMutexCreatedErrorがERROR_ALREADY_EXISTSの場合、
		// すでに、ミューテックスが存在していたことを示す。
		//
		// これは、すでにミューテックスを作成していたプロセスが存在していることを示し、
		// 本プログラムが既に起動している = 本プロセスが少なくとも2つ目の起動 であることを示す。
		//
		// 二重起動を防止したい場合はこのタイミングで、必要に応じてメッセージを表示した後に、
		// プログラムの終了を行う。
		wprintf( L"二重起動を検出しました。プログラムを終了します。\n" );

		//コンソールアプリケーションの場合、すぐに終了してしまうので、メッセージボックスを表示する。
		MessageBoxW( GetConsoleWindow( ), L"OKを押すとプログラムを終了します。", L"二重起動検出方法サンプル", MB_OK );
		return 0;
	}

	// 以降は通常通り、プログラムの処理を行う
	// 
	// ここでは、単純にメッセージを表示します。
	wprintf( L"本プロセスは初回起動です。\n\n" );

	//コンソールアプリケーションの場合、すぐに終了してしまうので、メッセージボックスを表示する。
	std::wstring wsMessage = L"OKを押すとプログラムを終了します。\n\n";
	wsMessage.append( L"なお、このメッセージが表示されている状態で本プログラムを別に起動すると\n" );
	wsMessage.append( L"二重起動されている場合の挙動を確認できます。" );

	if ( IsUserAnAdmin( ) && mutexGlobalMode ) {
		// 管理者権限実行時には注意事項をメッセージに追加
		wsMessage.append( L"\n\n【注意事項】\n" );
		wsMessage.append( L"現在、本プロセスは管理者権限で実行しており、グローバルなミューテックスが作成されております。\n\n" );
		wsMessage.append( L"そのため、本プログラムを別に起動する際、通常ユーザー権限で実行すると、\n" );
		wsMessage.append( L"CreateMutex関数は失敗する場合があります。\n\n" );
		wsMessage.append( L"その場合、GetLastError関数はERROR_ACCESS_DENIEDを返します。\n\n" );
		wsMessage.append( L"これは、ミューテックス作成時にセキュリティ記述子を指定していないのが要因となります。" );
	}

	MessageBoxW( GetConsoleWindow( ), wsMessage.c_str( ), L"二重起動検出方法サンプル", MB_OK );

	return 0;
}