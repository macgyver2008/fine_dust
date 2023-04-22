import time
import serial

import matplotlib.pyplot as plt
#그래프르 그리는데 필요한 툴
from matplotlib.animation import FuncAnimation
import numpy as np
#시리얼 통신 연결
com = serial.Serial(
    port='COM3', #아두이노 연결된 포트로
    baudrate=9600, #통신속도
)
#새 연결 후 아두이노가 리부팅 되는 시간 대기
time.sleep(5)
#5초 후에 줄바꿈 문자를 포함한 start를 보내기
com.write(b'start\n')

# 그래프 설정
fig = plt.figure()
ax = plt.axes(xlim=(0,100), ylim=(0, 100))
x, y1, y2, y3 = [0], [0], [0], [0]

def animate(i, x, y1, y2, y3):
    if com.readable():

        data = com.readline()
        if not data:
            return
        data = data.decode('ascii')

        humi, temp, dust = map(float, data.split(","))

        x.append(x[-1] + 1)

        y1.append(humi)
        y2.append(temp)
        y3.append(dust)

        x = x[-100:]
        y1 = y1[-100:]
        y2 = y2[-100:]
        y3 = y3[-100:]

        ax.clear()
        ax.plot(x,y1)
        ax.plot(x, y2)
        ax.plot(x, y3)

animation = FuncAnimation(
    fig=fig,
    func=animate,
    fargs=(x, y1, y2, y3),
    cache_frame_data=False,
    interval=200
)
plt.show()
