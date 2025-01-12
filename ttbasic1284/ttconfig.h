//
// Arduino Uno互換機+「アクティブマトリクス蛍光表示管（CL-VFD）MW25616L 実験用表示モジュール」対応
// 豊四季Tiny BASIC for Arduino uno 構築コンフィグレーション
// 修正 2019/06/11 GETFONTコマンド利用オプション設定の追加（美咲フォント対応）
// 修正 2019/07/13 NeoPixel、タイマーイベント利用オプション設定の追加
// 修正 2019/08/04 外部割込みイベント利用オプション設定の追加
// 修正 2019/09/07 機能利用オプション設定のデフォルト設定の見直し
// 修正 2019/10/08 MEGA2560用の機能利用オプション設定を追加
//

#ifndef __ttconfig_h__
#define __ttconfig_h__

#define MYDEBUG 0

/*
 * デフォルトの設定は Arduino UNO 用の設定になっています。
 * 「アクティブマトリクス蛍光表示管（CL-VFD）MW25616L 実験用表示モジュール」
 * で利用する場合は下記の変更を行って下さい。
 *   USE_CMD_VFD  0 ==> 1
 * また、ローカル変数領域不足により動作が不安定になる場合があります。
 * その場合は、プログラム領域サイズを512～1024の間で調整して下さい。
 */

// ** シリアルポート通信速度 *************************************************
#if defined(ARDUINO_AVR_ATmega1284)
  #define SERIALBAUD 9600
#else
  #define SERIALBAUD 115200
#endif

// ** 利用ピン設定 **********************************************************
#if defined(ARDUINO_AVR_MEGA2560)
  // Arduino MEGA2560
  #define   TonePin 49  // Tone用出力ピン（圧電スピーカー接続）
  #define   AutoPin 53  // 自動起動チェックピン
#elif defined(ARDUINO_AVR_ATmega1284)
  // Arduino MEGA1284
  #define   TonePin 1  // Tone用出力ピン（圧電スピーカー接続）
  #define   AutoPin 2  // 自動起動チェックピン
#else
  // Arduino Uno/nano/pro mini
  #define   TonePin 8  // Tone用出力ピン（圧電スピーカー接続）
  #define   AutoPin 7  // 自動起動チェックピン
#endif

// ** プログラム領域サイズ ***************************************************
#if defined(ARDUINO_AVR_MEGA2560)
  // Arduino MEGA2560
  #define   PRGAREASIZE 2048 // プログラム領域サイズ(Arduino Mega 512 ～ 4096 デフォルト:2048)
  #define   ARRYSIZE    100  // 配列領域
#elif defined(ARDUINO_AVR_ATmega1284)
  // Arduino MEGA1284
  #define   PRGAREASIZE 2048 // プログラム領域サイズ(Arduino Mega 512 ～ 4096 デフォルト:2048)
  #define   ARRYSIZE    300  // 配列領域
#else
  // Arduino Uno/nano/pro mini
  #define   PRGAREASIZE 1024 // プログラム領域サイズ(Arduino Uno  512 ～ 1024 デフォルト:1024)
  #define   ARRYSIZE    32   // 配列領域
#endif

#define USE_ALL_KEYWORD  1   // 未使用キーワードも有効にする(1:有効 2:無効 デフォルト:1)

#if defined(ARDUINO_AVR_MEGA2560) || defined(ARDUINO_AVR_ATmega1284)
// ** 機能利用オプション設定 for Arduino MEGA2560 MEGA1284 ***************************
#define USE_CMD_PLAY   1  // PLAYコマンドの利用(0:利用しない 1:利用する デフォルト:1)
#define USE_CMD_I2C    1  // I2Cコマンドの利用(0:利用しない 1:利用する デフォルト:1)
#define USE_PULSEIN    1  // PULSEIN関数の利用(0:利用しない 1:利用する デフォルト:1)
#define USE_SHIFTIN    1  // SHIFTIN関数の利用(0:利用しない 1:利用する デフォルト:1)
#define USE_SHIFTOUT   1  // SHIFTOUTコマンドの利用(0:利用しない 1:利用する デフォルト:1)
#define USE_CMD_VFD    0  // VFDモジュールコマンドの利用(0:利用しない 1:利用する デフォルト:0)
#define USE_RTC_DS3231 1  // I2C接続RTC DS3231の利用(0:利用しない 1:利用する デフォルト:1)
#define USE_I2CEEPROM  1  // I2C EEPROM対応(0:利用しない 1:利用する デフォルト:1)
#define USE_SYSINFO    1  // SYSINFOコマンド(0:利用しない 1:利用する デフォルト:1)
#define USE_GRADE      1  // GRADE関数(0:利用しない 1:利用する デフォルト:1)
#define USE_DMP        1  // DMP$関数(0:利用しない 1:利用する デフォルト:1)
#define USE_IR         1  // IR関数(0:利用しない 1:利用する デフォルト:1)
#define USE_ANADEF     1  // アナログピン定数A0～A7orA15(0:利用しない 1:利用する デフォルト:1)
#define USE_SO1602AWWB 1  // 有機ELキャラクタディスプレイ SO1602AWWB(0:利用しない 1:利用する デフォルト:1)
#define USE_MISAKIFONT 1  // 美咲フォント500文字の利用(0:利用しない 1:利用する 2:非漢字のみ利用 デフォルト:1)
#define USE_NEOPIXEL   1  // NeoPixelの利用(0:利用しない 1:利用する デフォルト:1)
#define USE_EVENT      1  // タイマー・外部割込みイベントの利用(0:利用しない 1:利用する デフォルト:1)
#define USE_SLEEP      1  // SLEEPコマンドの利用(0:利用しない 1:利用する デフォルト:1) ※USE_EVENTを利用必須
#else
// ** 機能利用オプション設定 for Arduino Uno *********************************
#define USE_CMD_PLAY   0  // PLAYコマンドの利用(0:利用しない 1:利用する デフォルト:0)
#define USE_CMD_I2C    1  // I2Cコマンドの利用(0:利用しない 1:利用する デフォルト:1)
#define USE_PULSEIN    1  // PULSEIN関数の利用(0:利用しない 1:利用する デフォルト:1)
#define USE_SHIFTIN    1  // SHIFTIN関数の利用(0:利用しない 1:利用する デフォルト:1)
#define USE_SHIFTOUT   1  // SHIFTOUTコマンドの利用(0:利用しない 1:利用する デフォルト:1)
#define USE_CMD_VFD    0  // VFDモジュールコマンドの利用(0:利用しない 1:利用する デフォルト:0)
#define USE_RTC_DS3231 0  // I2C接続RTC DS3231の利用(0:利用しない 1:利用する デフォルト:0)
#define USE_I2CEEPROM  1  // I2C EEPROM対応(0:利用しない 1:利用する デフォルト:1)
#define USE_SYSINFO    1  // SYSINFOコマンド(0:利用しない 1:利用する デフォルト:1)
#define USE_GRADE      1  // GRADE関数(0:利用しない 1:利用する デフォルト:1)
#define USE_DMP        1  // DMP$関数(0:利用しない 1:利用する デフォルト:1)
#define USE_IR         1  // IR関数(0:利用しない 1:利用する デフォルト:1)
#define USE_ANADEF     1  // アナログピン定数A0～A7orA15(0:利用しない 1:利用する デフォルト:1)
#define USE_SO1602AWWB 0  // 有機ELキャラクタディスプレイ SO1602AWWB(0:利用しない 1:利用する デフォルト:0)
#define USE_MISAKIFONT 0  // 美咲フォント500文字の利用(0:利用しない 1:利用する 2:非漢字のみ利用 デフォルト:0)
#define USE_NEOPIXEL   0  // NeoPixelの利用(0:利用しない 1:利用する デフォルト:0)
#define USE_EVENT      1  // タイマー・外部割込みイベントの利用(0:利用しない 1:利用する デフォルト:1)
#define USE_SLEEP      1  // SLEEPコマンドの利用(0:利用しない 1:利用する デフォルト:1) ※USE_EVENTを利用必須
#endif

#endif
