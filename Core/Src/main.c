/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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
#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define UART_TX_BUFFER_SIZE 64 /**< Taille du buffer d'émission UART */
#define UART_RX_BUFFER_SIZE 1  /**< Taille du buffer de réception UART */
#define CMD_BUFFER_SIZE     64 /**< Taille du buffer de commande */
#define MAX_ARGS            9  /**< Nombre maximum d'arguments */

// Constantes pour la fonction de changement de vitesse

#define SPEED_STEPS 100  // Nombre d'étapes pour un changement en douceur
#define SPEED_DELAY 15   // Délai entre chaque étape (en ms)

/** Codes ASCII */

#define ASCII_LF  0x0A /**< Line Feed (saut de ligne) */
#define ASCII_CR  0x0D /**< Carriage Return (retour chariot) */
#define ASCII_DEL 0x7F /**< Delete (suppression) */



/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

static int counter = 0;

/** @brief Prompt affiché */
const uint8_t prompt[] = "user@Nucleo-STM32G431>>";

/** @brief Message de bienvenue au démarrage du microprocesseur */
const uint8_t started[] =
    "\r\n*-----------------------------*"
    "\r\n| Welcome on Nucleo-STM32G431 |"
    "\r\n*-----------------------------*"
    "\r\n";

/** @brief chaîne pour le retour à la ligne */
const uint8_t newLine[] = "\r\n";


/**@brief Message pour la vitesse */
const uint8_t speedMsg[] = "Speed has been changed\r\n";

/** @brief Message d'aide listant les fonctions disponibles */
const uint8_t helpMsg[] =
    "Available commands:\r\n"
    "help    : Display this help message\r\n"
    "pinout  : Display the list of used pins\r\n"
    "start   : Power ON the motor stage\r\n"
    "stop    : Power OFF the motor stage\r\n";

/** @brief Liste des broches utilisées */
const uint8_t pinout[] =
    "Used pins:\r\n"
    "PA5 : LED\r\n"
    "PA2 : USART2_TX\r\n"
    "PA3 : USART2_RX\r\n";

/** @brief Message indiquant l'allumage du moteur */
const uint8_t powerOn[] = "Power ON\r\n";

/** @brief Message indiquant l'extinction du moteur */
const uint8_t powerOff[] = "Power OFF\r\n";

/** @brief Message pour commande non reconnue */
const uint8_t cmdNotFound[] = "Command not found\r\n";

/** @brief Flag indiquant la réception d'un caractère sur l'UART */
uint32_t uartRxReceived;

/** @brief Buffer de réception UART */
uint8_t uartRxBuffer[UART_RX_BUFFER_SIZE];

/** @brief Buffer d'émission UART */
uint8_t uartTxBuffer[UART_TX_BUFFER_SIZE];

/** @brief Buffer contenant la commande en cours */
char cmd[CMD_BUFFER_SIZE];

/** @brief Index pour le prochain caractère à remplir */
int idxCmd;

/** @brief Variables globales de courant et de tension adc */
float tension_adc, courant;

/** @brief Déclaration du buffer DMA */

uint16_t adc_buffer[1];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void changeSpeed(uint16_t speed);
void start();
void stop();
void ADC_conversion();
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int __io_putchar(int ch)
{
	HAL_UART_Transmit(&huart2, (uint8_t*)&ch, 1, HAL_MAX_DELAY);

	return ch;
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
  /** @brief Tableau des arguments extraits de la commande */
  char* argv[MAX_ARGS];

  /** @brief Nombre d'arguments */
  int argc = 0;

  /** @brief Token pour l'analyse de la chaîne */
  char* token;

  /** @brief Flag indiquant qu'une nouvelle commande est prête */
  int newCmdReady = 0;

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  idxCmd = 0;
  memset(cmd, '\0', CMD_BUFFER_SIZE);
  memset(argv, 0, MAX_ARGS * sizeof(char*));
  memset(uartRxBuffer, 0, UART_RX_BUFFER_SIZE);
  memset(uartTxBuffer, 0, UART_TX_BUFFER_SIZE);
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC2_Init();
  MX_ADC1_Init();
  MX_TIM1_Init();
  MX_TIM3_Init();
  MX_USART2_UART_Init();
  MX_USART3_UART_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */



  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_buffer, 1);
  HAL_TIM_Base_Start(&htim1);

  HAL_UART_Receive_IT(&huart2, uartRxBuffer, UART_RX_BUFFER_SIZE);
  HAL_Delay(10);
  HAL_UART_Transmit(&huart2, started, strlen((char*)started), HAL_MAX_DELAY);
  HAL_UART_Transmit(&huart2, prompt, strlen((char*)prompt), HAL_MAX_DELAY);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  while (1)
  {
	/**
	   * @brief Vérifie si un caractère a été reçu sur l'UART.
	   */
    if (uartRxReceived)
    {
      uint8_t receivedChar = uartRxBuffer[0];
      /**
        * @brief Écho du caractère reçu sur l'UART.
        */
      HAL_UART_Transmit(&huart2, &receivedChar, 1, HAL_MAX_DELAY);

      /**
        * @brief Traitement du caractère reçu.
        */
      switch (receivedChar)
      {
      /**
         * @brief Cas du retour chariot ou du saut de ligne : traite la commande entrée.
         */
        case ASCII_CR:
        case ASCII_LF:
          HAL_UART_Transmit(&huart2, newLine, strlen((char*)newLine), HAL_MAX_DELAY);
          cmd[idxCmd] = '\0';
          argc = 0;
          token = strtok(cmd, " ");
          while (token != NULL && argc < MAX_ARGS)
          {
            argv[argc++] = token;
            token = strtok(NULL, " ");
          }

          idxCmd = 0;
          newCmdReady = 1;
          break;
          /**
              * @brief Cas de la suppression : efface le dernier caractère saisi.
              */
        case ASCII_DEL:
          if (idxCmd > 0)
          {
            idxCmd--;
            cmd[idxCmd] = '\0';
            // Envoyer un retour en arrière pour effacer dans le terminal
            uint8_t backspace = '\b';
            HAL_UART_Transmit(&huart2, &backspace, 1, HAL_MAX_DELAY);
          }
          break;

        /**
           * @brief Cas par défaut : ajoute le caractère au buffer de commande.
           */
        default:
          if (idxCmd < CMD_BUFFER_SIZE - 1)
          {
            cmd[idxCmd++] = receivedChar;
          }
          else
          {

            idxCmd = 0;
            memset(cmd, '\0', CMD_BUFFER_SIZE);
            HAL_UART_Transmit(&huart2, (uint8_t*)"Command too long\r\n", 18, HAL_MAX_DELAY);
            HAL_UART_Transmit(&huart2, prompt, strlen((char*)prompt), HAL_MAX_DELAY);
          }
          break;
      }
      uartRxReceived = 0;
      /**
        * @brief Relance la réception UART en interruption.
        */
      HAL_UART_Receive_IT(&huart2, uartRxBuffer, UART_RX_BUFFER_SIZE);
    }
    /**
      * @brief Vérifie si une nouvelle commande est prête à être traitée.
      */
    if (newCmdReady)
    {
      newCmdReady = 0;
      if (argc > 0)
      {
    	/**
    	  * @brief Traite les commandes reconnues : help, pinout, start, stop.
    	  */
        if (strcmp(argv[0], "help") == 0)
        {
          HAL_UART_Transmit(&huart2, helpMsg, strlen((char*)helpMsg), HAL_MAX_DELAY);
        }
        else if (strcmp(argv[0], "pinout") == 0)
        {
          HAL_UART_Transmit(&huart2, pinout, strlen((char*)pinout), HAL_MAX_DELAY);
        }
        else if (strcmp(argv[0], "start") == 0)
        {
        	start();
          HAL_UART_Transmit(&huart2, powerOn, strlen((char*)powerOn), HAL_MAX_DELAY);
        }
        else if (strcmp(argv[0], "stop") == 0)
        {
          stop();
          HAL_UART_Transmit(&huart2, powerOff, strlen((char*)powerOff), HAL_MAX_DELAY);
        }
        else if(strcmp(argv[0], "adc") == 0){
        	ADC_conversion();
            printf("Tension adc : %.3f V\r\n", tension_adc);
            printf("I_moteur : %.3f A\r\n", courant);

        }
        else if (strcmp(argv[0], "speed") == 0) {
            if (argc > 1) {
            	changeSpeed(atoi(argv[1]));
                HAL_UART_Transmit(&huart2, speedMsg, strlen((char*)speedMsg), HAL_MAX_DELAY);
            } else {
                HAL_UART_Transmit(&huart2, (uint8_t*)"Speed value missing\r\n", 21, HAL_MAX_DELAY);
            }
        }
        else
        {
          HAL_UART_Transmit(&huart2, cmdNotFound, strlen((char*)cmdNotFound), HAL_MAX_DELAY);
        }
      }
      // Affiche le prompt
      HAL_UART_Transmit(&huart2, prompt, strlen((char*)prompt), HAL_MAX_DELAY);

      // Réinitialise le buffer de commande et l'index
      idxCmd = 0;
      memset(cmd, '\0', CMD_BUFFER_SIZE);
    }
  }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */


  /* USER CODE END 3 */
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
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV6;
  RCC_OscInitStruct.PLL.PLLN = 85;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
/**
  * @brief Callback appelé à la fin de la réception UART.
  * @param huart Handle de l'UART.
  * @retval None
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART2)
  {
    uartRxReceived = 1;
    // La relance de la réception UART est effectuée dans la boucle principale
  }
}

/** @brief Fonction qui genère les PWM avec un rapport cyclique initial de 50%
 */
void start(){


	  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
	  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);

	  HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);
	  HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2);
	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 512);
	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 512);



}

/** @brief Fonction qui stoppe la génération des PWM
 */
void stop(){

	  HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
	  HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
	  HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_1);
	  HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_2);

}




/** @brief Fonction qui change la vitesse du moteur en modifiant le rapport cyclique des PWM
	* @params targetspeed : vitesse d'entrée
 */

void changeSpeed(uint16_t targetSpeed) {
    static uint16_t currentSpeed = 512;  // Vitesse actuelle (persistant puisque static)

    if (targetSpeed > 100) {
        char errorMsg[50];
        sprintf(errorMsg, "Valeur de speed incorrecte: %d. Doit être entre 0 et 100.\r\n", targetSpeed);
        HAL_UART_Transmit(&huart2, (uint8_t*)errorMsg, strlen(errorMsg), HAL_MAX_DELAY);
        return;
    }

    // Converti la vitesse de 0-100 à 0-1024
    uint16_t targetValue = (targetSpeed * 1024) / 100;

    // pas de changement
    int16_t step = (targetValue - currentSpeed) / SPEED_STEPS;

    // Change progressivement la vitesse
    for (int i = 0; i < SPEED_STEPS; i++) {
        currentSpeed += step;

        // currentSpeed reste dans les limites
        if (currentSpeed > 1024) currentSpeed = 1024;
        if (currentSpeed < 0) currentSpeed = 0;

        uint16_t speed_channel_1 = currentSpeed;
        uint16_t speed_channel_2 = 1024 - currentSpeed;

        // Mise à jour du rapport cyclique
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, speed_channel_1);
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, speed_channel_2);

        HAL_Delay(SPEED_DELAY);  // Petit délai pour une transition en douceur
    }

    // Assurer que la vitesse finale est exactement celle demandée
    currentSpeed = targetValue;
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, currentSpeed);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 1024 - currentSpeed);

}

/** @brief Fonction qui fait la conversion de l'entrée de l'adc(capteur de courant a effet hall) en courant (de la mcc)
 */
void ADC_conversion() {

    float sensibilite = 0.05;
    float Vref = 1.65;

    float v_adc = (adc_buffer[0] * 3.3f) / 4096.0f;
    float I_courant = (Vref - v_adc) / sensibilite;

    tension_adc = v_adc;
	courant = I_courant;

    //printf("Tension adc : %.3f V\r\n", v_adc);
    //printf("I_moteur : %.3f A\r\n", I_courant);

}




/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

	if (htim->Instance == TIM1) {
		counter++;
		HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_buffer, 1);
		}

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */


  /* USER CODE END Callback 1 */
}

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
