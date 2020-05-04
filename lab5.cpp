#include <dos.h>
#include <stdio.h>
#include <conio.h>
#include <io.h>

//глобальный счётчик милисекунд
int miliSeconds = 0;
//указатель на старый обработчик прерываний ЧРВ
void interrupt far(*oldInt70h)(...);
//указатель на старый обработчик прерываний будильника
void interrupt far(*oldInt4Ah)(...);
//новый обработчик будильника
void interrupt newInt4Ahandler(...);
//новый обработчик ЧРВ
void interrupt newInt70handler(...);

void freeze(void);
void unfreeze(void);

int BCDToInteger(int bcd);
unsigned char IntToBCD(int value);
void getTime(void);
void setTime(void);
void delay_time(void);
void wait(void);
void enableAlarm(void);
void disableAlarm(void);



void wait(void)
{
	do
	{
		outp(0x70, 0x0A);		//регистр состояния 1(UIP - момент обновления показаний часов 0 -часы доступны)
		//DV - 3 бита задают значение частоты обновления часов(для правильной работы должны быть уст в 010 - 32.768Кгц)
		//RS - 4 бита возволяют выбрать делитель выходной частоты(для правильной работы должны быть уст в 0110 - 1.024)
	} while (inp(0x71) & 0x80);	//ожидаем пока часы станут доступными		
}

void getTime(void)
{
	int value;
	wait();									//ожидаем часы
	outp(0x70, 0x04);						//устанавливаем значение порта
	value = inp(0x71);						//получаем значение часов	
	printf("%d:", BCDToInteger(value));		//переводим двоично-десятичный вид в в представление int

	wait();
	outp(0x70, 0x02);
	value = inp(0x71);						//получаем значение минут
	printf("%d:", BCDToInteger(value));

	wait();
	outp(0x70, 0x00);
	value = inp(0x71);						//получаем значение секунды
	printf("%d   ", BCDToInteger(value));

	wait();
	outp(0x70, 0x07);
	value = inp(0x71);						//получаем значение даты
	printf("%d.", BCDToInteger(value));

	wait();
	outp(0x70, 0x08);
	value = inp(0x71);						//получаем значение месяца
	printf("%d.", BCDToInteger(value));

	wait();
	outp(0x70, 0x09);
	value = inp(0x71);						//получаем значение года
	printf("%d   ", BCDToInteger(value));

	wait();
	outp(0x70, 0x06);
	value = inp(0x71);						//получаем значение дня недели
	switch (BCDToInteger(value))
	{
	case 1: printf("Sunday\n"); break;
	case 2: printf("Monday\n"); break;
	case 3: printf("Tuesday\n"); break;
	case 4: printf("Wednesday\n"); break;
	case 5: printf("Thursday\n"); break;
	case 6: printf("Friday\n"); break;
	case 7: printf("Saturday\n"); break;
	default: printf("Day of week is not set\n"); break;
	}
}

void setTime(void)
{
	int value;
	freeze();
	//получение данных
	do
	{
		printf("Enter hour: ");
		fflush(stdin);
		scanf("%d", &value);
		if (value > 23 || value < 0)
			printf("Error incorrect value!\n");
	} while (value > 23 || value < 0);
	outp(0x70, 0x04);
	outp(0x71, IntToBCD(value));				//установка значений часов

	do
	{
		printf("Enter minute: ");
		fflush(stdin);
		scanf("%d", &value);
		if (value > 59 || value < 0)
			printf("Error incorrect value!\n");
	} while (value > 59 || value < 0);
	outp(0x70, 0x02);
	outp(0x71, IntToBCD(value));				//установка значений минут

	do
	{
		printf("Enter second: ");
		fflush(stdin);
		scanf("%d", &value);
		if (value > 59 || value < 0)
			printf("Error incorrect value!\n");
	} while (value > 59 || value < 0);
	outp(0x70, 0x00);
	outp(0x71, IntToBCD(value));				//установка значений секунд

	do
	{
		printf("Enter week day number(1...7): ");
		fflush(stdin);
		scanf("%d", &value);
		if (value > 7 || value < 1)
			printf("Error incorrect value!\n");
	} while (value > 7 || value < 1);
	outp(0x70, 0x06);
	outp(0x71, IntToBCD(value));				//6-отвечает за дни недели

	do
	{
		printf("Enter day of month(1...31): ");
		fflush(stdin);
		scanf("%d", &value);
		if (value > 31 || value < 1)
			printf("Error incorrect value!\n");
	} while (value > 31 || value < 1);
	outp(0x70, 0x07);
	outp(0x71, IntToBCD(value));				//7-дата

	do
	{
		printf("Enter mounth(1...12): ");
		fflush(stdin);
		scanf("%d", &value);
		if (value > 21 || value < 1)
			printf("Error incorrect value!\n");
	} while (value > 12 || value < 1);
	outp(0x70, 0x08);
	outp(0x71, IntToBCD(value));				//8-месяц

	do
	{
		printf("Enter year(0...20): ");
		fflush(stdin);
		scanf("%d", &value);
		if (value > 20 || value < 0)
			printf("Error incorrect value!\n");
	} while (value > 20 || value < 0);
	outp(0x70, 0x09);
	outp(0x71, IntToBCD(value));				//9-год

	unfreeze();
}

void enableAlarm(void)
{

	int value, second, minute, hour;
	//ввод данных для установки будильника
	do
	{
		printf("Enter hour: ");
		fflush(stdin);
		scanf("%d", &hour);
	} while (hour > 23 || hour < 0);

	do
	{
		printf("Enter minute: ");
		fflush(stdin);
		scanf("%d", &minute);
	} while (minute > 59 || minute < 0);

	do
	{
		printf("Enter second: ");
		fflush(stdin);
		scanf("%d", &second);
	} while (second > 59 || second < 0);

	//для записи времени 70 порт, для чтения и/или записи 71
	outp(0x70, 0x01);
	outp(0x71, IntToBCD(second));	//установка секунд для будильника

	outp(0x70, 0x03);
	outp(0x71, IntToBCD(minute));	//минуты для будильника

	outp(0x70, 0x05);
	outp(0x71, IntToBCD(hour));		//часы для будильника

	outp(0x70, 0xB);				//регистр состояния 2
	outp(0x71, inp(0x71) | 0x20);	//0001 0100 - 5-ый бит AIE стоит в 1(прерывания будильника разрешены), 2-ой бит в 1(DM-формат времени и даты в двоичном виде)

	//установка нового обработчика прерываний
	disable();

	oldInt4Ah = getvect(0x4A);		//запоминаем старый
	setvect(0x4A, newInt4Ahandler);	//устанавливаем новый

	enable();
}

//восстановление старого значения прерывания после срабатывания будильника
void disableAlarm()
{
	if (oldInt4Ah == NULL) {
		printf("Alarm is already disable or not set.\n");
		return;
	}
	_disable();

	setvect(0x4A, oldInt4Ah);			//устанавливаем старое прерывание будильника
	outp(0xA1, (inp(0xA0) | 0x01));
	outp(0x70, 0x0B);
	outp(0xA1, (inp(0x71) & 0xDF));		//11011111 для запрета прерываний будильника, конъюнкция даст 0 бит - в 1 и 5 бит - в 0

	_enable();

	printf("Alarm is disable.\n");
}

void freeze(void)
{
	unsigned char value;

	wait();											//ожидания часов

	outp(0x70, 0x0B);
	value = inp(0x71);								//заносим состояние регистра 0х0В
	value |= 0x80;									//запрещаем обновления часов,с помощи установки UPD 7-го бита на 1
	outp(0x70, 0x0B);
	outp(0x71, value);								//заносим новое значение в регистр с установленным битом запрета		
}

void unfreeze(void)
{
	unsigned char value;

	wait();											//ожидание часов							

	outp(0x70, 0x0B);								//регистр состояния 2
	value = inp(0x71);								//записываем состояние
	value -= 0x80;									//разрешаем обновление часов, т.е. UPD 7-ой бит меняется на ноль
	outp(0x70, 0x0B);
	outp(0x71, value);								//записываем новое значение регистра 0х0В,обновление часов разрешено
}

//новый обработчик прерывания задержки ЧРВ
void interrupt newInt70handler(...)
{
	miliSeconds++;									//глобальный счётчик милисекунд															
	outp(0x70, 0x0C);								//регистр состояния 3(текущий стату прерывания часов)					
	inp(0x71);
	outp(0x20, 0x20);								//посылаем контроллеру прерываний сигнал EOI(MASTER)
	outp(0xA0, 0x20);								//посылаем контроллеру прерываний сигнал EOI(SLAVE)
}

//получает управление при срабатывании будильника
void interrupt newInt4Ahandler(...)
{
	fflush(stdout);
	write(1, "Wake UP!\0", 8);
	//восстанавливаем старое значение
	disableAlarm();
}

//задержка на N мс
void delay_time(void)
{
	unsigned long delayPeriod;
	unsigned char value;

	do
	{
		printf("Enter delay time in milliseconds: ");
		fflush(stdin);
		scanf("%ld", &delayPeriod);
	} while (delayPeriod < 1);
	printf("Time before delay: ");
	getTime();											//получить текущее время
	printf("Delaying ...\n");

	_disable();											//запрещаем прерывания						
	oldInt70h = getvect(0x70);							//запоминаем старый обработчик прерывания
	setvect(0x70, newInt70handler);						//устанавливаем свой обработчик прерывания
	_enable();											//разрешаем прерывания


	outp(0xA1, inp(0xA1) & 0xFE);						//размасирование линий сигнала запроса от ЧРВ 11111110, в перовм бите 0-разрешить прерывания							
	outp(0x70, 0x0B);									//регистр состояния 2	
	outp(0x71, inp(0x71) | 0x40);						//01000000 устанавливаем в 6-ой бит 1-PIE генерация переодических прерываний			

	miliSeconds = 0;
	while (miliSeconds != delayPeriod);					//цикл задержки	 
	printf("End delay of %d ms\n", delayPeriod);

	_disable();
	setvect(0x70, oldInt70h);							//обратный процесс восстановления старого прерывания 		
	_enable();

	printf("Time after delay: ");
	getTime();											//выводим текущее время, дла сравнения время до задержки и посли неё
}

//перевод BCD-кода в интовое значение
int BCDToInteger(int bcd)
{
	return bcd % 16 + bcd / 16 * 10;
}

unsigned char IntToBCD(int value)
{
	return (unsigned char)((value / 10) << 4) | (value % 10);
}

void main()
{
	char c, value;
	clrscr();
	printf("\n1. Show time\n2. Set time\n3. Delay time\n4. Enable alarm\n5. Disable alarm\n0. Exit\n\n");
	while (c != 0)
	{
		c = getch();
		switch (c)
		{
		case '1': getTime(); break;
		case '2': setTime(); break;
		case '3': delay_time(); break;
		case '4': enableAlarm(); break;
		case '5': disableAlarm(); break;
		case '0': return;
		}
		printf("\n1. Show time\n2. Set time\n3. Delay time\n4. Enable alarm\n5. Disable alarm\n0. Exit\n\n");
	}
}
