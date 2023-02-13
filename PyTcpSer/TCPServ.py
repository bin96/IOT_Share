import socket
import threading
import time
import requests                 # 用于得到网页链接
import json                     # 用于解析JSON格式的库

def get_fans():
    UID = "1233366"
    headers = {'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:89.0) Gecko/20100101 Firefox/89.0'}
    try:
        response = requests.get('https://api.bilibili.com/x/relation/stat?vmid=' + UID + '&jsonp=jsonp',headers=headers)
    # 网络连接失败
    except:
        print("网络连接失败")
    # 成功得到网页
    else:
        J_data = json.loads(response.text)
        print("已爬取粉丝数：" + str(J_data['data']['follower']))
        return(J_data['data']['follower'])
    

def deal(conn, client):
    length = len(threading.enumerate()) #线程数量
    print(f'来自 {client} 的连接' + ' ' +time.strftime('%Y_%m_%d %H:%M:%S') + ' 线程:' + str(length))
    FansNum = str(get_fans()) + " "

    while True:
        conn.send(FansNum.encode('ascii'))

# 类型：socket.AF_INET 可以理解为 IPV4
# 协议：socket.SOCK_STREAM
server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server.bind(('0.0.0.0', 8266))  # (client客户端ip, 端口)
server.listen()  # 监听

while True:
    sock, addr = server.accept()  # 获得一个客户端的连接(阻塞式，只有客户端连接后，下面才执行)
    xd = threading.Thread(target=deal, args=(sock, addr))
    xd.start()  # 启动一个线程

