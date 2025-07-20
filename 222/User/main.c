 /**
  ******************************************************************************
  * @file    main.c
  * @author  fire
  * @version V1.0
  * @date    2013-xx-xx
  * @brief   ���� 8M����flash���ԣ�����������Ϣͨ������1�ڵ��Եĳ����ն��д�ӡ����
  ******************************************************************************
  * @attention
  *
  * ʵ��ƽ̨:Ұ�� F103-ָ���� STM32 ������ 
  * ��̳    :http://www.firebbs.cn
  * �Ա�    :https://fire-stm32.taobao.com
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

/* ��ȡ�������ĳ��� */
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
/* ���ͻ�������ʼ�� */
uint8_t Tx_Buffer[] = "��л��ѡ��Ұ��stm32������\r\n";
uint8_t Rx_Buffer[BufferSize];

__IO uint32_t DeviceID = 0;
__IO uint32_t FlashID = 0;
__IO TestStatus TransferStatus1 = FAILED;

// ����ԭ������
void Delay(__IO uint32_t nCount);
TestStatus Buffercmp(uint8_t* pBuffer1,uint8_t* pBuffer2, uint16_t BufferLength);

/*
 * ��������main
 * ����  ��������
 * ����  ����
 * ���  ����
 * ��ʾ  ����Ҫ�Ҹ�PC0��ñ����
 */
int main(void)
{ 	
	LED_GPIO_Config();
	LED_BLUE;
	
	/* ���ô���Ϊ��115200 8-N-1 */
	USART_Config();
	printf("\r\n ����һ��8Mbyte����flash(W25Q64)ʵ�� \r\n");
	printf("\r\n ʹ��ָ���ߵװ�ʱ ���Ͻ�����λ�� ��Ҫ��PC0������ñ ��ֹӰ��PC0��SPIFLASHƬѡ�� \r\n");

	/* 8M����flash W25Q64��ʼ�� */
	SPI_FLASH_Init();
	
	/* ��ȡ Flash Device ID */
	DeviceID = SPI_FLASH_ReadDeviceID();	
	Delay( 200 );
	
	/* ��ȡ SPI Flash ID */
	FlashID = SPI_FLASH_ReadID();	
	printf("\r\n FlashID is 0x%X,\
	Manufacturer Device ID is 0x%X\r\n", FlashID, DeviceID);
	
	/* ���� SPI Flash ID */
	if (FlashID == sFLASH_ID)
	{	
		printf("\r\n ��⵽����flash W25Q64 !\r\n");
		
		/* ������Ҫд��� SPI FLASH ������FLASHд��ǰҪ�Ȳ��� */
		// �������4K����һ����������������С��λ������
		SPI_FLASH_SectorErase(FLASH_SectorToErase);	 	 
		
		/* �����ͻ�����������д��flash�� */
		// ����дһҳ��һҳ�Ĵ�СΪ256���ֽ�
		SPI_FLASH_BufferWrite(Tx_Buffer, FLASH_WriteAddress, BufferSize);		
		printf("\r\n д�������Ϊ��%s \r\t", Tx_Buffer);
		
		/* ���ո�д������ݶ������ŵ����ջ������� */
		SPI_FLASH_BufferRead(Rx_Buffer, FLASH_ReadAddress, BufferSize);
		printf("\r\n ����������Ϊ��%s \r\n", Rx_Buffer);
		
		/* ���д�������������������Ƿ���� */
		TransferStatus1 = Buffercmp(Tx_Buffer, Rx_Buffer, BufferSize);
		
		if( PASSED == TransferStatus1 )
		{ 
			LED_GREEN;
			printf("\r\n 8M����flash(W25Q64)���Գɹ�!\n\r");
		}
		else
		{        
			LED_RED;
			printf("\r\n 8M����flash(W25Q64)����ʧ��!\n\r");
		}
	}// if (FlashID == sFLASH_ID)
	else// if (FlashID == sFLASH_ID)
	{ 
		LED_RED;
		printf("\r\n ��ȡ���� W25Q64 ID!\n\r");
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
 * ��������Buffercmp
 * ����  ���Ƚ������������е������Ƿ����
 * ����  ��-pBuffer1     src������ָ��
 *         -pBuffer2     dst������ָ��
 *         -BufferLength ����������
 * ���  ����
 * ����  ��-PASSED pBuffer1 ����   pBuffer2
 *         -FAILED pBuffer1 ��ͬ�� pBuffer2
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
�������ƣ�hex2str
�������ܣ���ʮ��������ת��Ϊ�ַ���
���������
    hexdata ��ʾ�����ʮ��������
    s ��ʾ�ַ�ָ��ָ��洢�Ľ���ַ��� 
    length ��ʾ�ַ�����
************************************************************************/
static void hex2str(char hexdata, char* s, char length)
{
    int k;
    s[length] = 0;
    /* һ���ַ�4Bit
    hexdata >>= 4  ÿ��������һ��ʮ������ת�ַ�֮������4λ������ */
    for (k = length - 1; k >= 0; k--, hexdata >>= 4)
    {
				if ((hexdata & 0xF) <= 9)//��ʮ�������е����ݽ��д��� 
           {
                s[k] = (hexdata & 0xF) + '0';//���ֱ���ַ�
           }
        else
            {
                s[k] = (hexdata & 0xF) + 'A' - 0x0A;//��ĸ����ַ�
            }
    }
}
//hexУ��ת2ascii
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

//����������
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
			checkValue = check(r.rxbuff);//У��
			checkValue1 = (u16)(r.rxbuff[18] << 8 | r.rxbuff[19]);
			
			if((checkValue == checkValue1) && (r.rxbuff[20] == '\r') && (r.rxbuff[21] == '\n'))
			{
				r.success = 1;
				r.mode = 0;
				printf("\r\n");
				memcpy(k3 , &r.rxbuff[10] , 4);
				sscanf(k3 ," %lf" , &fff);//Ũ��ֵ
				memcpy(k4 , &r.rxbuff[2] , 6);
				sscanf(k4 ," %lf" , &fff1);//�¶�ֵ
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
