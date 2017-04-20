import socket
import time
import random

while True:
    ip = "127.0.0.1"
    port = 11229
    st = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
    st.connect((ip,port))
    r = random.randint(0,2)
    if r == 0:
        st.send(b'up')
    elif r == 1:
        st.send(b'down')
    else:
        st.send(b'grasp')
    st.close()
    time.sleep(0.5)
