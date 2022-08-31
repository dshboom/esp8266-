#include <NTPClient.h>
//导入NTP库，实现NTP时钟
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ESP8266_Heweather.h>
#include <ArduinoJson.h>
//导入实现esp8266联网并实现天气状况获取的库
#include <TFT_eSPI.h>
#include <SPI.h>
//导入驱动TFT屏的库，本项目中使用的是128x160规格的TFT屏，利用SPI总线连接。
#include "esp8266_zm.h"
#include "bmp.h"
//导入所需的字库文件

unsigned int colour = 0;
const char *ssid     = "TP-LINK_6E29";
const char *password = "dfy6768507";
String UserKey = "99326fe894c5413aa85427a668190885";   
//设置知心天气私钥
String Location = "101280601"; 
//知心天气城市代码
String Unit = "m";             
//知心天气返回值公制-m/英制-i
String Lang = "zh";            
//知心天气返回值语言英文-en/中文-zh           
int Weather_Information_GetTime = 50;
//设置天气状况获取间隔，配合Time_Getime变量使用
int Weather_Information_GetTime_Setup = Weather_Information_GetTime + 1;
//该变量用于使程序在第一次运行时获取天气状况
int Time_Getime = 30000;
//获取NTP时间的间隔(ms)
String weatherNow_API;
String GetTimeUpdate;
String GetTemp;
String FeelTemp;
String WeatherNowPicture;
String WeatherText;
String WindDir;
String WindScale;
String GetHumidity;
String Precip;
String AirServerCode;
String Aqi;
String AirCategory;
String AirPrimary;
String WeatherServerCode;
String itime;
String itime1;
String itime2;
String itime3;
String itime4;
String Humidity;
String datefirst1;
String datefirst2;
unsigned long time_now = 0;
//声明一些变量



WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP,"ntp1.aliyun.com",60*60*8,30*60*1000);
TFT_eSPI tft = TFT_eSPI();
WeatherNow weatherNow;
AirQuality AirQuality;
//建立一些对象

void setup(){
  Serial.begin(115200);
  //启动串口
  ConnectWiFi(); 
  //连接WiFi
  AirQuality.config(UserKey, Location, Unit, Lang); 
  weatherNow.config(UserKey, Location, Unit, Lang); 
  //配置知心天气请求信息
  tft.init();
  tft.begin();
  //屏幕初始化
  tft.setRotation(2);
  //设置屏幕角度为90度，即竖屏模式
  timeClient.begin();
  //开始获取时间
}



void loop() {
    if(Weather_Information_GetTime_Setup > Weather_Information_GetTime){
    //设置天气状况获取间隔，配合Time_Getime变量使用,单位毫秒,如当前间隔为50*30000毫秒,即25分钟
    Weather_Information_GetTime_Setup = 0;
    //重置天气状况获取间隔，该写法是为了让程序启动时执行一次天气状况获取
    getAirQuality();
    //获取空气质量数据
    getweatherNow();
    //获取天气状况数据
    } 
    else{
    tft.fillScreen(TFT_BLACK);
    //刷新屏幕并设置背景颜色为黑色
    tft.loadFont(font_16);
    //导入font_16字库具体见esp8266_zm.h
    tft.setTextColor(TFT_WHITE);
    //设置字体颜色为白色
    tft.drawString(GetMonth(),5,66);
    //获取日期中的月
    tft.drawString(GetDate(),36,66);
    //获取日期中的日
    tft.drawString("月",20,68);
    tft.drawString("日",51,68);
    tft.drawString("深圳",10,6);
    tft.drawString(WeatherText,80,6);
    tft.drawString(GetTemp,4,87);
    tft.drawString("℃/T",22,87);
    //显示信息
    tft.drawString(NewHumidity(),4,103);
    //对相对湿度原始数据进行处理，得到两位数整数并输出
    tft.drawString("%/RH",22,103);
    //显示信息
    if(WindDir == "无持续风向"){
      tft.drawString("无风向",5,122);
      tft.drawString(WindScale,59,122);
      tft.drawString("级",69,122);
    }else {
      tft.drawString(WindDir,5,122);
      tft.drawString(WindScale,59,122);
      tft.drawString("级",69,122);
    }
    //判断是否有风向，以免原始数据输出导致显示错误
    tft.unloadFont();
    //卸载font_16字库
    tft.loadFont(font_17);
    //载入font_17字库
    tft.fillRect(0, 144, 128, 26,TFT_WHITE);
    //在屏幕下方显示白条
    tft.setTextColor(TFT_BLACK);
    //设置字体为黑色
    tft.drawString("AQI:",5,146);
    tft.drawString(Aqi,33,146);
      if(AirCategory == "NA"){
    tft.drawString("优",85,146);
    }else{
      tft.drawString(AirCategory,85,146);
    }
    //此处判断返回值是否为NA，若为NA即空气质量为优
    if(AirPrimary != "NA"){
      tft.drawString(AirPrimary,90,146);
    }
    //此处判断是否有空气污染物，若有则显示，没有则不显示
    tft.unloadFont();
    //卸载字库
    tft.setTextColor(0xFBE0);
    //自定义字体颜色
    timeClient.update();
    itime = timeClient.getFormattedTime();
    //开始获取时间原始数据，并保存在变量itime中
    Serial.println(itime); 
    //发送时间原始数据到串口
    tft.drawRightString(itimecharge("hour"),127,25,6);
    tft.setTextColor(TFT_WHITE);
    tft.drawRightString(itimecharge("minutes"),157,25,6);
    //显示小时和分钟
    tft.setTextColor(tft.color565(204, 204, 10));
    //自定义屏幕颜色
    tft.drawRightString(":",68,25,6);

    delay(Time_Getime);
    Weather_Information_GetTime_Setup++;
      }
}
void getweatherNow(){
  if(weatherNow.get()){ 
    Serial.println("======当前天气======");
    //获取天气更新
    weatherNow_API = String(weatherNow.getServerCode());
    Serial.print("API状态码: ");
    Serial.println(weatherNow_API);  
    //获取API状态码
    GetTimeUpdate = String(weatherNow.getLastUpdate());
    Serial.print("更新天气信息时间: ");
    Serial.println(GetTimeUpdate);  
    //获取服务器更新天气信息时间
    GetTemp = String(weatherNow.getTemp());
    Serial.print("温度: ");
    Serial.println(GetTemp);       
    //获取实况温度
    FeelTemp = String(weatherNow.getFeelLike());
    Serial.print("体感温度: ");
    Serial.println(FeelTemp);    
    //获取实况体感温度
    WeatherNowPicture = String(weatherNow.getIcon());
    Serial.print("天气图标代码: ");
    Serial.println(WeatherNowPicture);        
    //获取当前天气图标代码
    WeatherText = String(weatherNow.getWeatherText());
    Serial.print("天气状况: ");
    Serial.println(WeatherText); 
    //获取实况天气状况的文字描述
    WindDir = String(weatherNow.getWindDir());
    Serial.print("风向: ");
    Serial.println(WindDir);     
    //获取风向
    WindScale = String(weatherNow.getWindScale());
    Serial.print("风力: ");
    Serial.println(WindScale);   
    //获取风力等级
    GetHumidity = String(weatherNow.getHumidity());
    Serial.print("相对湿度: ");
    Serial.println(GetHumidity);    
    //获取实况相对湿度百分比数值
    Precip = String(weatherNow.getPrecip());
    Serial.print("降水量: ");
    Serial.println(Precip);      
    //获取实况降水量单位：毫米
    Serial.println("========================");
    } else {   
    //更新失败
    Serial.println("更新失败...");
    Serial.print("状态码: ");
    WeatherServerCode = String(weatherNow.getServerCode());
    Serial.println(AirServerCode); 
  }
}
void getAirQuality(){
    if(AirQuality.get()){ 
    //获取更新
    Serial.println("======空气质量信息======");
    AirServerCode = String(AirQuality.getServerCode());
    Serial.print("API状态码: ");
    Serial.println(AirServerCode); 
    //获取API状态码
    Aqi = String(AirQuality.getAqi());
    Serial.print("空气质量指数(AQI): ");
    Serial.println(Aqi);       
    //实时空气质量指数
    AirCategory = String(AirQuality.getCategory());
    Serial.print("空气质量指数级别: ");
    Serial.println(AirCategory);  
    //实时空气质量指数级别
    AirPrimary = String(AirQuality.getPrimary());
    Serial.print("主要污染物: ");
    Serial.println(AirPrimary);    
    //实时空气质量的主要污染物，优时返回值为 NA
    Serial.println("========================");
  } else {  
    //更新失败
    Serial.println("空气质量信息更新失败...");
    Serial.print("状态码: ");
    WeatherServerCode = String(AirQuality.getServerCode());
    Serial.println(WeatherServerCode); 
}
}
String GetMonth(){
  datefirst1 = String(weatherNow.getLastUpdate());
  datefirst1.setCharAt(0,' ');
  datefirst1.setCharAt(1,' ');
  datefirst1.setCharAt(2,' ');
  datefirst1.setCharAt(3,' ');
  datefirst1.setCharAt(4,' ');
  datefirst1.setCharAt(7,' ');
  datefirst1.setCharAt(8,' ');
  datefirst1.setCharAt(9,' ');
  datefirst1.setCharAt(10,' ');
  datefirst1.setCharAt(11,' ');
  datefirst1.setCharAt(12,' ');
  datefirst1.setCharAt(13,' ');
  datefirst1.setCharAt(14,' ');
  datefirst1.setCharAt(15,' ');
  datefirst1.setCharAt(16,' ');
  datefirst1.setCharAt(17,' ');
  datefirst1.setCharAt(18,' ');
  datefirst1.setCharAt(19,' ');
  datefirst1.setCharAt(20,' ');
  datefirst1.setCharAt(21,' ');
  datefirst1.replace(" ", "");
  return datefirst1;
  //对字符串进行操作，得到月份
}
String GetDate(){
  datefirst2 = String(weatherNow.getLastUpdate());
  datefirst2.setCharAt(0,' ');
  datefirst2.setCharAt(1,' ');
  datefirst2.setCharAt(2,' ');
  datefirst2.setCharAt(3,' ');
  datefirst2.setCharAt(4,' ');
  datefirst2.setCharAt(5,' ');
  datefirst2.setCharAt(6,' ');
  datefirst2.setCharAt(7,' ');
  datefirst2.setCharAt(10,' ');
  datefirst2.setCharAt(11,' ');
  datefirst2.setCharAt(12,' ');
  datefirst2.setCharAt(13,' ');
  datefirst2.setCharAt(14,' ');
  datefirst2.setCharAt(15,' ');
  datefirst2.setCharAt(16,' ');
  datefirst2.setCharAt(17,' ');
  datefirst2.setCharAt(18,' ');
  datefirst2.setCharAt(19,' ');
  datefirst2.setCharAt(20,' ');
  datefirst2.setCharAt(21,' ');
  datefirst2.replace(" ", "");
  return datefirst2;
}
String NewHumidity(){
  Humidity = String(GetHumidity);
  Humidity.setCharAt(2, ' ');
  Humidity.setCharAt(3, ' ');
  Humidity.setCharAt(4, ' ');
  Humidity.replace(" ", "");
  return Humidity;
}
void ConnectWiFi(){
  WiFi.mode(WIFI_STA);
  //将esp8266设置为STA模式
  WiFi.begin(ssid, password);
  //尝试使用所设置的password连接名为ssid的wifi
  Serial.print("正在连接WiFi：");
  Serial.println(ssid);
  int wifiConnectionTime = 0;
  //设置WiFi连接等待时间
  while (WiFi.status() != WL_CONNECTED && (wifiConnectionTime < 10)) {
  //如果WiFi.status()函数返回值不为"WL_CONNECTED"即未连接成功且等待时间不为10*1000毫秒即10秒则重复执行以下函数
    delay(1000);
    Serial.print('.');
    wifiConnectionTime++;
  }
  if (wifiConnectionTime == 10) { 
    //10s后仍未成功连接连接WiFi
    Serial.println("");
    Serial.println("WiFi连接失败！");
    Serial.println("请使用WiFiimanger配网");
  } else { // 成功
    Serial.println("");
    Serial.println("WiFi连接成功");
    Serial.print("IP地址: ");
    Serial.println(WiFi.localIP());
  }
}
String itimecharge(String itimeType){
  itime1 = itime;//时和分88：88
  itime1.setCharAt(7, ' ');
  itime1.setCharAt(6, ' ');
  itime1.setCharAt(5, ' ');
  if(itimeType == "hour"){
  itime2 = itime1;//88
  itime2.setCharAt(2, ' ');
  itime2.setCharAt(3, ' ');
  itime2.setCharAt(4, ' ');
  return itime2;
  }
  else if(itimeType == "minutes"){
  itime3 = itime1;//88
  itime3.setCharAt(0, ' ');
  itime3.setCharAt(1, ' ');
  itime3.setCharAt(2, ' ');
  return itime3;
    }
}
