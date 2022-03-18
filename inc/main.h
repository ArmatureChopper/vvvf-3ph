/*
 * main.h
 *
 *  Created on: 2022/03/08
 *      Author: @ArmatureChopper
 */

#ifndef MAIN_H_
#define MAIN_H_

#include <stdint.h>

/*
	CPUクロック周波数 (cclk) の設定について

	プロジェクト"CMSIS_CORE_LPC17xx"の中にある"system_LPC17xx.c"の287行目に記述されている
	"PLL0CFG_Val"の値を変更すると周波数を変更できる

		0x00050063 を設定すると cclk = 100MHz動作 (デフォルト)
		0x00050077 を設定すると cclk = 120MHz動作 (このプログラム)
*/

// 波形テーブル
extern const uint8_t g_pwm_table_45p[16384];
extern const uint8_t g_pwm_table_27p[16384];
extern const uint8_t g_pwm_table_15p[16384];
extern const uint8_t g_pwm_table_9p[16384];
extern const uint8_t g_pwm_table_5p[16384];
extern const uint8_t g_pwm_table_3p[16384];


// マクロ定義
#define CHATTERING_PERIOD		100				// nRUN入力チャタリング除去時間 [ms]
#define WDT_TIMEOUT_PERIOD		10000			// WDTタイムアウト時間 [us]
#define PWM_UPDATE_FREQ			65536			// PWM出力更新周波数 [Hz]
#define VF_CTRL_FREQ			128				// V/f制御用変数更新周波数 [Hz]
#define BUZZER_ON_PERIOD		83				// ブザー鳴動ON期間 [ms]
#define BUZZER_REPEAT_PERIOD	167				// ブザー鳴動繰り返し間隔 [ms]
#define FAN_CTRL_PWM_PRESCALE	11				// 冷却ファン制御用PWMクロック分周器設定値 [+1分周]
#define FAN_CTRL_PWM_PERIOD		100				// 冷却ファン制御用PWM周期 [x(10^6*分周値*4/cclk)us]
#define FAN_CTRL_PWM_DUTY		80				// 冷却ファン制御用PWMオン時間 [x(10^6*分周値*4/cclk)us]
#define INV_THETA_120			1431655765U		// 出力位相120度の値 [x360/(2^32)deg]
#define INV_THETA_240			2863311531U		// 出力位相240度の値 [x360/(2^32)deg]
#define INV_ALPHA_RISE_TIME		750				// 出力電圧立ち上げ時間 [ms]
#define INV_FREQ_HOLD_TIME		1000			// 出力周波数保持モード時間 [ms]
#define INV_ACCEL_RATE_INIT		5.0				// 出力周波数変化率初期値 [Hz/s]
#define INV_ACCEL_RATE_LAST		1.0				// 出力周波数変化率最終値 [Hz/s]
#define INV_FREQ_INIT			2.0				// 出力周波数初期値 [Hz]
#define INV_FREQ_WEAK_ACCEL		52.0			// 出力周波数弱加速開始値 [Hz]
#define INV_FREQ_REF			60.0			// 出力周波数目標値 [Hz]
#define INV_FREQ_27P			8.5				// 出力周波数27Pモード開始値 [Hz]
#define INV_FREQ_15P			17.0			// 出力周波数15Pモード開始値 [Hz]
#define INV_FREQ_9P				32.5			// 出力周波数9Pモード開始値 [Hz]
#define INV_FREQ_5P				40.0			// 出力周波数5Pモード開始値 [Hz]
#define INV_FREQ_3P				46.0			// 出力周波数3Pモード開始値 [Hz]
#define INV_FREQ_1P				52.0			// 出力周波数1Pモード開始値 [Hz]

// トルクブースト電圧 [x100%]
// 2P 0.75kW 相間R測定値3.9ohm Y換算R1相分1.95ohm 定格電流3.9A 相電圧7.6V 線間電圧13V
#define INV_VOLT_TORQUEBOOST	(6.3 / 100)		// 13V / 210V * 100 = 6.3%

// V/f比 [x100%/Hz]
#define VF_CTRL_RATIO			((1 - INV_VOLT_TORQUEBOOST) / INV_FREQ_1P)

// nRUN入力チャタリング除去カウント値
#define CHATTERING_COUNT		(PWM_UPDATE_FREQ * CHATTERING_PERIOD / 1000)

// ブザー鳴動ON期間カウント値
#define BUZZER_ON_COUNT			(PWM_UPDATE_FREQ * BUZZER_ON_PERIOD / 1000)

// ブザー鳴動繰り返し間隔カウント値
#define BUZZER_REPEAT_COUNT		(PWM_UPDATE_FREQ * BUZZER_REPEAT_PERIOD / 1000)

// V/f制御起動カウント値
#define VF_CTRL_COUNT			(PWM_UPDATE_FREQ / VF_CTRL_FREQ)

// 出力周波数保持モードカウント値
#define INV_FREQ_HOLD_COUNT		(VF_CTRL_FREQ * INV_FREQ_HOLD_TIME / 1000)

// 出力周波数変化量初期値 [Hz]
#define DELTA_INV_FREQ_INIT		(INV_ACCEL_RATE_INIT / VF_CTRL_FREQ)

// 弱加速時間 [s]
#define WEAK_ACCEL_TIME			(2 * (INV_FREQ_REF - INV_FREQ_WEAK_ACCEL)\
										/ (INV_ACCEL_RATE_INIT+ INV_ACCEL_RATE_LAST))

// 出力周波数変化率の変化率 [Hz/s^2]
#define INV_ACCEL_RATE2			((INV_ACCEL_RATE_INIT - INV_ACCEL_RATE_LAST) / WEAK_ACCEL_TIME)

// 出力周波数変化量減少値 [Hz]
#define DELTA_INV_FREQ_DROP		(INV_ACCEL_RATE2 / (VF_CTRL_FREQ * VF_CTRL_FREQ))

// 出力電圧立ち上げ変調率変化量 [x100%]
#define DELTA_INV_ALPHA			(1000.0 / (VF_CTRL_FREQ * INV_ALPHA_RISE_TIME))


// 固定小数点型定義
typedef uint32_t	FIX8_T;			// 8bitシフト固定小数点
typedef uint32_t	FIX16_T;		// 16bitシフト固定小数点
typedef uint32_t	FIX32_T;		// 32bitシフト固定小数点


// 列挙型定義
// 出力周波数パターン生成モード
typedef enum FREQ_STATE {
	FREQ_STATE_HOLD,		// 保持モード
	FREQ_STATE_ACCEL,		// 加速モード
	FREQ_STATE_CONST		// 定速モード
} FREQ_STATE_T;

// パルスモード
typedef enum PMODE {
	PMODE_45P,
	PMODE_27P,
	PMODE_15P,
	PMODE_9P,
	PMODE_5P,
	PMODE_3P,
	PMODE_1P
} PMODE_T;


// プロトタイプ宣言
void init_peripheral(void);
void init_indicator(void);
void init_variable(void);
void update_vf_buff(void);
void generate_inv_freq(void);
void generate_inv_alpha(void);
void set_inv_pmode(void);
void calc_inv_volt(void);
void update_pwm_buff(void);
void sw_inv_pmode(void);
void calc_pwm_alpha(void);
void set_pwm_table(void);
void update_pwm_pin(void);
void reload_wdt(void);
void buff_empty_error(void);


#endif /* MAIN_H_ */
