 /**
  ******************************************************************************
  * @file    main.c
  * @author  fire
  * @version V1.0
  * @date    2013-xx-xx
  * @brief   华邦 8M串行flash测试，并将测试信息通过串口1在电脑的超级终端中打印出来
  ******************************************************************************
  * @attention
  *
  * 实验平台:野火 F103-指南者 STM32 开发板 
  * 论坛    :http://www.firebbs.cn
  * 淘宝    :https://fire-stm32.taobao.com
  *
  ******************************************************************************
  */ 
#include "stm32f10x.h"
#include "./usart/bsp_usart.h"
#include "./led/bsp_led.h"
#include "./flash/bsp_spi_flash.h"
#include "stdio.h"
#include "string.h"

typedef enum { FAILED = 0, PASSED = !FAILED} TestStatus;

/* 获取缓冲区的长度 */
#define TxBufferSize1   (countof(TxBuffer1) - 1)
#define RxBufferSize1   (countof(TxBuffer1) - 1)
#define countof(a)      (sizeof(a) / sizeof(*(a)))
#define  BufferSize (countof(Tx_Buffer)-1)

#define  FLASH_WriteAddress     0x00000
#define  FLASH_ReadAddress      FLASH_WriteAddress
#define  FLASH_SectorToErase    FLASH_WriteAddress

typedef struct
{
	u8 rxbuff[128];
	u8 cnt;
	u8 mode;
	u8 success;
}receive_;	
receive_ r;
/* 发送缓冲区初始化 */
uint8_t Tx_Buffer[] = "感谢您选用野火stm32开发板\r\n";
uint8_t Rx_Buffer[BufferSize];

__IO uint32_t DeviceID = 0;
__IO uint32_t FlashID = 0;
__IO TestStatus TransferStatus1 = FAILED;

// 函数原型声明
void Delay(__IO uint32_t nCount);
TestStatus Buffercmp(uint8_t* pBuffer1,uint8_t* pBuffer2, uint16_t BufferLength);

/*
 * 函数名：main
 * 描述  ：主函数
 * 输入  ：无
 * 输出  ：无
 * 提示  ：不要乱盖PC0跳帽！！
 */
int main(void)
{ 	
	LED_GPIO_Config();
	LED_BLUE;
	
	/* 配置串口为：115200 8-N-1 */
	USART_Config();
	printf("\r\n 这是一个8Mbyte串行flash(W25Q64)实验 \r\n");
	printf("\r\n 使用指南者底板时 左上角排针位置 不要将PC0盖有跳帽 防止影响PC0做SPIFLASH片选脚 \r\n");

	/* 8M串行flash W25Q64初始化 */
	SPI_FLASH_Init();
	
	/* 获取 Flash Device ID */
	DeviceID = SPI_FLASH_ReadDeviceID();	
	Delay( 200 );
	
	/* 获取 SPI Flash ID */
	FlashID = SPI_FLASH_ReadID();	
	printf("\r\n FlashID is 0x%X,\
	Manufacturer Device ID is 0x%X\r\n", FlashID, DeviceID);
	
	/* 检验 SPI Flash ID */
	if (FlashID == sFLASH_ID)
	{	
		printf("\r\n 检测到串行flash W25Q64 !\r\n");
		
		/* 擦除将要写入的 SPI FLASH 扇区，FLASH写入前要先擦除 */
		// 这里擦除4K，即一个扇区，擦除的最小单位是扇区
		SPI_FLASH_SectorErase(FLASH_SectorToErase);	 	 
		
		/* 将发送缓冲区的数据写到flash中 */
		// 这里写一页，一页的大小为256个字节
		SPI_FLASH_BufferWrite(Tx_Buffer, FLASH_WriteAddress, BufferSize);		
		printf("\r\n 写入的数据为：%s \r\t", Tx_Buffer);
		
		/* 将刚刚写入的数据读出来放到接收缓冲区中 */
		SPI_FLASH_BufferRead(Rx_Buffer, FLASH_ReadAddress, BufferSize);
		printf("\r\n 读出的数据为：%s \r\n", Rx_Buffer);
		
		/* 检查写入的数据与读出的数据是否相等 */
		TransferStatus1 = Buffercmp(Tx_Buffer, Rx_Buffer, BufferSize);
		
		if( PASSED == TransferStatus1 )
		{ 
			LED_GREEN;
			printf("\r\n 8M串行flash(W25Q64)测试成功!\n\r");
		}
		else
		{        
			LED_RED;
			printf("\r\n 8M串行flash(W25Q64)测试失败!\n\r");
		}
	}// if (FlashID == sFLASH_ID)
	else// if (FlashID == sFLASH_ID)
	{ 
		LED_RED;
		printf("\r\n 获取不到 W25Q64 ID!\n\r");
	}
	
	while(1)
	{
		if(r.success == 1)
		{
			printf("success");
			r.success = 0;
			r.cnt = 0;
		}
	}
}

/*
 * 函数名：Buffercmp
 * 描述  ：比较两个缓冲区中的数据是否相等
 * 输入  ：-pBuffer1     src缓冲区指针
 *         -pBuffer2     dst缓冲区指针
 *         -BufferLength 缓冲区长度
 * 输出  ：无
 * 返回  ：-PASSED pBuffer1 等于   pBuffer2
 *         -FAILED pBuffer1 不同于 pBuffer2
 */
TestStatus Buffercmp(uint8_t* pBuffer1, uint8_t* pBuffer2, uint16_t BufferLength)
{
  while(BufferLength--)
  {
    if(*pBuffer1 != *pBuffer2)
    {
      return FAILED;
    }

    pBuffer1++;
    pBuffer2++;
  }
  return PASSED;
}

void Delay(__IO uint32_t nCount)
{
  for(; nCount != 0; nCount--);
}


/***********************************************************************
函数名称：hex2str
函数功能：将十六进制数转换为字符串
输入参数：
    hexdata 表示输入的十六进制数
    s 表示字符指针指向存储的结果字符串 
    length 表示字符个数
************************************************************************/
static void hex2str(char hexdata, char* s, char length)
{
    int k;
    s[length] = 0;
    /* 一个字符4Bit
    hexdata >>= 4  每次运算完一个十六进制转字符之后右移4位二进制 */
    for (k = length - 1; k >= 0; k--, hexdata >>= 4)
    {
				if ((hexdata & 0xF) <= 9)//对十六进制中的数据进行处理 
           {
                s[k] = (hexdata & 0xF) + '0';//数字变成字符
           }
        else
            {
                s[k] = (hexdata & 0xF) + 'A' - 0x0A;//字母变成字符
            }
    }
}
//hex校验转2ascii
u16 check(u8 data[])
{
	u8 ii,checksum,m[3];
	char ascII[2];
	u16 checkValue;
	
	checksum = data[0];
	for(ii = 1 ; ii < 18 ; ii++)
	{
		checksum = checksum ^ data[ii];
	}
	m[0] = checksum;
	
	hex2str(m[0] , ascII , 2);
	
	checkValue = (u16)(ascII[0] << 8 | ascII[1]);
	
	
	return checkValue;
}


double fff,fff1;
char k3[4]="1234";
char k4[6]="123456";

//传感器解析
void dataAnalysis(u8 data)
{
	u8 temp;
	u16 checkValue , checkValue1 ;

	temp = data;
	if(temp == 'A')
	{
		r.mode = 1;
	}
	
	if(r.success == 1)
	{
		return;
	}
	
	if(r.mode == 1)
	{
		r.rxbuff[r.cnt++] = temp;
		
		if(r.cnt == 22)//success
		{
			checkValue = check(r.rxbuff);//校验
			checkValue1 = (u16)(r.rxbuff[18] << 8 | r.rxbuff[19]);
			
			if((checkValue == checkValue1) && (r.rxbuff[20] == '\r') && (r.rxbuff[21] == '\n'))
			{
				r.success = 1;
				r.mode = 0;
				printf("\r\n");
				memcpy(k3 , &r.rxbuff[10] , 4);
				sscanf(k3 ," %lf" , &fff);//浓度值
				memcpy(k4 , &r.rxbuff[2] , 6);
				sscanf(k4 ," %lf" , &fff1);//温度值
			}
			else
			{
				r.success = 1;
				r.mode = 0;
				printf("fail");
			}
		}
	}
}





/*********************************************END OF FILE**********************/
