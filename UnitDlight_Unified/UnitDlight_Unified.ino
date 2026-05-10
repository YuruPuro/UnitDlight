/**
 * M5StickCPlis / M5StickCPlus2 / M5StickS3 で
 * M5StickC 環境光センサ Hat(商品コード: M5STACK-U134) を使用して照度を測定し、適正な照度範囲をアイコンで表示。
 * @YouPuro
 */

#include <M5Unified.h>
#include <Wire.h>
#include "image.h"

// I2Cピンの定義
#if defined(ARDUINO_M5STACK_STICKC_PLUS)
  #define SDA_PIN 0
  #define SCL_PIN 26
#elif defined(ARDUINO_M5STACK_STICKC_PLUS2)
  #define SDA_PIN 0
  #define SCL_PIN 26
#elif defined(ARDUINO_M5STACK_STICKS3)
  #define SDA_PIN 8
  #define SCL_PIN 0
#else
  #error "Unsupported board"
#endif

// 照度値の保持
uint16_t dlightHold = 0 ;

/**
 * 照度センサから値を取得
 * 0x23 はセンサのI2Cアドレス、0x10 は連続ハイレゾリューションモードのコマンド
 * 測定時間は約180msで、2バイトのデータを読み取る。取得した値は1.2で割ってルクスに変換する。
 * エラーの場合は-1を返す。
 */
uint16_t getDlight() {
  Wire.beginTransmission(0x23);
  Wire.write(0x10); // Continuously H-Resolution Mode
  Wire.endTransmission();
  delay(180); // Measurement time

  Wire.requestFrom(0x23, 2);
  if (Wire.available() == 2) {
    uint16_t raw = Wire.read() << 8 | Wire.read();
    return raw / 1.2; // Convert to lux
  }
  return -1; // Error
}

/**
 * セットアップ関数では、M5StickCの初期化、I2C通信の開始、照度センサの初期化を行います。
 * 表示の初期化では、背景を黒に設定し、照度アイコンを表示します。さらに、照度の単位「Lx」を表示し、初期の照度値を0で表示します。最後に、適正照度アイコン
 * （パソコン操作、読書、手書きレポート・勉強、はんだ付け）をグレーで表示します。
 */
void setup() {
    auto cfg = M5.config();
    M5.begin(cfg);
    Wire.begin(SDA_PIN, SCL_PIN);

    // 表示初期化
    M5.Display.setRotation(1);
    M5.Display.fillScreen(BLACK);
    M5.Display.setTextSize(1);
    // 照度アイコン表示
    M5.Display.drawBitmap(10, 16, ILLUMINACE, ILLUMINACE_W, ILLUMINACE_H, YELLOW);

    // 表示単位 Lx
    M5.Display.setTextColor(WHITE);
    M5.Display.setFont(&fonts::Font4);
    M5.Display.setCursor(210, 34);
    M5.Display.print("Lx");

    // Lx値表示
    M5.Display.setTextColor(GREEN);
    M5.Display.setFont(&fonts::Font7);
    M5.Display.setCursor(50, 10);
    M5.Display.printf("%05d",0);

    // IDCON
    M5.Display.drawBitmap(  5, 80, icon_PC, icon_PC_W, icon_PC_H, DARKGREY);
    M5.Display.drawBitmap( 70, 70, icon_BOOK, icon_BOOK_W, icon_BOOK_H, DARKGREY);
    M5.Display.drawBitmap(135, 80, icon_NOTE, icon_NOTE_W, icon_NOTE_H, DARKGREY);
    M5.Display.drawBitmap(200, 65, icon_ELEC, icon_ELEC_W, icon_ELEC_H, DARKGREY);
}

/**
 * ループ関数では、照度センサから値を取得し、前回の値と比較して変化があった場合にのみ表示を更新します。照度値が0以上の場合は緑色で表示し、エラーの場合は緑色で「88888」を表示します。
 * 適正照度アイコンの表示も更新され、各アイコンは照度値に応じてグレー、白、黄色で表示されます。最後に、1秒の遅延を入れてループを繰り返します。
 * 適正照度の範囲は以下の通りです：
 * - パソコン操作: 300〜500 lx
 * - 読書: 400〜700 lx
 * - 手書きレポート・勉強: 600〜1000 lx
 * - はんだ付け: 1000〜2000 lx
 */
void loop() {
  // 照度値の取得と表示更新
  uint16_t dlight = getDlight();
  // 前回の値と比較して変化があった場合にのみ表示を更新
  if (dlightHold != dlight) {
    dlightHold = dlight ;
    // Lx値表示
    if (dlight != 0xFFFF) {
      // 前回表示を消すために黒で上書き
        M5.Display.setTextColor(BLACK);
        M5.Display.setCursor(50, 10);
        M5.Display.printf("88888");
        // 新しい値を緑色で表示
        M5.Display.setTextColor(GREEN);
        M5.Display.setCursor(50, 10);
        M5.Display.printf("%05d",dlight);
    } else {
      // エラー表示
        M5.Display.setTextColor(GREEN);
        M5.Display.setCursor(50, 10);
        M5.Display.printf("88888");
    }
    // 適正照度アイコン表示
    uint16_t colorPC   = DARKGREY ;
    uint16_t colorBOOK = DARKGREY ;
    uint16_t colorNOTE = DARKGREY ;
    uint16_t colorELEC = DARKGREY ;
    // パソコン操作 300〜500 lx
    if (300 <= dlight) { colorPC = (dlight <= 500) ? WHITE : YELLOW ;  }
    // 読書 400〜700 lx
    if (400 <= dlight) { colorBOOK = (dlight <= 700) ? WHITE : YELLOW ;  }
    // 手書きレポート・勉強 600〜1000 lx
    if (600 <= dlight) { colorNOTE = (dlight <= 1000) ? WHITE : YELLOW ;  }
    // はんだ付け 1000〜2000 lx
    if (1000 <= dlight) { colorELEC = (dlight <= 2000) ? WHITE : YELLOW ;  }

    // アイコンの再描画
    M5.Display.drawBitmap(  5, 80, icon_PC, icon_PC_W, icon_PC_H, colorPC);
    M5.Display.drawBitmap( 70, 70, icon_BOOK, icon_BOOK_W, icon_BOOK_H, colorBOOK);
    M5.Display.drawBitmap(135, 80, icon_NOTE, icon_NOTE_W, icon_NOTE_H, colorNOTE);
    M5.Display.drawBitmap(200, 65, icon_ELEC, icon_ELEC_W, icon_ELEC_H, colorELEC);
  }
  delay(1000) ;
}
