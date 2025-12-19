/*
    自作ライブラリ『HSSoundBasisLib』によって提供される機能の実行レベルでの確認を目的とした
    実験的な汎用実行プログラムです。
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
#include "../../Libraries/HSSoundBasisLib/HSSoundBasisLib.hpp"

// エントリーポイント
int main( void ) {
    setlocale( LC_ALL, "Japanese" );
    SetConsoleOutputCP( 65001 ); // UTF-8コードページに変更

    wprintf( L"HSSoundBasisLib 汎用実行プログラム\n" );

    return 0;
}
