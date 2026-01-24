# ビルド手順

## 前提条件

- CMake 3.20以降
- C++20対応コンパイラ（MSVC 2019以降、またはGCC 10以降）
- Qt6（GUIビルド時のみ）

## Windowsでのビルド手順

### 1. ビルドディレクトリの作成

```powershell
cd fast-batch-image-utility
mkdir build
cd build
```

### 2. CMake設定の生成

Qt6がシステムにインストールされている場合：

```powershell
cmake ..
```

Qt6のパスを明示的に指定する場合：

```powershell
cmake .. -DCMAKE_PREFIX_PATH="C:/Qt/6.5.0/msvc2019_64"
```

### 3. ビルド実行

```powershell
cmake --build . --config Release
```

または、Visual Studioを使用する場合：

```powershell
cmake --build . --config Release --target fbiu_gui
cmake --build . --config Release --target fbiu_cli
```

### 4. 実行ファイルの確認

ビルドが成功すると、以下のファイルが生成されます：

- `build/bin/Release/fbiu_gui.exe` (GUI版)
- `build/bin/Release/fbiu_cli.exe` (CLI版)

## CLIのみビルド（Qt6不要）

```powershell
cd fast-batch-image-utility
mkdir build
cd build
cmake .. -DBUILD_GUI=OFF
cmake --build . --config Release
```

## トラブルシューティング

### Qt6が見つからない場合

1. Qt6がインストールされているか確認：
   ```powershell
   Get-ChildItem "C:\Qt" -Directory | Select-Object Name
   ```

2. CMake実行時にパスを指定：
   ```powershell
   cmake .. -DCMAKE_PREFIX_PATH="C:/Qt/6.5.0/msvc2019_64"
   ```

### 日本語パスで問題が発生する場合

プロジェクトを英語名のディレクトリに移動するか、短いパス名（8.3形式）を使用してください。

### ビルドエラーの確認

詳細なエラーメッセージを確認するには：

```powershell
cmake --build . --config Release --verbose
```
