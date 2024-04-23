import util as cc
import matplotlib.pyplot as plt
import numpy as np
import math

def count_sent_num( p_info ):
    Len = len(p_info.replys)
    return p_info.replys[Len - 1].seq

def count_lose_num( p_info ):
    return count_sent_num( p_info ) - len(p_info.replys)

def find_min_RTT( p_info ):
    min_RTT = 0xFFFFFFFF
    for pr in p_info.replys:
        min_RTT = min(min_RTT, pr.time)
    return min_RTT

def find_max_RTT( p_info ):
    max_RTT = 0
    for pr in p_info.replys:
        max_RTT = max(max_RTT, pr.time)
    return max_RTT

def print_longest_receive( p_info ):
    left, right = 0, 1
    max_len, beg_left, end_right = 0, 0, 0
    while(left < len(p_info.replys) ):
        while(right < len(p_info.replys) and p_info.replys[right].seq == p_info.replys[right - 1].seq + 1):
            right += 1
        tem_len = right - left
        if( tem_len > max_len ):
            max_len = tem_len
            beg_left = p_info.replys[left].seq
            end_right = p_info.replys[right - 1].seq
        left = right
        right = left + 1
    tem_len = right - left
    if( tem_len > max_len ):
        max_len = tem_len
        beg_left = p_info.replys[left].seq
        end_right = p_info.replys[right - 1].seq
    print("longest burst of receve is: [" + str(beg_left) + ", " + str(end_right) + "], len is " + str(max_len))

def print_longest_loss( p_info ):
    left, right = 0, 1
    max_len, beg_left, end_right = 0, 0, 0
    while(left < len(p_info.replys) - 1):
        if( p_info.replys[right].seq == p_info.replys[left].seq + 1):
            left += 1
            right = left + 1
            continue
        tem_len = p_info.replys[right].seq- p_info.replys[left].seq - 1
        if(tem_len > max_len):
            max_len = tem_len
            beg_left = p_info.replys[left].seq + 1
            end_right = p_info.replys[right].seq - 1
        left = right
        right = left + 1
    
    print("longest burst of lose is: [" + str(beg_left) + ", " + str(end_right) + "], len is " + str(max_len))

def paint_RTT_with_T( p_info ) :
    time, rtt = [], []
    t = -0.2
    for pr in p_info.replys:
        rtt.append( pr.time )
        t += 0.2
        time.append( t )
    plt.figure(figsize=(12,6), dpi=80)
    use = plt.gca()
    use.plot(time, rtt)
    plt.title("The value of RTT in different time")
    plt.show() 

def paint_RTT_with_proportion( p_info ):
    rtt_count = {}
    for pr in p_info.replys:
        if str(pr.time) not in rtt_count:
            rtt_count[str(pr.time)] = 1
            continue
        rtt_count[str(pr.time)] += 1
    total = len(p_info.replys)
    proportion, rtt = [], []
    for rtt_, count_ in rtt_count.items():
        rtt.append(rtt_)
        proportion.append((count_ / total) * 100)
    plt.figure(figsize=(12, 6), dpi=80)
    # use = plt.gca()
    plt.plot(rtt, proportion)
    tem_x = []
    for i in range (0, len(rtt), 30):
        tem_x.append(rtt[i])
    plt.title(" distribution of RTTs observed")
    plt.xticks(tem_x)
    plt.show()

def paint_N_N_plus_1( p_info ) :
    x, y = [], []
    for i in range(len(p_info.replys) - 1):
        x.append(p_info.replys[i].time)
        y.append(p_info.replys[i + 1].time)
    plt.figure(figsize=(12, 6), dpi=80)
    plt.title("RTT of ping #N” and “RTT of ping #N+1")
    plt.scatter(x, y, 7)
    plt.show()

def show_info( file_name ):
    file = cc.PingInfo("data[41.186.255.86].txt")
    print("fileName = " + str(file.file_name))
    sent_num, lost_num = count_sent_num(file), count_lose_num(file)
    rate = (1 - (lost_num / sent_num)) * 100
    print("send-num: " + str(sent_num) + "\tlost-num: " + str(lost_num) + "\trate: " + str(rate) + "%")
    print("---------------------------------")
    print("max-RTT: " + str(find_max_RTT( file )) + "\tmin-RTT: " + str(find_min_RTT( file )))
    print("---------------------------------")
    print_longest_receive( file )
    print_longest_loss( file )
    paint_RTT_with_T( file )
    paint_RTT_with_proportion( file )
    paint_N_N_plus_1( file )
    print("\n\n\n\n")

    



if __name__ == "__main__":
    # data[www.canterbury.ac.nz].txt
    # data[162.105.253.58].txt
    # data[41.186.255.86].txt
    show_info("data[www.canterbury.ac.nz].txt")
    show_info("data[162.105.253.58].txt")
    show_info("data[41.186.255.86].txt")
