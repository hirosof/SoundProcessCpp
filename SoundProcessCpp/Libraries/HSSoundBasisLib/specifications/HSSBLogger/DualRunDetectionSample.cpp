#define NOMINMAX
#include <windows.h>
#include <locale>
#include <TlHelp32.h>
#include <shlwapi.h>

#pragma	comment( lib, "shlwapi.lib" )

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


void ShowQuitMessageDialog( void );

bool  GetMyselfExecuteFileProcessCount( bool exclude_myself_process, uint32_t* pCount, uint32_t* pSuspicionCount );

int main( ) {

	// 日本語ロケールに設定
	setlocale( LC_ALL, "ja_JP.Utf-8" );

	// UTF-8コードページに変更
	SetConsoleOutputCP( CP_UTF8 );


	uint32_t count, suspicious_count;

	if ( GetMyselfExecuteFileProcessCount( true, &count, &suspicious_count ) ) {

		if ( count > 0 ) {
			wprintf( L"多重起動を検出しました。プログラムを終了します。\n" );
			wprintf( L"なお、現在本プロセス以外で%u個(+ 疑惑%u個)のプロセスが起動しております。\n", count, suspicious_count );
			ShowQuitMessageDialog( );
			return 0;
		}

		if ( suspicious_count > 0 ) {
			wprintf( L"多重起動している可能性を検出しました。プログラムを終了します。\n" );
			wprintf( L"なお、現在本プロセス以外で%u個のプロセスが本プログラムの可能性があります。\n", suspicious_count );
			ShowQuitMessageDialog( );
			return 0;
		}

		wprintf( L"多重起動はしておりませんでした。プログラムを終了します。\n" );
		ShowQuitMessageDialog( );

	} else {
		wprintf( L"自分自身のプロセスのカウントに失敗しました。プログラムを終了します。\n" );
		ShowQuitMessageDialog( );
	}

	return 0;
}

void ShowQuitMessageDialog( void ) {
	MessageBoxW( GetConsoleWindow( ), L"OKを押すとプログラムを終了します。", L"二重起動検出方法サンプル", MB_OK );
}

bool  GetMyselfExecuteFileProcessCount( bool exclude_myself_process, uint32_t* pCount, uint32_t* pSuspicionCount ) {

	if ( !pCount || !pSuspicionCount ) {
		return false;
	}

	*pCount = 0;
	*pSuspicionCount = 0;

	// 自分自身の実行ファイルパスを取得
	wchar_t szMyselfExecutePath[MAX_PATH] = { 0 };
	DWORD dwSize = MAX_PATH;

	if ( QueryFullProcessImageNameW( GetCurrentProcess( ), 0, szMyselfExecutePath, &dwSize ) == FALSE ) {
		return false;
	}

	wchar_t* pMyselfExecuteFileName = PathFindFileNameW( szMyselfExecutePath );

	HANDLE hSnapShot = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, NULL );
	if ( hSnapShot == INVALID_HANDLE_VALUE ) {
		return false;
	}

	CHandleAutoCloser handleAutoCloser( hSnapShot );

	PROCESSENTRY32W processEntry = { 0 };
	processEntry.dwSize = static_cast<DWORD>( sizeof( PROCESSENTRY32W ) );

	if ( Process32FirstW( hSnapShot, &processEntry ) == FALSE ) {
		return false;
	}

	wchar_t szCheckTargetExecutePath[MAX_PATH];

	do {


		if ( processEntry.th32ProcessID == GetCurrentProcessId( ) ) {
			if ( !exclude_myself_process ) {
				( *pCount )++;
			}
			continue;
		}


		if ( lstrcmpiW( processEntry.szExeFile, pMyselfExecuteFileName ) == 0 ) {

			HANDLE hProcess = OpenProcess( PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processEntry.th32ProcessID );
			if ( hProcess == NULL ) {
				( *pSuspicionCount )++;
				continue;
			}

			CHandleAutoCloser handleAutoCloser_Another_Process( hProcess );

			dwSize = MAX_PATH;
			if ( QueryFullProcessImageNameW( hProcess, 0, szCheckTargetExecutePath, &dwSize ) == FALSE ) {
				( *pSuspicionCount )++;
				continue;
			}

			if ( lstrcmpiW( szMyselfExecutePath, szCheckTargetExecutePath ) == 0 ) {
				( *pCount )++;
			}
		}

	} while ( Process32NextW( hSnapShot, &processEntry ) != FALSE );

	return true;
}