========================================================================
MFC (MICROSOFT FOUNDATION CLASS) ライブラリ : NyWMCapture プロジェクトの概要
========================================================================


この NyWMCapture DLL は、AppWizard によって作成されました。この DLL は 
MFC の基本的な使い方を示すだけでなく、DLL を作成するための開始点でもあります。

このファイルには、NyWMCapture DLL を構成する各ファイルの内容の概略が
記述されています。

NyWMCapture.vcproj
    これは、アプリケーション ウィザードで生成される VC++ プロジェクトのメインの
    プロジェクト ファイルです。 
    ファイルを生成した Visual C++ のバージョン情報と、アプリケーション ウィザードで
    選択したプラットフォーム、構成、およびプロジェクトの機能に関する情報が記述されて
    います。

NyWMCapture.h
    これは DLL のメインのヘッダー ファイルです。CNyWMCaptureApp クラスを宣言します。

NyWMCapture.cpp
    これは、メインの DLL ソース ファイルです。CNyWMCaptureApp クラスが含まれています。
    インプロセス サーバーに必要な OLE エントリ ポイントも含まれています。

NyWMCapture.idl
    このファイルには、DLL のタイプ ライブラリに対するオブジェクト記述言語のソース 
    コードが含まれています。


NyWMCaptureppc.rc
    これは、Pocket PC プラットフォームや、同じユーザー インターフェイス モデルを
    サポートするプラットフォーム用のコンパイル時にプロジェクトが使用する、すべての 
    Microsoft Windows リソースの一覧です。RES サブディレクトリに格納されているアイコン、
    ビットマップ、およびカーソルが含まれます。.rc ファイルが永続化される場合、データ 
    セクション内の定義は、定義の表示名ではなく 16 進表現の数値として永続化されます。



res\NyWMCapture.rc2
    このファイルには、Microsoft Visual C++ で編集できないリソースが含まれています。
    リソース エディタで編集できないリソースはすべて、このファイルに入れる必要があります。

NyWMCapture.def
    このファイルには、Microsoft Windows で実行するために指定する必要のある DLL の情報が
    含まれています。DLL の名前や説明などのパラメータを定義します。また、DLL から関数を
    エクスポートします。

/////////////////////////////////////////////////////////////////////////////
その他の標準ファイル :

StdAfx.h, StdAfx.cpp
    これらのファイルは、プリコンパイル済みヘッダー (PCH) ファイル 
    NyWMCapture.pch とプリコンパイル済み型ファイル StdAfx.obj をビルドする
    ために使用します。


resourceppc.h
    これは、新しいリソース ID を定義する標準のヘッダー ファイルです。
    このファイルの読み込みおよび更新は、Microsoft Visual C++ で行います。



/////////////////////////////////////////////////////////////////////////////
その他のメモ :

    AppWizard では "TODO:" を使用して、ユーザーが追加またはカスタマイズする必要のある
    ソース コードを示します。

/////////////////////////////////////////////////////////////////////////////




