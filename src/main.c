/*
===============================================================================
 Name        : main.c
 Author      : @ArmatureChopper
 Version     : 1.1
 Copyright   : @ArmatureChopper
 Description : main definition
===============================================================================
*/

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>
#include "pin_config.h"
#include "main.h"


// グローバル変数
uint32_t g_buzzer_ctrl_count = 0;		// ブザー鳴動間隔制御カウンタ
uint32_t g_buzzer_on = 0;				// ブザー鳴動ONフラグ
uint32_t g_fault_occur = 0;				// 保護動作フラグ
uint32_t g_ocr_trip = 0;				// OCRトリップフラグ
uint32_t g_inv_run = 0;					// インバータ運転指令
uint32_t g_inv_run_buff = 0;			// インバータ運転指令バッファ
uint32_t g_vf_ctrl_count;				// V/f制御起動カウンタ
FREQ_STATE_T g_inv_freq_state;			// 出力周波数パターン生成モード
double g_inv_volt_buff;					// 出力電圧バッファ [x100%]
FIX32_T g_inv_volt_buff_fx;				// 出力電圧バッファ [x100/(2^32-1)%] (32bitシフト固定小数点)
double g_inv_freq_buff;					// 出力周波数バッファ [Hz]
FIX16_T g_inv_freq_buff_fx;				// 出力周波数バッファ [x1/(2^16)Hz] (16bitシフト固定小数点)
PMODE_T g_inv_pmode_buff;				// パルスモードバッファ
uint32_t g_vf_buff_empty;				// V/f制御用バッファ空フラグ
double g_inv_alpha;						// 出力電圧立ち上げ変調率 [x100%]
FIX32_T g_inv_volt;						// 出力電圧 [x100/(2^32-1)%] (32bitシフト固定小数点)
FIX16_T g_inv_freq;						// 出力周波数 [x1/(2^16)Hz] (16bitシフト固定小数点)
PMODE_T g_inv_pmode;					// パルスモード
PMODE_T g_inv_pmode_ref;				// パルスモード指令値
uint32_t g_inv_theta_u;					// U相出力位相 [x360/(2^32)deg] (32bitシフト固定小数点)
uint32_t g_inv_theta_v;					// V相出力位相 [x360/(2^32)deg] (32bitシフト固定小数点)
uint32_t g_inv_theta_w;					// W相出力位相 [x360/(2^32)deg] (32bitシフト固定小数点)
uint32_t g_inv_theta_u_neg_now;			// U相出力位相範囲の現在値 (0: 正の半周期, 1: 負の半周期)
uint32_t g_inv_theta_v_neg_now;			// V相出力位相範囲の現在値 (0: 正の半周期, 1: 負の半周期)
uint32_t g_inv_theta_w_neg_now;			// W相出力位相範囲の現在値 (0: 正の半周期, 1: 負の半周期)
uint32_t g_inv_theta_u_neg_prv;			// U相出力位相範囲の前回値 (0: 正の半周期, 1: 負の半周期)
uint32_t g_inv_theta_v_neg_prv;			// V相出力位相範囲の前回値 (0: 正の半周期, 1: 負の半周期)
uint32_t g_inv_theta_w_neg_prv;			// W相出力位相範囲の前回値 (0: 正の半周期, 1: 負の半周期)
const uint8_t *gp_pwm_table;			// 波形テーブル
FIX8_T g_pwm_alpha;						// 変調率 [x100/(256)%] (8bitシフト固定小数点)
uint32_t g_pwm_buff_u;					// U相PWM出力バッファ
uint32_t g_pwm_buff_v;					// V相PWM出力バッファ
uint32_t g_pwm_buff_w;					// W相PWM出力バッファ


int main(void)
{
	// 周辺機能の初期化
	init_peripheral();

	// ユーザインタフェースの初期化
	init_indicator();

	// 変数の初期化
	init_variable();

	// SysTickタイマ初期化 & スタート
	SystemCoreClockUpdate();
	SysTick_Config(SystemCoreClock / PWM_UPDATE_FREQ);

	// WDTスタート
	LPC_WDT->WDMOD |= (1 << 0);

	// メインループ
	while (1) {

		// 入力ピンの読み取り & 状態更新
		if (FAULT_GetValue() != 0) {		// 保護動作信号入力
			g_fault_occur = 1;
		} else {
			g_fault_occur = 0;
		}
		if (nOCR_GetValue() == 0) {			// OCRトリップ信号入力
			g_ocr_trip = 1;
		} else {
			g_ocr_trip = 0;
		}

		// インバータ運転・停止制御
		if ((g_inv_run_buff != 0) && (g_fault_occur == 0)) {
			g_inv_run = 1;
		} else {
			g_inv_run = 0;
		}

		// V/f制御用バッファの更新
		if (g_vf_buff_empty != 0) {
			update_vf_buff();
			g_vf_buff_empty = 0;		// バッファ空フラグクリア
		} else {
			// DO NOTHING
		}

		// 冷却ファン動作制御
		if (g_inv_run != 0) {			// インバータ運転時に冷却ファンも動作させる
			FAN_CTRL_SetPWMEnable();
		} else {
			FAN_CTRL_SetPWMDisable();
		}

		// ブザー鳴動制御
		// 保護動作時とOCRトリップ時に鳴動
		if ((g_fault_occur != 0) || (g_ocr_trip != 0)) {
			// ブザー鳴動ONフラグセット
			g_buzzer_on = 1;
			// ピーピーと断続的に鳴らす
			if (g_buzzer_ctrl_count < BUZZER_ON_COUNT) {
				BUZZER_SetHigh();
			} else if (g_buzzer_ctrl_count < BUZZER_REPEAT_COUNT) {
				BUZZER_SetLow();
			} else {
				g_buzzer_ctrl_count = 0;
			}
		} else {
			// ブザー鳴動ONフラグクリア
			g_buzzer_on = 0;
			// ブザー鳴動停止
			BUZZER_SetLow();
			g_buzzer_ctrl_count = 0;
		}

		// LED表示制御
		if (g_ocr_trip != 0) {
			LED_OCR_SetHigh();
		} else {
			LED_OCR_SetLow();
		}
		if (g_inv_run != 0) {
			LED_RUN_SetHigh();
		} else {
			LED_RUN_SetLow();
		}

		// WDTリロード
		reload_wdt();
    }

    return 0 ;
}


// 周辺機能の初期化
void init_peripheral(void)
{
	// GPIO初期化
	BUZZER_SetLow();
	BUZZER_SetDigitalOutput();
	LED_OCR_SetLow();
	LED_OCR_SetDigitalOutput();
	LED_RUN_SetLow();
	LED_RUN_SetDigitalOutput();
	PWM_U_SetLow();
	PWM_U_SetDigitalOutput();
	PWM_V_SetLow();
	PWM_V_SetDigitalOutput();
	PWM_W_SetLow();
	PWM_W_SetDigitalOutput();
	PWM_SD_SetHigh();				// 強制遮断状態を保つ
	PWM_SD_SetDigitalOutput();
	FAN_CTRL_SetLow();
	FAN_CTRL_SetDigitalOutput();

	// PWM初期化 (冷却ファン制御用)
	LPC_PWM1->TCR |= (1 << 3);					// PWM mode is enabled (counter resets to 1)
	LPC_PWM1->PR = FAN_CTRL_PWM_PRESCALE;		// The TC is incremented every PR+1 cycles of PCLK
	LPC_PWM1->MCR |= (1 << 1);					// the PWMTC will be reset if PWMMR0 matches it
	LPC_PWM1->MR0 = FAN_CTRL_PWM_PERIOD;		// Write a new value to the PWM Match0 register
	LPC_PWM1->MR6 = FAN_CTRL_PWM_DUTY;			// Write a new value to the PWM Match6 register
	LPC_PWM1->LER |= ((1 << 6) | (1 << 0));		// Enables use of new PWM match values
	LPC_PWM1->PCR |= (1 << 14);					// The PWM6 output enabled
	LPC_PWM1->TCR |= (1 << 0);					// The PWM Timer Counter and PWM Prescale
												// Counter are enabled for counting
	// WDT初期化
	LPC_WDT->WDTC = WDT_TIMEOUT_PERIOD;			// タイムアウト時間をセット
	LPC_WDT->WDMOD |= (1 << 1);					// WDTによるリセット有効
	LPC_WDT->WDCLKSEL |= (1 << 31);				// クロック源を4MHz内部RC発振器の設定でロック
	LPC_WDT->WDFEED = 0xAA;						// フィードシーケンス
	LPC_WDT->WDFEED = 0x55;
}


// ユーザインタフェースの初期化
void init_indicator(void)
{
	volatile uint32_t i;

	// ランプチェック
	LED_OCR_SetHigh();
	LED_RUN_SetHigh();

	// 冷却ファン動作チェック
	FAN_CTRL_SetPWMEnable();

	// ブザーチェック
	// 短くピピッと鳴らす
	BUZZER_SetHigh();
	for (i = 0; i < 500000; i++) {
		__NOP();
	}
	BUZZER_SetLow();
	for (i = 0; i < 500000; i++) {
		__NOP();
	}
	BUZZER_SetHigh();
	for (i = 0; i < 500000; i++) {
		__NOP();
	}
	BUZZER_SetLow();			// ブザーチェック終了

	// ブザーチェック以外はしばらく継続する
	for (i = 0; i < 10000000; i++) {
		__NOP();
	}
	LED_OCR_SetLow();			// ランプチェック終了
	LED_RUN_SetLow();
	FAN_CTRL_SetPWMDisable();	// 冷却ファン動作チェック終了
}


// 変数の初期化
void init_variable(void)
{
	// インバータ制御用変数の初期化
	g_vf_ctrl_count = 0;
	g_inv_freq_state = FREQ_STATE_HOLD;
	g_inv_volt_buff = 0.0;
	g_inv_volt_buff_fx = 0;
	g_inv_freq_buff = INV_FREQ_INIT;
	g_inv_freq_buff_fx = (uint32_t)(INV_FREQ_INIT * 65536);
	g_inv_pmode_buff = PMODE_45P;
	g_vf_buff_empty = 1;
	g_inv_alpha = 0.0;
	g_inv_volt = 0;
	g_inv_freq = (uint32_t)(INV_FREQ_INIT * 65536);
	g_inv_pmode = PMODE_45P;
	g_inv_pmode_ref = PMODE_45P;
	g_inv_theta_u = 0;
	g_inv_theta_v = INV_THETA_240;
	g_inv_theta_w = INV_THETA_120;
	g_inv_theta_u_neg_now = 0;
	g_inv_theta_v_neg_now = 1;
	g_inv_theta_w_neg_now = 0;
	g_inv_theta_u_neg_prv = 1;
	g_inv_theta_v_neg_prv = 1;
	g_inv_theta_w_neg_prv = 0;
	gp_pwm_table = g_pwm_table_45p;
	g_pwm_alpha = 0;
	update_pwm_buff();			// PWM出力バッファの初回データを準備
}


// V/f制御用バッファの更新
void update_vf_buff(void)
{
	// 出力電圧・出力周波数・パルスモードを演算しバッファに格納する

	// 出力周波数パターン生成
	generate_inv_freq();

	// 出力電圧立ち上げ変調率パターン生成
	generate_inv_alpha();

	// パルスモード設定
	set_inv_pmode();

	// 出力電圧演算
	calc_inv_volt();

	// 浮動小数点から固定小数点に変換
	// 単精度だと出力電圧は4294967296に丸められて, uint32_tにキャストすると桁あふれして0になってしまう
	g_inv_volt_buff_fx = (uint32_t)(g_inv_volt_buff * 4294967295);
	g_inv_freq_buff_fx = (uint32_t)(g_inv_freq_buff * 65536);
}


// 出力周波数パターン生成
void generate_inv_freq(void)
{
	static uint32_t hold_count = 0;			// 保持モード用カウンタ
	static double delta_inv_freq = 0.0;		// 出力周波数変化量 [Hz]

	if (g_inv_run != 0) {
		// インバータ運転時
		switch (g_inv_freq_state) {
		case FREQ_STATE_HOLD:
			// 保持モード
			// 出力周波数が初期値の状態を設定された時間保持し
			// その後加速モードに移行する
			g_inv_freq_buff = INV_FREQ_INIT;
			hold_count++;
			if (hold_count >= INV_FREQ_HOLD_COUNT) {
				hold_count = 0;
				g_inv_freq_state = FREQ_STATE_ACCEL;
			} else {
				// DO NOTHING
			}
			break;

		case FREQ_STATE_ACCEL:
			// 加速モード
			// 出力周波数を出力周波数変化量に応じた割合で増加させる
			// 出力周波数変化量は弱加速開始まで初期値を保持し
			// 弱加速開始後は出力周波数が目標値に到達するまで一定割合で減少させる
			// 出力周波数が目標値に到達後は定速モードに移行する

			// 出力周波数変化量計算
			if (g_inv_freq_buff <= INV_FREQ_WEAK_ACCEL) {
				delta_inv_freq = DELTA_INV_FREQ_INIT;			// 定加速域では初期値で一定
			} else {
				delta_inv_freq -= DELTA_INV_FREQ_DROP;			// 弱加速域では初期値から徐々に減少
			}

			// 出力周波数計算
			g_inv_freq_buff += delta_inv_freq;
			if (g_inv_freq_buff >= INV_FREQ_REF) {
				// 目標値を超過したら目標値をセットする
				g_inv_freq_buff = INV_FREQ_REF;
				// 加速完了
				// 定速モードに移行する
				g_inv_freq_state = FREQ_STATE_CONST;
			} else {
				// DO NOTHING
			}
			break;

		default:
			// 定速モード (FREQ_STATE_CONST)
			// 出力周波数は目標値を維持する
			g_inv_freq_buff = INV_FREQ_REF;
			delta_inv_freq = 0.0;
			break;
		}
	} else {
		// インバータ停止時
		// 出力周波数と制御用変数には初期値をセットする
		g_inv_freq_state = FREQ_STATE_HOLD;
		g_inv_freq_buff = INV_FREQ_INIT;
		delta_inv_freq = 0.0;
		hold_count = 0;
	}
}


// 出力電圧立ち上げ変調率パターン生成
void generate_inv_alpha(void)
{
	if (g_inv_run != 0) {
		// インバータ運転時
		// 変調率が100%になるまで一定割合で増加させる
		// その後はそのまま100%を維持する
		g_inv_alpha += DELTA_INV_ALPHA;
		if (g_inv_alpha >= 1.0) {
			g_inv_alpha = 1.0;
		} else {
			// DO NOTHING
		}
	} else {
		// インバータ停止時
		// 変調率はゼロに初期化する
		g_inv_alpha = 0.0;
	}
}


// パルスモード設定
void set_inv_pmode(void)
{
	// パルスモード選択
	if (g_inv_freq_buff >= INV_FREQ_1P) {
		g_inv_pmode_buff = PMODE_1P;
	} else if (g_inv_freq_buff >= INV_FREQ_3P) {
		g_inv_pmode_buff = PMODE_3P;
	} else if (g_inv_freq_buff >= INV_FREQ_5P) {
		g_inv_pmode_buff = PMODE_5P;
	} else if (g_inv_freq_buff >= INV_FREQ_9P) {
		g_inv_pmode_buff = PMODE_9P;
	} else if (g_inv_freq_buff >= INV_FREQ_15P) {
		g_inv_pmode_buff = PMODE_15P;
	} else if (g_inv_freq_buff >= INV_FREQ_27P) {
		g_inv_pmode_buff = PMODE_27P;
	} else {
		g_inv_pmode_buff = PMODE_45P;
	}
}


// 出力電圧演算
void calc_inv_volt(void)
{
	double inv_volt_ref;		// 出力電圧目標値 [x100%]

	// 出力電圧目標値演算
	if (g_inv_freq_buff < INV_FREQ_1P) {
		// 出力周波数がPWM域の時
		// トルクブースト付きV/f一定制御
		inv_volt_ref = g_inv_freq_buff * VF_CTRL_RATIO + INV_VOLT_TORQUEBOOST;
	} else {
		// 出力周波数が全電圧域の時
		inv_volt_ref = 1.0;		// 出力電圧は100%をセット
	}

	// 出力電圧立ち上げ制御
	g_inv_volt_buff = inv_volt_ref * g_inv_alpha;
}


// PWM出力バッファの更新
void update_pwm_buff(void)
{
	uint32_t addr_u, addr_v, addr_w;		// 波形テーブル参照アドレス
	uint32_t data_u, data_v, data_w;		// 波形データ
	uint32_t pola_u, pola_v, pola_w;		// PMWパルス極性

	// 出力電圧・出力周波数・パルスモードの現在値 (バッファの値ではなく)
	// を使用してPWMパルスを生成しPWM出力バッファに格納する

	// 出力位相の更新
	g_inv_theta_u += g_inv_freq;
	g_inv_theta_v = g_inv_theta_u + INV_THETA_240;
	g_inv_theta_w = g_inv_theta_u + INV_THETA_120;

	// パルスモード切替
	// いずれかの相が0度または180度のタイミングで各相同時に切替
	sw_inv_pmode();

	// 変調率演算
	calc_pwm_alpha();

	// 波形テーブル設定
	set_pwm_table();

	// 波形テーブル参照アドレスの計算
	addr_u = (g_inv_theta_u >> 17) & 0x3FFFU;		// 出力位相の30bit目～17bit目
	addr_v = (g_inv_theta_v >> 17) & 0x3FFFU;		// までの14bitをアドレスとして使う
	addr_w = (g_inv_theta_w >> 17) & 0x3FFFU;

	// 波形データ取得
	data_u = *(gp_pwm_table + addr_u);
	data_v = *(gp_pwm_table + addr_v);
	data_w = *(gp_pwm_table + addr_w);

	// PWMパルス極性判定
	pola_u = (g_inv_theta_u >> 31) & 1U;		// 出力位相の最上位bitを極性データとして使う
	pola_v = (g_inv_theta_v >> 31) & 1U;		// 正の半周期の時"0", 負の半周期の時"1"
	pola_w = (g_inv_theta_w >> 31) & 1U;

	// PWMパルス生成
	// 変調率と波形データを比較して
	// 変調率の方が大きいか等しい時"1", 変調率の方が小さい時"0"を出力する (負の半周期の時は出力を反転する)
	// 変調率に最大値 (0xFF) をセットすると正の半周期は常に"1", 負の半周期は常に"0"となり
	// パルスモードや波形データに関係なく1パルスモード波形を出力する
	if (g_pwm_alpha >= data_u) {
		g_pwm_buff_u = 1 ^ pola_u;
	} else {
		g_pwm_buff_u = 0 ^ pola_u;
	}
	if (g_pwm_alpha >= data_v) {
		g_pwm_buff_v = 1 ^ pola_v;
	} else {
		g_pwm_buff_v = 0 ^ pola_v;
	}
	if (g_pwm_alpha >= data_w) {
		g_pwm_buff_w = 1 ^ pola_w;
	} else {
		g_pwm_buff_w = 0 ^ pola_w;
	}
}


// パルスモード切替
void sw_inv_pmode(void)
{
	// いずれかの相が0度または180度のタイミングで各相同時に切替
	// 出力位相の最上位bitは180度ごとに変化する
	// "1"->"0"に変化した時が0度, "0"->"1"に変化した時が180度となる

	// 切替タイミング検出処理
	// 現在の出力位相範囲を更新
	g_inv_theta_u_neg_now = (g_inv_theta_u >> 31) & 1U;		// 出力位相の最上位bitの状態で判定
	g_inv_theta_v_neg_now = (g_inv_theta_v >> 31) & 1U;
	g_inv_theta_w_neg_now = (g_inv_theta_w >> 31) & 1U;

	// いずれかの相で現在値が前回値から変化した場合に切替実行
	if ((g_inv_theta_u_neg_now != g_inv_theta_u_neg_prv)
			|| (g_inv_theta_v_neg_now != g_inv_theta_v_neg_prv)
			|| (g_inv_theta_w_neg_now != g_inv_theta_w_neg_prv)) {

		g_inv_pmode = g_inv_pmode_ref;		// パルスモード指令値を反映

	} else {
		// DO NOTHING
	}

	// 現在値を次回実行時の前回値とする
	g_inv_theta_u_neg_prv = g_inv_theta_u_neg_now;
	g_inv_theta_v_neg_prv = g_inv_theta_v_neg_now;
	g_inv_theta_w_neg_prv = g_inv_theta_w_neg_now;
}


// 変調率演算
void calc_pwm_alpha(void)
{
	if (g_inv_pmode == PMODE_1P) {
		// 1パルスモード時は常に最大値をセットする
		g_pwm_alpha = 0xFFU;
	} else {
		// 多パルスモード時は出力電圧の上位8bitを変調率として使う
		g_pwm_alpha = (g_inv_volt >> 24) & 0xFFU;
		// 1パルスモード波形になるのを防ぐため変調率を (最大値 - 1) に制限する
		if (g_pwm_alpha >= 0xFFU) {
			g_pwm_alpha = 0xFEU;
		} else {
			// DO NOTHING
		}
	}
}


// 波形テーブル設定
void set_pwm_table(void)
{
	// 使用する波形テーブルの先頭アドレスを設定する
	switch (g_inv_pmode) {
	case PMODE_45P:
		gp_pwm_table = g_pwm_table_45p;
		break;
	case PMODE_27P:
		gp_pwm_table = g_pwm_table_27p;
		break;
	case PMODE_15P:
		gp_pwm_table = g_pwm_table_15p;
		break;
	case PMODE_9P:
		gp_pwm_table = g_pwm_table_9p;
		break;
	case PMODE_5P:
		gp_pwm_table = g_pwm_table_5p;
		break;
	default:
		gp_pwm_table = g_pwm_table_3p;
		break;
	}
}


// PWM出力ピンの更新
void update_pwm_pin(void)
{
	if (g_inv_run != 0) {
		// インバータ運転時
		if (g_pwm_buff_u == 0) {	// U相PWM出力ピン更新
			PWM_U_SetLow();
		} else {
			PWM_U_SetHigh();
		}
		if (g_pwm_buff_v == 0) {	// V相PWM出力ピン更新
			PWM_V_SetLow();
		} else {
			PWM_V_SetHigh();
		}
		if (g_pwm_buff_w == 0) {	// W相PWM出力ピン更新
			PWM_W_SetLow();
		} else {
			PWM_W_SetHigh();
		}
		PWM_SD_SetLow();			// PWM強制遮断解除
	} else {
		// インバータ停止時
		PWM_SD_SetHigh();			// PWM強制遮断
		PWM_U_SetLow();				// 全相Low出力
		PWM_V_SetLow();
		PWM_W_SetLow();
	}
}


// WDTリロード
void reload_wdt(void)
{
	// リロードシーケンス
	LPC_WDT->WDFEED = 0xAA;
	LPC_WDT->WDFEED = 0x55;
}


// バッファ空エラー
void buff_empty_error(void)
{
	// V/f制御演算またはPWMパルス生成処理が間に合わなかった場合に呼ばれる
	// PWM出力を初期化してWDTによるリセットがかかるまで待機

	/* Disable SysTick IRQ */
	SysTick->CTRL &= ~SysTick_CTRL_TICKINT_Msk;

	// PWM出力初期化
	PWM_SD_SetHigh();		// PWM強制遮断
	PWM_U_SetLow();			// 全相Low出力
	PWM_V_SetLow();
	PWM_W_SetLow();

	// WDTによるリセットがかかるまで待機
	while (1) {
		__NOP();
	}
}


// SysTickタイマ割り込みハンドラ
void SysTick_Handler(void)
{
	static uint32_t nrun_count = 0;		// チャタリング除去用カウンタ

	// PWM出力ピンの更新
	update_pwm_pin();

	// V/f制御起動カウンタ+1
	g_vf_ctrl_count++;

	// V/f制御用変数の更新
	if (g_vf_ctrl_count >= VF_CTRL_COUNT) {		// V/f制御周期毎に実行
		g_vf_ctrl_count = 0;
		if (g_vf_buff_empty == 0) {
			// バッファの内容を反映する
			g_inv_volt = g_inv_volt_buff_fx;
			g_inv_freq = g_inv_freq_buff_fx;
			g_inv_pmode_ref = g_inv_pmode_buff;
			g_vf_buff_empty = 1;				// バッファ空フラグセット
		} else {
			// バッファ空エラー
			// V/f制御演算またはPWMパルス生成処理が
			// 間に合わなかった場合に呼ばれる
			buff_empty_error();
		}
	} else {
		// DO NOTHING
	}

	// ブザー鳴動間隔制御カウンタ+1
	if (g_buzzer_on != 0) {
		g_buzzer_ctrl_count++;
	} else {
		// DO NOTHING
	}

	// nRUN入力チャタリング除去
	// 設定値以上の継続した入力がある場合にインバータ運転を開始する
	if (nRUN_GetValue() == 0) {
		nrun_count++;
		if (nrun_count >= CHATTERING_COUNT) {
			nrun_count = CHATTERING_COUNT;
			g_inv_run_buff = 1;
		} else {
			// DO NOTHING
		}
	} else {
		nrun_count = 0;
		g_inv_run_buff = 0;
	}

	// PWM出力バッファの更新
	update_pwm_buff();

	// タイムアウトまでの余裕はこのくらい
//	volatile uint32_t i;
//	for (i = 0; i < 102; i++) {
//		__NOP();
//	}
}
