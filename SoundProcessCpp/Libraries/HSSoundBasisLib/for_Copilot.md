# HSSoundBasisLib — Copilot 用サマリ

## 一行要約

HSSoundBasisLib: 基本的な音声基礎処理を提供するDLL。

## HSSoundBasisLibが提供する機能 (未実装なものを含む)

* メモリ管理
* WAV ファイルの読み書き

## ソースファイル説明

### HSSoundBasisLib.hpp


### HSSoundBasisLib.cpp


### impl_IHSSBMemoryBuffer.hpp

### impl_IHSSBMemoryBuffer.cpp


### impl_IHSSBReadOnlyMemoryBuffer.hpp

### impl_IHSSBReadOnlyMemoryBuffer.cpp


### impl_IHSSBWritableMemoryBuffer.hpp

### impl_IHSSBWritableMemoryBuffer.cpp

## Copilot Chatで使用する用語定義

### IHSSBReadOnlyMemoryBuffer実装群

以下の総称である

* 以下で実装されているHSSBCreateReadOnlyMemoryBuffer関数
	- HSSoundBasisLib.hpp
	- HSSoundBasisLib.cpp 
* 以下で実装されているHSSBCreateReadOnlyMemoryBufferType関数
	-　HSSoundBasisLib.hpp
* 以下で実装されているimpl_IHSSBReadOnlyMemoryBufferクラス
	- impl_IHSSBReadOnlyMemoryBuffer.hpp
	- impl_IHSSBReadOnlyMemoryBuffer.cpp
