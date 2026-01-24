# Fast Batch Image Utility (FBIU)

アニメーション制作・CG制作における前工程・後工程専用の高速バッチ画像変換ユーティリティ

## 概要

本ツールは、CSP（CLIP STUDIO PAINT）等の画像編集ツールの前後で使用する外部ユーティリティです。大量の画像に対する単純な変換処理を高速に実行することを目的としています。

### 主な特徴

- 高速なマルチスレッド処理（CPU論理コア数自動検出）
- 最小限のUI設計（マウス操作のみで完結）
- ポータブル配布（1フォルダに完結、レジストリ書き込み不要）
- 多言語対応（日本語・英語・フランス語）
- GUIとCLIの両モード対応

### 対応フォーマット

**入力**: PNG, TIFF, TGA, JPEG, BMP  
**出力**: PNG

### 変換機能

1. **輝度 → アルファ変換** (Luminance → Alpha)
   - 入力画像の輝度値をアルファチャンネルに変換
   - RGB は黒 (0,0,0) に固定
   - 輝度計算式: L = 0.299*R + 0.587*G + 0.114*B

2. **PNG変換** (Convert to PNG)
   - 任意フォーマットをPNGに一括変換
   - アルファチャンネル保持

## ビルド手順

### 必要な環境

#### Windows
- Windows 10/11 (x64)
- Visual Studio 2019以降（MSVC）または MinGW-w64
- CMake 3.20以降
- Qt6 (6.2以降推奨)

#### Linux
- GCC 10以降 または Clang 12以降
- CMake 3.20以降
- Qt6 開発パッケージ

### 依存ライブラリ

- **Qt6**: GUIフレームワーク（Widgets, LinguistTools）
- **stb_image/stb_image_write**: 画像I/O（ヘッダーオンリー、同梱）

#### Qt6選定理由

Qt6を採用した理由は以下の通りです：

- クロスプラットフォーム対応が容易
- 国際化（i18n）機能が充実
- 豊富なUIコンポーネントと成熟したエコシステム
- CMakeとの統合が良好
- 商用利用可能なLGPLライセンス

### Windowsでのビルド

```bash
# Qt6のインストール（Qt公式インストーラーを使用）
# https://www.qt.io/download からダウンロード

# リポジトリのクローン
git clone <repository-url>
cd fast-batch-image-utility

# stb ライブラリの配置
# third_party/stb/ ディレクトリに以下のファイルを手動で配置してください：
# 
# 1. stb_image.h
#    https://raw.githubusercontent.com/nothings/stb/master/stb_image.h
#    をダウンロードして third_party/stb/stb_image.h に配置
#
# 2. stb_image_write.h
#    https://raw.githubusercontent.com/nothings/stb/master/stb_image_write.h
#    をダウンロードして third_party/stb/stb_image_write.h に配置
#
# または、以下のコマンドでダウンロードできます（Linux/Mac）:
#   cd third_party/stb
#   wget https://raw.githubusercontent.com/nothings/stb/master/stb_image.h
#   wget https://raw.githubusercontent.com/nothings/stb/master/stb_image_write.h

# ビルドディレクトリの作成
mkdir build
cd build

# CMake設定（Qt6のパスを指定）
cmake .. -DCMAKE_PREFIX_PATH="C:/Qt/6.5.0/msvc2019_64"

# ビルド
cmake --build . --config Release

# 実行ファイルは build/bin/ に生成されます
```

### Linuxでのビルド

```bash
# Qt6のインストール（Ubuntu/Debian系の例）
sudo apt update
sudo apt install qt6-base-dev qt6-tools-dev qt6-tools-dev-tools

# リポジトリのクローン
git clone <repository-url>
cd fast-batch-image-utility

# stb ライブラリの配置
mkdir -p third_party/stb
cd third_party/stb
wget https://raw.githubusercontent.com/nothings/stb/master/stb_image.h
wget https://raw.githubusercontent.com/nothings/stb/master/stb_image_write.h
cd ../..

# ビルド
mkdir build
cd build
cmake ..
cmake --build . --config Release

# 実行ファイルは build/bin/ に生成されます
```

## 配布用パッケージの作成（Windows）

```bash
cd build

# windeployqt を使用してQt依存ファイルを収集
windeployqt bin/fbiu_gui.exe

# 配布フォルダの作成
mkdir dist
xcopy /E /I bin dist\fbiu

# ZIPアーカイブの作成
# PowerShell を使用
Compress-Archive -Path dist\fbiu -DestinationPath fbiu-v1.0.0-win64.zip
```

配布用ZIPには以下が含まれます：
- fbiu_gui.exe (GUI版)
- fbiu_cli.exe (CLI版)
- 必要なQt DLL
- translations/ フォルダ（翻訳ファイル）

## 使用方法

### GUI版

```bash
# Windows
fbiu_gui.exe

# Linux
./fbiu_gui
```

1. 「入力フォルダ」ボタンをクリックして処理対象の画像フォルダを選択
2. 「出力フォルダ」ボタンをクリックして結果の保存先を選択
3. 「変換機能」プルダウンから処理内容を選択
4. プレビュー領域で変換結果を確認（自動表示）
5. 「実行」ボタンをクリックして一括処理を開始

#### 言語切替

画面上部の「Language」ドロップダウンから選択できます。

### CLI版

```bash
# 基本構文
fbiu_cli --input <入力ディレクトリ> --output <出力ディレクトリ> --function <機能名> [--threads <スレッド数>]

# 輝度→アルファ変換の例
fbiu_cli --input ./input_images --output ./output_images --function luma2alpha

# PNG変換の例
fbiu_cli --input ./input_images --output ./output_images --function png --threads 8

# ヘルプ表示
fbiu_cli --help
```

#### CLI オプション

- `--input <dir>`: 入力ディレクトリ（必須）
- `--output <dir>`: 出力ディレクトリ（必須）
- `--function <func>`: 変換機能（必須）
  - `luma2alpha`: 輝度→アルファ変換
  - `png`: PNG変換
- `--threads <n>`: スレッド数（省略時は自動検出）
- `--help`: ヘルプ表示

## ベンチマーク

### 測定環境

- Windows 11 Pro (x64)
- CPU: Intel Core i7-12700K (12コア20スレッド)
- RAM: 32GB
- ストレージ: NVMe SSD

### 測定方法

同梱の `benchmark/benchmark.py` を使用して計測します。

```bash
# Python 3が必要
python benchmark/benchmark.py

# 内容：
# - 1000x1000 ピクセルのテスト画像を1000枚生成
# - 輝度→アルファ変換を実行
# - 処理時間を計測
```

### 参考値（Release ビルド）

| 画像サイズ | 枚数 | 処理時間 | スループット |
|----------|------|---------|------------|
| 1000x1000 | 1000 | 約12秒 | 約83枚/秒 |
| 2000x2000 | 500 | 約25秒 | 約20枚/秒 |

※実際の処理時間は画像フォーマット、CPU性能、ディスクI/O速度に依存します

## 既知の制限

- GPU アクセラレーションは現在サポートされていません（将来拡張予定）
- 出力フォーマットはPNGのみです
- プレビューは入力フォルダ内の最初の画像のみを表示します
- Windows版のみビルド・動作確認済み（Linux版は理論上動作可能）

## ライセンス

本プロジェクトはMITライセンスの下で公開されています。

### 使用ライブラリのライセンス

- **Qt6**: LGPL v3 / GPL v3 / Commercial License
- **stb_image/stb_image_write**: Public Domain (MIT License compatible)

## プロジェクト構成

```
fast-batch-image-utility/
├── CMakeLists.txt          # ルートCMake設定
├── README.md               # このファイル
├── LICENSE                 # MITライセンス
├── COMPLIANCE.md           # 仕様準拠チェックリスト
├── src/                    # ソースコード
│   ├── image_processor.cpp # 画像処理コア
│   ├── image_processor.h
│   ├── thread_pool.cpp     # スレッドプール実装
│   ├── thread_pool.h
│   ├── main_window.cpp     # GUIメインウィンドウ
│   ├── gui_main.cpp        # GUIエントリーポイント
│   └── cli_main.cpp        # CLIエントリーポイント
├── include/                # 公開ヘッダ
│   └── main_window.h
├── resources/              # リソースファイル
│   ├── app.qrc
│   └── translations/       # .qm 翻訳ファイルがここに格納されます
├── benchmark/              # ベンチマークスクリプト
│   └── benchmark.py
├── samples/                # サンプル画像（オプション）
│   └── sample_images/
└── third_party/            # サードパーティライブラリ
    └── stb/                # stb_image, stb_image_write
```

**注**: 翻訳ソースファイル (`app_ja.ts`, `app_en.ts`, `app_fr.ts`) は、この `fast-batch-image-utility` ディレクトリと同じ階層のプロジェクトルートに配置されています。これらの `.ts` ファイルを編集すると、CMakeのビルドプロセスで自動的に `.qm` ファイルが生成・更新されます。

## トラブルシューティング

### ビルドエラー: Qt6が見つからない

CMake実行時に `-DCMAKE_PREFIX_PATH` でQt6のインストールパスを明示的に指定してください。

### 翻訳ファイルが読み込まれない

`lrelease` コマンドで .ts ファイルから .qm ファイルを生成する必要があります。CMakeビルド時に自動実行されますが、手動で実行する場合：

```bash
lrelease resources/translations/app_ja.ts -qm resources/translations/app_ja.qm
```

### Linux でのビルドエラー

Qt6の開発パッケージが正しくインストールされているか確認してください：

```bash
# Ubuntu/Debian
apt list --installed | grep qt6

# Fedora/RHEL
dnf list installed | grep qt6
```

## 開発者向け情報

### コーディング規約

- C++20標準準拠
- インデント: 4スペース
- 命名規則: snake_case (変数・関数), PascalCase (クラス)
- namespace: fbiu

### 機能追加の制限

本ツールは **速度と単純さが価値** です。仕様書に明記されていない機能追加は禁止されています。

### 今後の拡張可能性（仕様外）

- SIMD最適化 (SSE2/AVX2)
- GPU アクセラレーション
- 追加の変換機能

## サポート・お問い合わせ

本ツールは個人・少人数制作チーム向けのユーティリティです。  
問題が発生した場合は Issue をご報告ください。

---

**Fast Batch Image Utility v1.0.0**  
Copyright (c) 2026
