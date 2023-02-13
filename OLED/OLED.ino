#include <U8g2lib.h>
#include <ESP8266WiFi.h>
#include <ESP8266httpUpdate.h>

const char *ssid = "2222";
const char *password = "12345687";
const uint16_t port = 8266;
const char *host = "abc.xyz"; // ip or dns
WiFiClient client;                  //创建一个tcp client连接
String fansNum = "";
String fansNumDelay = "";
String upUrl = "http://bin.bemfa.com/b/3Bc**************************WM=BfansOLED.bin";
int ConnectFail = 0;

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0,U8X8_PIN_NONE);//实例化

void SigStr(int x,int y,char* DislayStr)
{
  u8g2.firstPage();
  do 
  { 
    u8g2.drawStr(x, y, DislayStr);//在指定位置显示字符串   
  } 
  while ( u8g2.nextPage() );
}

void DoubleStr(int x,int y,char* DislayStr1,char* DislayStr2)
{
  u8g2.firstPage();
  do 
  { 
    u8g2.drawStr(x, y, DislayStr1);
    u8g2.drawStr(x, y+20, DislayStr2);
  } 
  while ( u8g2.nextPage() );
}

void ThreeStr(int x,int y,char* DislayStr1,char* DislayStr2,char* DislayStr3)
{
  u8g2.firstPage();
  do 
  { 
    u8g2.drawStr(x, y, DislayStr1);
    u8g2.drawStr(x, y+20, DislayStr2);
    u8g2.drawStr(x, y+40, DislayStr3);
  } 
  while ( u8g2.nextPage() );
}

void WifiConnect()
{
  WiFi.disconnect();
  DoubleStr(5,20,"Waiting to","Connect WIFI");
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false); //关闭STA模式下wifi休眠，提高响应速度
  WiFi.begin(ssid, password);
  int WifiCycle = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    WifiCycle++;
    if(WifiCycle > 20) 
      break;
  }

  if((WiFi.status() == WL_CONNECTED))
  {
    SigStr(5,20,"Wifi connected");
    delay(1000);
  }
  else
  {
    SigStr(5,20,"Wifi not connected");
    delay(1000);
  }
}


void updateBin()
{
  WiFiClient UpdateClient;

  if(WiFi.status() != WL_CONNECTED)
  {
    WifiConnect();
  }

  SigStr(0, 20, "Start Update");   

  t_httpUpdate_return ret = ESPhttpUpdate.update(UpdateClient, upUrl);
}

char* unconstchar(const char* s) {
    if(!s)
      return NULL;
    int i;
    char* res = NULL;
    res = (char*) malloc(strlen(s)+1);
    if(!res){
        return NULL;;
    } else{
        for (i = 0; s[i] != '\0'; i++) {
            res[i] = s[i];
        }
        res[i] = '\0';
        return res;
    }
}

void setup() {
  u8g2.begin();//初始化
  //u8g2.setFont(u8g2_font_ncenB08_tr);//设定字体
  u8g2.setFont(u8g2_font_unifont_t_symbols);
  WifiConnect();
}

void loop(){

  if (!client.connect(host, port))
  {
    DoubleStr(5,20,"connect failed","wait 1 sec");
    ConnectFail = 1;
    delay(1000);
    return;
  }

  String readBuff = "";
  int Cycle = 0;
  while (client.connected())
  {
    if (client.available()) //如果有可读数据
    {
      char c = client.read(); //读取一个字节//也可以用readLine()等其他方法
      if (c == ' ')           //接收到符
      {
        client.print("已收到: " + readBuff);   //向客户端发送
        break;                                 //跳出循环
      }
      readBuff += c;
      Cycle = 0;
    }
    else
    {
      Cycle = Cycle + 1;
      delay(10);
      if(Cycle >= 6000)
        break;
    }
  }

  client.stop();
  fansNum = readBuff;
  
  if(readBuff == "update")
    updateBin();

  if(fansNum.compareTo(fansNumDelay) || ConnectFail)
  {
    ThreeStr(5,20,"Number of fans","in Bilibili:",unconstchar(fansNum.c_str()));
    ConnectFail = 0;
  }

  fansNumDelay = fansNum;
  delay(5000);

}
