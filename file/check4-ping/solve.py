import util as cc


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



if __name__ == "__main__":
    # data[www.canterbury.ac.nz].txt
    # data[162.105.253.58].txt
    # data[41.186.255.86].txt
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
