#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266httpUpdate.h>

const char *updatessid = "2222";
const char *uapdatepassword = "12345687";
const char *APssid = "RalayWifi";
const char *APpassword = "12345687";
String upUrl = "http://bin.bemfa.com/b/3Bc**************************WM=Ralay1.bin";

WiFiServer server(100); //创建TCP server对象,并将端口指定为100

int SetCycle = 0;
int OnTimeMs = 0;
int OffTimeMs = 0;

void WifiConnect()
{
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  // WiFi.setSleep(false); //关闭STA模式下wifi休眠，提高响应速度
  WiFi.begin(updatessid, uapdatepassword);
  int WifiCycle = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
    WifiCycle++;
    if (WifiCycle > 20)
      break;
  }

  if ((WiFi.status() == WL_CONNECTED))
  {
    Serial.println("Connected");
    Serial.print("IP Address:");
    Serial.println(WiFi.localIP());
    delay(500);
  }
  else
  {
    Serial.println("Wifi not connected");
    delay(500);
  }
}

void updateBin()
{
  delay(5000);
  WifiConnect();
  Serial.println("start update");
  WiFiClient UpdateClient;
  t_httpUpdate_return ret = ESPhttpUpdate.update(UpdateClient, upUrl);
}

void APBegin()
{
  WiFi.softAP(APssid, APpassword);
  Serial.print("IP Adress:");
  Serial.println(WiFi.softAPIP());
  server.begin();
}

void TCPCheck()
{
  int SelectNum = 1;
  WiFiClient client = server.available(); //尝试建立客户对象
  if (client)                             //如果当前客户可用
  {
    Serial.println("[存在客户端连接]");
    String readBuff = "";      //读取信息暂存
    while (client.connected()) //如果客户端处于连接状态
    {
      if (client.available()) //如果有可读数据
      {
        char c = client.read(); //读取一个字节
        if (c == ' ')           //接收到符
        {
          // client.print("已收到: " + readBuff);   //向客户端发送
          Serial.println(SetCycle);
          Serial.println(OnTimeMs);
          Serial.println(OffTimeMs);
          break; //跳出循环
        }
        if ((c != '.') && (SelectNum == 1))
          SetCycle = SetCycle * 10 + (c - '0');
        if ((c != '.') && (SelectNum == 2))
          OnTimeMs = OnTimeMs * 10 + (c - '0');
        if ((c != '.') && (SelectNum == 3))
          OffTimeMs = OffTimeMs * 10 + (c - '0');
        readBuff += c;
        if (c == '.')
          SelectNum = SelectNum + 1;
      }
    }
    if (readBuff == "update")
      updateBin();
  }
}

void RelayCtrl()
{
  if ((SetCycle > 0) && (OnTimeMs > 0) && (OffTimeMs > 0))
  {
    for (SetCycle; SetCycle > 0; SetCycle--)
    {
      digitalWrite(0, LOW);
      delay(OnTimeMs);
      digitalWrite(0, HIGH);
      delay(OffTimeMs);
    }
  }
  SetCycle = 0;
  OnTimeMs = 0;
  OffTimeMs = 0;
}

void setup()
{
  pinMode(0, OUTPUT);
  Serial.begin(9600);
  Serial.println("Begin");
  digitalWrite(0, HIGH);
  APBegin();
}

void loop()
{
  TCPCheck();
  RelayCtrl();
}
