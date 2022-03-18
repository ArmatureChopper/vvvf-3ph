/*
 * pin_config.h
 *
 *  Created on: 2022/03/02
 *      Author: @ArmatureChopper
 */

#ifndef PIN_CONFIG_H_
#define PIN_CONFIG_H_


// ピン番号設定 & GPIO操作関数マクロ

// ブザー制御信号出力ピン
#define BUZZER_PORT		0
#define BUZZER_PIN		2
#define BUZZER_SetHigh()				(LPC_GPIO0->FIOSET = (1 << 2))
#define BUZZER_SetLow()					(LPC_GPIO0->FIOCLR = (1 << 2))
#define BUZZER_SetDigitalOutput()		(LPC_GPIO0->FIODIR |= (1 << 2))

// OCRトリップ表示LED出力ピン
#define LED_OCR_PORT	0
#define LED_OCR_PIN		9
#define LED_OCR_SetHigh()				(LPC_GPIO0->FIOSET = (1 << 9))
#define LED_OCR_SetLow()				(LPC_GPIO0->FIOCLR = (1 << 9))
#define LED_OCR_SetDigitalOutput()		(LPC_GPIO0->FIODIR |= (1 << 9))

// インバータ運転表示LED出力ピン
#define LED_RUN_PORT	0
#define LED_RUN_PIN		22
#define LED_RUN_SetHigh()				(LPC_GPIO0->FIOSET = (1 << 22))
#define LED_RUN_SetLow()				(LPC_GPIO0->FIOCLR = (1 << 22))
#define LED_RUN_SetDigitalOutput()		(LPC_GPIO0->FIODIR |= (1 << 22))

// U相PWM出力ピン
#define PWM_U_PORT		2
#define PWM_U_PIN		0
#define PWM_U_SetHigh()					(LPC_GPIO2->FIOSET = (1 << 0))
#define PWM_U_SetLow()					(LPC_GPIO2->FIOCLR = (1 << 0))
#define PWM_U_SetDigitalOutput()		(LPC_GPIO2->FIODIR |= (1 << 0))

// V相PWM出力ピン
#define PWM_V_PORT		2
#define PWM_V_PIN		1
#define PWM_V_SetHigh()					(LPC_GPIO2->FIOSET = (1 << 1))
#define PWM_V_SetLow()					(LPC_GPIO2->FIOCLR = (1 << 1))
#define PWM_V_SetDigitalOutput()		(LPC_GPIO2->FIODIR |= (1 << 1))

// W相PWM出力ピン
#define PWM_W_PORT		2
#define PWM_W_PIN		2
#define PWM_W_SetHigh()					(LPC_GPIO2->FIOSET = (1 << 2))
#define PWM_W_SetLow()					(LPC_GPIO2->FIOCLR = (1 << 2))
#define PWM_W_SetDigitalOutput()		(LPC_GPIO2->FIODIR |= (1 << 2))

// PWM強制遮断信号出力ピン
#define PWM_SD_PORT		2
#define PWM_SD_PIN		3
#define PWM_SD_SetHigh()				(LPC_GPIO2->FIOSET = (1 << 3))
#define PWM_SD_SetLow()					(LPC_GPIO2->FIOCLR = (1 << 3))
#define PWM_SD_SetDigitalOutput()		(LPC_GPIO2->FIODIR |= (1 << 3))

// 保護動作信号入力ピン
#define FAULT_PORT		2
#define FAULT_PIN		4
#define FAULT_GetValue()				(LPC_GPIO2->FIOPIN & (1 << 4))

// 冷却ファン制御信号出力ピン
#define FAN_CTRL_PORT	2
#define FAN_CTRL_PIN	5
#define FAN_CTRL_SetHigh()				(LPC_GPIO2->FIOSET = (1 << 5))
#define FAN_CTRL_SetLow()				(LPC_GPIO2->FIOCLR = (1 << 5))
#define FAN_CTRL_SetDigitalOutput()		(LPC_GPIO2->FIODIR |= (1 << 5))
#define FAN_CTRL_SetPWMEnable()			(LPC_PINCON->PINSEL4 |= (1 << 10))
#define FAN_CTRL_SetPWMDisable()		(LPC_PINCON->PINSEL4 &= ~(1 << 10))

// OCRトリップ信号入力ピン (負論理)
#define nOCR_PORT		2
#define nOCR_PIN		6
#define nOCR_GetValue()					(LPC_GPIO2->FIOPIN & (1 << 6))

// インバータ運転指令信号入力ピン (負論理)
#define nRUN_PORT		2
#define nRUN_PIN		7
#define nRUN_GetValue()					(LPC_GPIO2->FIOPIN & (1 << 7))


#endif /* PIN_CONFIG_H_ */
