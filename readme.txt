室内环境监测系统使用说明

一、smartair
功能：传感器集成模组，实时获取温度，湿度，PM2.5，可燃气体浓度等值显示到液晶屏上，并实现报警功能和风扇控制功能
使用方法：
1、按照 IO接口表.xlsx 接好所有的线到arduino模块
2、传感器集成板上建议使用移动电源供电
3、arduino模块可直接由PC供电
4、先上电传感器集成板电源，在打开arduino模块电源
5、可采用加温，吹气的方式模拟温湿度，PM2.5的变化，可以看到液晶屏上值有变化
开发环境：arduino C开发

二、smartairPC 
功能：上位机软件，通过串口读取来自传感器的数据并显示到屏幕上，实现PC端和传感器端数据同步
使用方法：
1、插上来自arduino板的USB线到电脑USB口
2、打开 temperatureTest.exe 选择波特率 9600 串口号为电脑设备管理器显示的串口号，点击 open
3、PC上即可显示和开发板的LCD1602上相同的数据
开发环境：VS2010 C#开发

三、开发过程工程
在测试各个模块时遗留的工程，仅起参考作用