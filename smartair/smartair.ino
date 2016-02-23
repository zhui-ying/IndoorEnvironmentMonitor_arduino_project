
// include the library code:
#include <LiquidCrystal.h>
#include <dht11.h>
#include <MsTimer2.h>               //定时器库的 头文件

//报警阈值定义
#define T_WARNING_VALUE 25
#define H_WARNING_VALUE 75
#define P_WARNING_VALUE 200//1050 
#define C_WARNING_VALUE 700

#define MQ5_REF 0.15 //MQ5初始电压基准

#define DHT11PIN 12
#define LEDPOWER_PIN 13
#define DUST_PIN 1//PM2.5模拟输入接口
#define CON_PIN 0//MQ5模拟接口

#define RELAYPIN A2
#define BEEPPIN A3

boolean flag10ms=true;
boolean dispflag = false;
boolean warningflag = false;
boolean beepflag = true;
int count_1s=0;
int count_2s=0;
int count_500ms=0;
dht11 DHT11;
int dustVals[10];//record dust sample AD value
int dustValAverages[10];
int dustVal;
char dust_count=0;//sample count
char dustAverage_count=0;
int concentration=0;//煤气浓度

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(2, 3, 4, 5, 6, 7,8,9,10,11);

//隔一段时间读取一次PM平均值，并记录在另外一组数组中，求加权平均值
void getDustValue()
{
	int dustValAD,index,ADmax;
	dustValAD = 0;
	dust_count = 0;
	ADmax = dustVals[0];
	//find max value
	for(index = 1;index<10;index++)
	{
		if(ADmax < dustVals[index])
		{
			ADmax = dustVals[index];
		}
	}
	for(index = 0;index<10;index++)
	{
		dustValAD += dustVals[index];
	}
	dustValAD = dustValAD - ADmax;//去除最大值
	dustValAverages[dustAverage_count++] = dustValAD/9;//get average value	
	if(dustAverage_count == 5) dustAverage_count = 0;
	
	//计算5次加权平均数
	dustValAD = dustValAverages[dustAverage_count]*1 + dustValAverages[(dustAverage_count+1)%5]*2
	+ dustValAverages[(dustAverage_count+2)%5]*3 + dustValAverages[(dustAverage_count+3)%5]*4
	+ dustValAverages[(dustAverage_count+4)%5]*5;
	
	dustValAD = dustValAD/15;
	
	double dustVolt = (double)dustValAD/1024.0 * 5.0;
	if(dustVolt > 0.412)//避免出现负数
	{
		dustVal = (0.17 * dustVolt - 0.07)*1000.0; //ug/m3
	}else
	{
		dustVal = 0;
	}	
}

void setup() {
	MsTimer2::set(10, timer10msint);        // 中断设置函数，每 500ms 进入一次中断
	MsTimer2::start();                //开始计时
	// set up the LCD's number of columns and rows: 
	lcd.begin(16, 2);
	Serial.begin(9600);
	// Print a message to the LCD.
	lcd.print("H:      T:");//湿度 温度
	lcd.setCursor(0, 1);
	lcd.print("P:      C:");//PM2.5  煤气浓度
	
	pinMode(LEDPOWER_PIN,OUTPUT);
	pinMode(RELAYPIN,OUTPUT);
	pinMode(BEEPPIN,OUTPUT);
	
	pinMode(DUST_PIN, INPUT);
	pinMode(CON_PIN, INPUT);
	
	digitalWrite(RELAYPIN,LOW);
	digitalWrite(BEEPPIN,LOW);
	digitalWrite(LEDPOWER_PIN,LOW); 
}

/* 测试得到的数据和空气质量对照：
3000 + = 很差
1050-3000 = 差
300-1050 = 一般
150-300 = 好
75-150 = 很好
0-75 = 非常好 */
void loop() {
	
	if(Serial.available() > 0)
	{
		int incomingByte = Serial.read();
		if(incomingByte == 'N')
		{
			beepflag = false;
		}else
		{
			beepflag = true;
		}
	}
	
	//每隔10ms获取一次PM2.5 AD值
	if(flag10ms == true)
	{
		flag10ms = false;
		count_1s++;
		count_2s++;
		count_500ms++;
		
		digitalWrite(LEDPOWER_PIN,HIGH); 
		delayMicroseconds(280);//delay 280us
		dustVals[dust_count]=analogRead(DUST_PIN); //read AD
		dustVals[dust_count]+=analogRead(DUST_PIN);
		dustVals[dust_count]+=analogRead(DUST_PIN);
		dustVals[dust_count] = dustVals[dust_count]/3;//读三次取平均值
		dust_count++;
		delayMicroseconds(20);//delay 40us
		digitalWrite(LEDPOWER_PIN,LOW); 
		
		if(dust_count == 10)
		{
			dust_count = 0;
		}

		if(count_1s == 100)//1s  read MQ5 AD value  and send msg to PC 
		{
			int concentration_temp;
			count_1s = 0;
			//每隔1s获取一次MQ5的值 ppm
			concentration_temp = analogRead(CON_PIN);
			double concentVolt = (double)concentration_temp/1024.0 * 5.0;
			if(concentVolt >MQ5_REF)
			{
				concentration = (concentVolt - MQ5_REF) * 200;
			}else
			{
				concentration = 0;
			}
			
			
		}
		if(count_2s == 200)//2s read humidity and temperature
		{
			count_2s = 0;
			DHT11.read(DHT11PIN);
		}
		
		if(count_500ms == 50)//500ms update display
		{
			//每隔500ms计算一次PM2.5的值 ug/m3
			getDustValue();
			
			count_500ms = 0;
			
			//send to PC 
			Serial.print("value=");
			Serial.print(DHT11.humidity);
			Serial.print(",");
			Serial.print(DHT11.temperature);
			Serial.print(",");
			Serial.print(dustVal);
			Serial.print(",");
			Serial.print(concentration);	
			Serial.print("\r\n");
			
			lcd.setCursor(2, 0);
			lcd.print("     ");
			lcd.setCursor(2, 0);
			lcd.print(DHT11.humidity);//湿度
			
			lcd.setCursor(10, 0);
			lcd.print("     ");
			lcd.setCursor(10, 0);
			lcd.print(DHT11.temperature);//温度

			lcd.setCursor(2, 1);
			lcd.print("     ");
			lcd.setCursor(2, 1);
			lcd.print(dustVal);//PM2.5
			
			lcd.setCursor(10, 1);
			lcd.print("     ");
			lcd.setCursor(10, 1);
			lcd.print(concentration);//煤气浓度			

			if((DHT11.humidity > H_WARNING_VALUE) || (DHT11.temperature > T_WARNING_VALUE) 
					|| (dustVal > P_WARNING_VALUE) || (concentration > C_WARNING_VALUE))
			{
				warningflag = true;
				digitalWrite(RELAYPIN,HIGH);
			}else
			{
				warningflag = false;
				digitalWrite(RELAYPIN,LOW);
				digitalWrite(BEEPPIN,LOW);
			}
			
			if(warningflag == true)//蜂鸣器响起，被报警的数据闪动
			{
				if(dispflag == true)
				{
					dispflag = false;
					if(beepflag == true)//禁止蜂鸣器乱叫，太吵了
					{
						  digitalWrite(BEEPPIN,HIGH);
					}
					if(DHT11.humidity > H_WARNING_VALUE)
					{
						lcd.setCursor(2, 0);
						lcd.print("     ");//湿度
					}
					if(DHT11.temperature > T_WARNING_VALUE)
					{
						lcd.setCursor(10, 0);
						lcd.print("     ");//温度						
					}
					if(dustVal > P_WARNING_VALUE)
					{
						lcd.setCursor(2, 1);
						lcd.print("     ");//PM2.5
					}
					if(concentration > C_WARNING_VALUE)
					{
						lcd.setCursor(10, 1);
						lcd.print("     ");//煤气浓度				
					}
				}else
				{
					dispflag = true;
					digitalWrite(BEEPPIN,LOW);
					
				}		
			}

		}
	}
}

void timer10msint()
{
	flag10ms = true;
}
















