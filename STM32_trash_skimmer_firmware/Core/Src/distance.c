
#include "main.h"
#include <stdio.h>

UART_HandleTypeDef huart2;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);

int __io_putchar(int ch)
{
    HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}

void Error_Handler(void)
{
    __disable_irq();
    while (1)
    {
    }
}

void delay_us(uint32_t us)
{
    us *= (SystemCoreClock / 1000000) / 4;

    while (us--)
    {
        __NOP();
    }
}

float Read_Ultrasonic_Distance(GPIO_TypeDef* trig_port,
                               uint16_t trig_pin,
                               GPIO_TypeDef* echo_port,
                               uint16_t echo_pin)
{
    HAL_GPIO_WritePin(trig_port, trig_pin, GPIO_PIN_RESET);
    delay_us(2);

    HAL_GPIO_WritePin(trig_port, trig_pin, GPIO_PIN_SET);
    delay_us(12);

    HAL_GPIO_WritePin(trig_port, trig_pin, GPIO_PIN_RESET);

    uint32_t timeout = 0;

    while (HAL_GPIO_ReadPin(echo_port, echo_pin) == GPIO_PIN_RESET)
    {
        timeout++;

        if (timeout > 40000)
        {
            return -1.0f;
        }
    }

    uint32_t echo_time = 0;

    while (HAL_GPIO_ReadPin(echo_port, echo_pin) == GPIO_PIN_SET)
    {
        echo_time++;
        delay_us(1);

        if (echo_time > 30000)
        {
            break;
        }
    }

    float distance = (float)echo_time * 0.0343f / 2.0f;

    return distance;
}

int main(void)
{
    HAL_Init();
    SystemClock_Config();

    MX_GPIO_Init();
    MX_USART2_UART_Init();

    while (1)
    {
        float left_dist = Read_Ultrasonic_Distance(
            GPIOA, GPIO_PIN_9,
            GPIOA, GPIO_PIN_8
        );

        HAL_Delay(50);

        float right_dist = Read_Ultrasonic_Distance(
            GPIOB, GPIO_PIN_6,
            GPIOB, GPIO_PIN_7
        );

        // Check LEFT sensor
        if (left_dist >= 0 && left_dist < 7.0f)
        {
            printf("WARNING: LEFT TOO CLOSE!\r\n");
        }

        // Check RIGHT sensor
        if (right_dist >= 0 && right_dist < 7.0f)
        {
            printf("WARNING: RIGHT TOO CLOSE!\r\n");
        }

        // Print distances
        printf("Left: ");

        if (left_dist < 0)
            printf("Out of range");
        else
            printf("%d cm", (int)left_dist);

        printf(" | Right: ");

        if (right_dist < 0)
            printf("Out of range");
        else
            printf("%d cm", (int)right_dist);

        printf("\r\n");

        HAL_Delay(300);
    }
}

// SYSTEM CLOCK CONFIGURATION

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSI_CALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM = 16;
    RCC_OscInitStruct.PLL.PLLN = 336;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
    RCC_OscInitStruct.PLL.PLLQ = 7;

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK |
                                  RCC_CLOCKTYPE_SYSCLK |
                                  RCC_CLOCKTYPE_PCLK1 |
                                  RCC_CLOCKTYPE_PCLK2;

    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
    {
        Error_Handler();
    }
}

// UART CONFIGURATION

static void MX_USART2_UART_Init(void)
{
    __HAL_RCC_USART2_CLK_ENABLE();

    huart2.Instance = USART2;
    huart2.Init.BaudRate = 115200;
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
}

// GPIO CONFIGURATION

static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    // LEFT TRIG - PA9
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_RESET);

    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // LEFT ECHO - PA8
    GPIO_InitStruct.Pin = GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // RIGHT TRIG - PB6
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);

    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // RIGHT ECHO - PB7
    GPIO_InitStruct.Pin = GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}
