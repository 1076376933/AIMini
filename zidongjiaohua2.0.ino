/*本程序由忻州师范学院电子系2002班王宇乐所写，联系方式QQ：1076376933，如需转载请说明
 忻州师范学院大学生创新创业训练计划项目
 项目名称：AIMini智能土壤湿度管家
 所使用编程程序Arduino
 库文件点灯科技Blinker
 使用单片机ESP8266-12F
 本代码基于Arduino开发，使用Blinker库，实现智能配网、数据总结、数据调试以及对设备的控制
 本产品需要外围电路，详情请移步本产品电路原理图以及pcb文档
 目前仅做学习使用，请勿商用，谢谢合作！！
*/
#define BLINKER_WIFI                 //定义wifi模块
#include <Blinker.h>                 //包含Blinker头文件
int count=0;                         //定义wifi通断计数函数  
bool WIFI_Status = true;             //定义wifi连接判断布尔函数
int soilPin = A0;                    //设置模拟口A0为信号输入端
int humidityValue = 0;               //存放土壤模拟信号的变量
float humidity = 0;                  //浮点类型
int waterPumpPin = D1;               //继电器 水泵控制引脚IO5
int sliderValue1 = 40;               //湿度默认最高值
int sliderValue2 = 20;               //湿度默认最低值
int sliderValue3 = 10;               //默认定时浇花时长/h
int sliderValue4 = 5;                //默认浇水持续时长/s
int time1=0;                         //时间1，重置定时时间/h
int time2;                           //定时等待时间2
bool manualFlag = false;             //布尔型变量1
bool manualFlag1 = false;            //布尔型变量2
bool manualFlag2 = false;            //布尔型变量3
bool manualFlag3 = false;            //布尔型变量4

BlinkerNumber NumSoil("humi");       //土壤湿度的值
BlinkerButton BtnManual("jidianqi"); //手动浇花按键
BlinkerButton Button1("zidong");     //手动浇花按键
BlinkerButton Button2("dingshi");    //定时浇花按键
BlinkerSlider Slider1("ran-a");      //浇花土壤最高湿度
BlinkerSlider Slider2("ran-b");      //浇花土壤最低湿度
BlinkerSlider Slider3("ran-c");      //定时浇水的时间/h
BlinkerSlider Slider4("ran-d");      //浇水持续时常/s
BlinkerText Tex1("wenben");          //文本1
BlinkerText Tex2("wenben1");         //文本2
BlinkerText Tex3("wenben2");         //文本3
//****************网络信息******************************
//请更改您设备的专属密钥
char auth[] = "xxxxxxx";    //设备密钥
//****************网络配置Smartconfig*******************
void smartConfig()//配网函数
{
  WiFi.mode(WIFI_STA);//使用wifi的STA模式
  Serial.println("\r\nWait for Smartconfig...");//串口打印
  WiFi.beginSmartConfig();//等待手机端发出的名称与密码
  //死循环，等待获取到wifi名称和密码
  while (1)
  {
    //等待过程中一秒打印一个.
    Serial.print(".");
    delay(1000);
    //指示灯持续闪烁
    digitalWrite(LED_BUILTIN, LOW);
    delay(1000);
    digitalWrite(LED_BUILTIN, HIGH);
    if (WiFi.smartConfigDone())//获取到之后退出等待
    {
      Serial.println("SmartConfig Success");
      //打印获取到的wifi名称和密码
      Serial.printf("SSID:%s\r\n", WiFi.SSID().c_str());
      Serial.printf("PSW:%s\r\n", WiFi.psk().c_str());
      break;
    }
  }
}
void WIFI_Init()
{
    Serial.println("\r\n正在连接");
    //当设备没有联网的情况下，执行下面的操作
    while(WiFi.status()!=WL_CONNECTED)
    {
        if(WIFI_Status)//WIFI_Status为真,尝试使用flash里面的信息去 连接路由器
        {
            Serial.print(".");
            delay(1000);                                        
            count++;
            if(count>=5)
            {
                WIFI_Status = false;
                Serial.println("WiFi连接失败，请用手机进行配网"); 
            }
        }
        else//使用flash中的信息去连接wifi失败，执行
        {
            smartConfig();  //smartConfig技术配网
        }
     }  
     //串口打印连接成功的IP地址
     Serial.println("连接成功");  
     Serial.print("IP:");
     Serial.println(WiFi.localIP());
     digitalWrite(LED_BUILTIN, LOW); //闪光灯闪烁
     delay(500);
     digitalWrite(LED_BUILTIN, HIGH);
     delay(200);
     digitalWrite(LED_BUILTIN, LOW);
     delay(200);
     digitalWrite(LED_BUILTIN, HIGH);
     
}
//****************滑块设置*******************
void slider1_callback(int32_t value)
{
  sliderValue1 = value;
  BLINKER_LOG("get slider value: ", value);
}
void slider2_callback(int32_t value)
{
  sliderValue2 = value;
  BLINKER_LOG("get slider value: ", value);
}
void slider3_callback(int32_t value)
{
  sliderValue3 = value;
  BLINKER_LOG("get slider value: ", value);
}
void slider4_callback(int32_t value)
{
  sliderValue4 = value;
  BLINKER_LOG("get slider value: ", value);
}
//***************心跳包回调函数****************
void heartbeat() 
{
//*****由继电器变量查看工作状态*****
  if (digitalRead(waterPumpPin) == HIGH)
    Tex1.print("工作状态","待机中…");
  else if (digitalRead(waterPumpPin) == LOW)
    Tex1.print("工作状态","工作中！");
//*****向APP发送数据*****
  Slider1.print(sliderValue1);
  Slider2.print(sliderValue2);
  Slider3.print(sliderValue3);
  Slider4.print(sliderValue4);
  NumSoil.print(humidity);
}
//***************按钮设置*********************
void BtnManual_callback(const String &state) //手动浇花按键回调函数
{
  BLINKER_LOG("get button state: ", state);
  if (state == "on")                         //开关打开时
  {
    manualFlag = true;                       //布尔ture
    digitalWrite(waterPumpPin, LOW);         //打开继电器，LOW
    Tex1.print("工作状态","工作中！");
    BtnManual.print("on");
  }
  if (state == "off")                        //开关关闭时
  {
    manualFlag = false;                      //布尔false
    digitalWrite(waterPumpPin, HIGH);        //关掉继电器，HIGH
    Tex1.print("工作状态","待机中…");
    BtnManual.print("off");
  }
}

void Button1_callback(const String &state1)  //自动浇花按键回调函数
{
  BLINKER_LOG("get button state: ", state1);
  if (state1 == "on")                        //开关打开时
  {
    manualFlag1 = true;                      //布尔2ture
    manualFlag2 = false;                     //布尔3false
    Tex2.print("自动浇水工作中");
    Tex3.print("定时浇水已关闭");
    Button1.print("on");
    Button2.print("off");                    //关闭定时浇水
  }
  if (state1 == "off")                       //开关关闭时
  {
    manualFlag1 = false;                     //布尔2false
    Tex2.print("自动浇水已关闭");
    Button1.print("off");
  }
}

void Button2_callback(const String &state2)  //定时浇花按键回调函数
{
  BLINKER_LOG("get button state: ", state2);
  
  if (state2 == "on")                        //开关打开时
  {
    manualFlag2 = true;                      //布尔3ture
    manualFlag1 = false;                     //布尔2false
    Tex3.print("定时浇水工作中");
    Tex2.print("自动浇水已关闭");
    Button2.print("on");
    Button1.print("off");                    //关闭自动浇水
  }
  if (state2 == "off")                       //开关关闭时
  {
    manualFlag2 = false;                     //布尔3false
    manualFlag3=false; 
    Tex3.print("定时浇水已关闭");
    Button2.print("off");
  }
}
//***************data设置 设置************************
void dataStorage()
{
  Blinker.dataStorage("humi",humidity);
}
//***************设备运行初始化设置*********************
void setup()
{
    //初始化端口
    Serial.begin(115200);
    //BLINKER_DEBUG.stream(Serial);        //调试代码
    //BLINKER_DEBUG.debugAll();            //调试代码
    pinMode(LED_BUILTIN, OUTPUT);          // 初始化有LED的IO
    digitalWrite(LED_BUILTIN, HIGH);       //关闭指示灯
    pinMode(waterPumpPin, OUTPUT);         //设置继电器变量为输出模式
    digitalWrite(waterPumpPin, HIGH);      //关掉继电器，HIGH
    WIFI_Init();                           //调用WIFI函数
    Blinker.begin(auth, WiFi.SSID().c_str(), WiFi.psk().c_str());
    Blinker.attachHeartbeat(heartbeat);    //心跳包初始化
    BtnManual.attach(BtnManual_callback);  //初始化按键1
    Button1.attach(Button1_callback);      //初始化按键2
    Button2.attach(Button2_callback);      //初始化按键3
    Slider1.attach(slider1_callback);      //初始化滑块1
    Slider2.attach(slider2_callback);      //初始化滑块2
    Slider3.attach(slider3_callback);      //初始化滑块3
    Slider4.attach(slider4_callback);      //初始化滑块4
    Blinker.attachDataStorage(dataStorage);//初始化data
    Blinker.setTimezone(-8.0);             //获取当前时间（北京）
}
//********************循环语句设置*************************
void loop()
{
  Blinker.run();                                          //blinker程序运行代码
  humidityValue = analogRead(soilPin);                    //传感器信号赋值给模拟信号
  humidity = map(humidityValue,0,670,100,0);              //浮点类型变量的转化
  Serial.println(manualFlag1);                            //串口显示内容
  int8_t hour1 = Blinker.hour();                          //获取小时
 // int8_t min1 = Blinker.minute();                       //获取分钟
 // int8_t sec1 = Blinker.second();                       //获取秒
  if(manualFlag != true){
  if ((manualFlag1 != false)&&(manualFlag2 != true))      //自动控制标志位ture的时候进行
  {    
    manualFlag3=false;
    if (humidity < sliderValue2)                          //认为水少 需要浇水
      digitalWrite(waterPumpPin, LOW);                    //开启继电器
    else if(humidity > sliderValue1)                      //认为水多 不需要浇水
      digitalWrite(waterPumpPin, HIGH);                   //关掉继电器
  }

  
  if ((manualFlag1 != true)&&(manualFlag2 != false))
  {
    if((hour1 == sliderValue3)&&(manualFlag3 == false)){
       time2=sliderValue4*1000;
       digitalWrite(waterPumpPin, LOW);                   //开启继电器
       Blinker.delay(time2);                              //等待时间
       digitalWrite(waterPumpPin, HIGH);                  //off继电器
       manualFlag3=true;
    }
  }
    
  if ((manualFlag1 != true)&&(manualFlag2 != true))
  {
    digitalWrite(waterPumpPin, HIGH);                     //关掉继电器，HIGH
    manualFlag3=false; 
  }
  }
  if(hour1 == time1){                                     //每日零点重置定时按钮
    manualFlag3=false;
  }
  Blinker.delay(1000);
}
