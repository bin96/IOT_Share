
/*
 * 智能语言控制控制，支持天猫、小爱、小度、google Assistent同时控制
 * 2021-08-12
 * QQ交流群：566565915
 * 官网https://bemfa.com 
 */
#include <ESP8266WiFi.h>
#include <ESP8266httpUpdate.h>

#define TCP_SERVER_ADDR "bemfa.com" //巴法云服务器地址默认即可
#define TCP_SERVER_PORT "8344" //服务器端口，tcp创客云端口8344

//********************需要修改的部分*******************//

#define DEFAULT_STASSID  "2222"    //WIFI名称，区分大小写，不要写错
#define DEFAULT_STAPSW   "12345687"  //WIFI密码
String UID = "c67**********************************1c";  //用户私钥，可在控制台获取,修改为自己的UID
String TOPIC = "DemoLight002";         //主题名字，可在控制台新建
String upUrl = "http://bin.bemfa.com/b/3Bc**************************WM=DemoLight01s.bin";
//**************************************************//
//最大字节数
#define MAX_PACKETSIZE 512
//设置心跳值30s
#define KEEPALIVEATIME 30*1000
//tcp客户端相关初始化，默认即可

WiFiClient TCPclient;
String TcpClient_Buff = "";//初始化字符串，用于接收服务器发来的数据
unsigned int TcpClient_BuffIndex = 0;
unsigned long TcpClient_preTick = 0;
unsigned long preHeartTick = 0;//心跳
unsigned long preTCPStartTick = 0;//连接
bool preTCPConnected = false;
int NoConnectCycle = 0;
//相关函数初始化
void updateBin();
//连接WIFI
void doWiFiTick();
void startSTA();

//TCP初始化连接
void doTCPClientTick();
void startTCPClient();
void sendtoTCPServer(String p);

//led控制函数，具体函数内容见下方
void servodriver(int deg);
void TurnOffLED();
void TurnOnLED();



/*
  *发送数据到TCP服务器
 */
void sendtoTCPServer(String p){
  if (!TCPclient.connected()) 
  {
    Serial.println("Client is not readly");
    return;
  }
  TCPclient.print(p);
}

void updateBin()
{
  WiFiClient UpdateClient;

  if (WiFi.status() != WL_CONNECTED)
  {
    doWiFiTick();
  }

  t_httpUpdate_return ret = ESPhttpUpdate.update(UpdateClient, upUrl);
}


/*
  *初始化和服务器建立连接
*/
void startTCPClient(){
  if(TCPclient.connect(TCP_SERVER_ADDR, atoi(TCP_SERVER_PORT))){
    Serial.print("\nConnected to server:");
    Serial.printf("%s:%d\r\n",TCP_SERVER_ADDR,atoi(TCP_SERVER_PORT));
    
    String tcpTemp="";  //初始化字符串
    tcpTemp = "cmd=1&uid="+UID+"&topic="+TOPIC+"\r\n"; //构建订阅指令
    sendtoTCPServer(tcpTemp); //发送订阅指令
    tcpTemp="";//清空
    /*
     //如果需要订阅多个主题，可再次发送订阅指令
      tcpTemp = "cmd=1&uid="+UID+"&topic="+主题2+"\r\n"; //构建订阅指令
      sendtoTCPServer(tcpTemp); //发送订阅指令
      tcpTemp="";//清空
     */
    
    preTCPConnected = true;
    preHeartTick = millis();
    TCPclient.setNoDelay(true);
  }
  else{
    Serial.print("Failed connected to server:");
    Serial.println(TCP_SERVER_ADDR);
    TCPclient.stop();
    preTCPConnected = false;
  }
  preTCPStartTick = millis();
}


/*
  *检查数据，发送心跳
*/
void doTCPClientTick(){
 //检查是否断开，断开后重连
   if(WiFi.status() != WL_CONNECTED) return;
  if (!TCPclient.connected()) {//断开重连
  if(preTCPConnected == true){
    preTCPConnected = false;
    preTCPStartTick = millis();
    Serial.println();
    Serial.println("TCP Client disconnected.");
    TCPclient.stop();
  }
  else if(millis() - preTCPStartTick > 1*1000)//重新连接
    startTCPClient();
  }
  else
  {
    if (TCPclient.available()) {//收数据
      char c =TCPclient.read();
      TcpClient_Buff +=c;
      TcpClient_BuffIndex++;
      TcpClient_preTick = millis();
      
      if(TcpClient_BuffIndex>=MAX_PACKETSIZE - 1){
        TcpClient_BuffIndex = MAX_PACKETSIZE-2;
        TcpClient_preTick = TcpClient_preTick - 200;
      }
      preHeartTick = millis();
    }
    if(millis() - preHeartTick >= KEEPALIVEATIME){//保持心跳
      preHeartTick = millis();
      Serial.println("--Keep alive:");
      NoConnectCycle = NoConnectCycle + 1;
      Serial.println(NoConnectCycle);
      sendtoTCPServer("ping\r\n"); //发送心跳，指令需\r\n结尾，详见接入文档介绍
    }
  }
  if((TcpClient_Buff.length() >= 1) && (millis() - TcpClient_preTick>=200))
  {
    TCPclient.flush();
    Serial.print("Rev string: ");
    NoConnectCycle = NoConnectCycle - 1;
    TcpClient_Buff.trim(); //去掉首位空格
    Serial.println(TcpClient_Buff); //打印接收到的消息
    String getTopic = "";
    String getMsg = "";
    if(TcpClient_Buff.length() > 15){//注意TcpClient_Buff只是个字符串，在上面开头做了初始化 String TcpClient_Buff = "";
          //此时会收到推送的指令，指令大概为 cmd=2&uid=xxx&topic=light002&msg=off
          int topicIndex = TcpClient_Buff.indexOf("&topic=")+7; //c语言字符串查找，查找&topic=位置，并移动7位，不懂的可百度c语言字符串查找
          int msgIndex = TcpClient_Buff.indexOf("&msg=");//c语言字符串查找，查找&msg=位置
          getTopic = TcpClient_Buff.substring(topicIndex,msgIndex);//c语言字符串截取，截取到topic,不懂的可百度c语言字符串截取
          getMsg = TcpClient_Buff.substring(msgIndex+5);//c语言字符串截取，截取到消息
          Serial.print("topic:------");
          Serial.println(getTopic); //打印截取到的主题值
          Serial.print("msg:--------");
          Serial.println(getMsg);   //打印截取到的消息值
   }
   if(getMsg  == "on"){       //如果是消息==打开
     TurnOnLED();
   }else if(getMsg == "off"){ //如果是消息==关闭
      TurnOffLED();
    }
   if(getMsg  == "update")
   {
      updateBin();
   }

   TcpClient_Buff="";
   TcpClient_BuffIndex = 0;
  }

  if(NoConnectCycle < 0)
  {
    NoConnectCycle = 0;
  }

  if(NoConnectCycle > 1)
  {
    NoConnectCycle = 0;
    startSTA();
  }

}
/*
  *初始化wifi连接
*/
void startSTA(){
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false); //关闭STA模式下wifi休眠，提高响应速度
  WiFi.begin(DEFAULT_STASSID, DEFAULT_STAPSW);
}



/**************************************************************************
                                 WIFI
***************************************************************************/
/*
  WiFiTick
  检查是否需要初始化WiFi
  检查WiFi是否连接上，若连接成功启动TCP Client
  控制指示灯
*/
void doWiFiTick(){
  static bool startSTAFlag = false;
  static bool taskStarted = false;
  static uint32_t lastWiFiCheckTick = 0;

  if (!startSTAFlag) {
    startSTAFlag = true;
    startSTA();
  }

  //未连接1s重连
  if ( WiFi.status() != WL_CONNECTED ) 
  {
    if (millis() - lastWiFiCheckTick > 1000)
    {
      lastWiFiCheckTick = millis();
    }
  }
  //连接成功建立
  else {
    if (taskStarted == false) {
      taskStarted = true;
      Serial.print("\r\nGet IP Address: ");
      Serial.println(WiFi.localIP());
      startTCPClient();
    }
  }
}

void TurnOffLED()
{
  digitalWrite(0, HIGH);
}

void TurnOnLED()
{
  digitalWrite(0, LOW);
}



// 初始化，相当于main 函数
void setup() {
  Serial.begin(9600);
  pinMode(0, OUTPUT);
  digitalWrite(0, HIGH);
  Serial.println("Beginning...");  
}

//循环
void loop() {
  doWiFiTick();
  doTCPClientTick();
}
