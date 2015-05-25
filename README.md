# WindowsMobile用キャプチャCOM DLL.
Copyright (C)2008 nyatla

version 0.1.2

http://nyatla.jp/  
airmail(at)ebony.plala.or.jp  


## NyWMCapture

NyARToolkitは、WindowsMobile用のキャプチャ支援COMオブジェクトです。
DirectShowからコールバック関数を経由してキャプチャ画像を取り込むAPIを提供します。


## 開発環境
 Visual Stadio 2008 Professional 以上(2005でもプロジェクトを作り直せばコンパイルできるかも？)


## ディレクトリ構成

#### ./build
プロジェクトファイルがあります。
#### ./dll
コンパイル済のdllがあります。コンパイルしたdllもこのディレクトリに出力されます。
#### ./doc
設定値などのメモがあります。
#### ./extlib
外部ライブラリBaseClassesがあります。
NyARToolkitより先にコンパイルしておいてください。
BaseClassesのコンパイルは、"BaseClassesのコンパイル方法"を参考にしてください。
#### ./inc
DLLのC++用のヘッダファイルです。
#### ./inc.cs
C#用のヘッダプロジェクトです。
#### ./obj
オブジェクトファイルの出力フォルダです。
#### ./sample
キャプチャのサンプルプログラムがあります。
DLLTest C++のサンプルプロジェクトです。
DllTestCS C#のサンプルプロジェクトです。
#### ./src
ソースファイルがあります。



## BaseClassesのコンパイル方法
付属のBaseClassプロジェクトはソースファイルを削除してあります。別途ソースファイルを入手する必要があります。

BaseClassesのソースファイルは、WindowsCE Platform Builderに含まれています。
評価版をダウンロードしてインストールしておいて下さい。

インストールディレクトリから以下のようにファイルをコピーします。

.\WINCE500\PUBLIC\DIRECTX\SDK\SAMPLES\DSHOW\BASECLASSES\から、.cppファイルを
./extlib/BaseClassesCE500/srcへコピーします。

.\WINCE500\PUBLIC\DIRECTX\SDK\から、.h,.idlファイルを
./extlib/BaseClassesCE500/incへコピーします。

コピーしただけではコンパイルエラーが出ると思うので、適切に修正をしてください。


## ライセンス
NyWMCaptureのライセンスは、MITライセンスです。

