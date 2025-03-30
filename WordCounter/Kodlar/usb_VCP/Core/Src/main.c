/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usbd_cdc_if.h"
#include "usb_device.h"
#include "stm32f4xx_hal.h"

// Pin tanimlari
#define SEGMENT_COUNT 7
#define DIGIT_COUNT 2

// Segment pinleri
uint16_t segmentPins[SEGMENT_COUNT] = {GPIO_PIN_8, GPIO_PIN_9, GPIO_PIN_10, GPIO_PIN_11, GPIO_PIN_12, GPIO_PIN_13, GPIO_PIN_14};

// Digit pinleri (2 basamakli gösterge için)
uint16_t digitPins[DIGIT_COUNT] = {GPIO_PIN_0, GPIO_PIN_1};

// Rakam desenleri (7 segment)
uint8_t digitPatterns[10][SEGMENT_COUNT] = {
    {1, 1, 1, 1, 1, 1, 0}, // 0
    {0, 1, 1, 0, 0, 0, 0}, // 1
    {1, 1, 0, 1, 1, 0, 1}, // 2
    {1, 1, 1, 1, 0, 0, 1}, // 3
    {0, 1, 1, 0, 0, 1, 1}, // 4
    {1, 0, 1, 1, 0, 1, 1}, // 5
    {1, 0, 1, 1, 1, 1, 1}, // 6
    {1, 1, 1, 0, 0, 0, 0}, // 7
    {1, 1, 1, 1, 1, 1, 1}, // 8
    {1, 1, 1, 1, 0, 1, 1}  // 9
};

void SystemClock_Config(void);
void GPIO_Init(void);
void displayDigit(uint8_t digit, uint8_t position);
void display_correct_words(int number);


/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "fonts.h"         //fontlari kullanmak için
#include "ssd1306.h"       //OLED gösterge komutlarini içerir
//#include "test.h"          //test fonksyonlari için
//#include "bitmap.h"       //ege logo için
//#include "horse_anim.h"   //animasyon için
#include "stdlib.h"       // komutlari için
#include "stdio.h"       // c i/o komutlari için
#include "string.h"       // dizi komutlarini kullanmak için

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;
UART_HandleTypeDef huart2;
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART2_UART_Init(void);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
//uint32_t say=0;
//char dizi[10];
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USB_DEVICE_Init();
	GPIO_Init();
	MX_I2C1_Init();
	MX_USART2_UART_Init();
	


//UART üzerinden veri gelme kontrolü

while(1){
	while(1){
	//Veri gelene kadar bekle
	        if (received_flag) { // USB'den veri alindiginda kontrol et
            received_flag = 0; // Bayragi sifirla
						break;}

	}

 if (strcmp(CDC_Buffer, "a") == 0) {
		//Başlama komutu gelince ekrana "Start! yazdır.
		SSD1306_Init();  // initialise
    SSD1306_GotoXY (20,20);
    SSD1306_Puts ("Start!", &Font_16x26, 1);
    SSD1306_UpdateScreen(); //display
			
		
		uint8_t count = 30;  // 30'dan basla
    uint8_t tens = 0;
    uint8_t ones = 0;

    
        // Sayaci sürekli olarak bastan baslat
        for (count = 30; count >0 ; count--) {
            tens = count / 10; // Onlar basamagi
            ones = count % 10; // Birler basamagi

            // Her rakami kisa süreyle göster (multiplexing)
            for (uint8_t t = 0; t < 100; t++) {
                displayDigit(tens, 0);  // Onlar basamagini göster
                HAL_Delay(4);          // 5ms bekle
                displayDigit(ones, 1); // Birler basamagini göster
                HAL_Delay(4);          // 5ms bekle
            }
						    
        }
				// Sayacı kapat.
				for (uint8_t i = 0; i < DIGIT_COUNT; i++)
					{
        HAL_GPIO_WritePin(GPIOB, digitPins[i], GPIO_PIN_SET); // Kapalı = LOW
				}
				
//	
	 }
	else{
		// Başlama komutu hariç gelen veriyi doğru kelime sayısı olarak OLED ekrana yazdır.
		int number = atoi(CDC_Buffer);
		display_correct_words(number);
		received_flag = 0;
	}
}	
	
}

// Belirli bir rakami ve pozisyonu göster
void displayDigit(uint8_t digit, uint8_t position)
{
    // Tüm segmentleri kapat
    for (uint8_t i = 0; i < SEGMENT_COUNT; i++) {
        HAL_GPIO_WritePin(GPIOB, segmentPins[i], GPIO_PIN_RESET);
    }

    // Tüm digit pinlerini kapat
    for (uint8_t i = 0; i < DIGIT_COUNT; i++) {
        HAL_GPIO_WritePin(GPIOB, digitPins[i], GPIO_PIN_RESET);
    }

    // Ilgili rakamin segmentlerini aç
    for (uint8_t i = 0; i < SEGMENT_COUNT; i++) {
        HAL_GPIO_WritePin(GPIOB, segmentPins[i], digitPatterns[digit][i] ? GPIO_PIN_SET : GPIO_PIN_RESET);
    }

    // Ilgili digit pinini aktif et
    HAL_GPIO_WritePin(GPIOB, digitPins[position], GPIO_PIN_SET);
}

// GPIO yapilandirmasi
void GPIO_Init(void)
{
    __HAL_RCC_GPIOB_CLK_ENABLE();  // GPIOB portunu etkinlestir

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // Segment pinlerini çikis olarak yapilandir
    GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // Digit pinlerini çikis olarak yapilandir
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}


// uart veri bekleme fonksiyonu


// "begin" komutunu bekleyen fonksiyon
void wait_for_begin_command(void) {
    while (1) {
            if (strcmp(CDC_Buffer, "begin") == 0) {
                // "begin" komutu alindiginda döngüden çik
                break;
            }
        
    }
}

void display_correct_words(int number) {
		SSD1306_Fill(SSD1306_COLOR_BLACK); // Ekranı temizle
		SSD1306_GotoXY(5,5);
		SSD1306_Puts("True Words", &Font_11x18, 1);
    SSD1306_GotoXY(45,30);
    SSD1306_Puts(CDC_Buffer, &Font_16x26, 1);
    SSD1306_UpdateScreen();
}


/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */


static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 9600;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}


static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 400000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
