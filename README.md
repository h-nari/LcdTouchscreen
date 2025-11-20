# LcdTouchscreen

library for touchscreen using XPT2046, calibration included
XPT2046使用のタッチスクリーンライブラリ、 calibration機能付き

## 詳細

XPT2046_touchscreenライブラリに以下の機能を追加しました。

- calibration機能
- calibrationデータのEEPROMへのload/save機能

## 必要なライブラリ

- XPT2046_touchscreen
- [Params](https://github.com/h-nari/Params)
- Adafruit_GFX

## calibrationについて

- calibrationは、4回、画面に表示される十字の中心をタッチすることによって行う。
- Adafruit_GFX::setRotation(n)で設定される表示の向きは、自動で判定される。
- 表示された十字の位置とタッチスクリーンの出力値の相関が十分でなかった場合、calibrationは失敗する。

## サンプルスケッチ

- calibrationSaveLoad.ino
 - タッチスクリーンのcalibration を行いEEPROMに記憶
 - 起動時、EEPROMにcalibrationデータが存在すれば、calibrationをスキップする
 - 起動時、タッチスクリーンがタッチされていれば、EEPROMのデータに関係なくcalibrationを行う
