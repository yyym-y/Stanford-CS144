class EachReply:
    seq = 0
    ttl = 0
    time = 0.0
    def __init__( self, seq, ttl, time ):
        self.seq = int(seq)
        self.ttl = int(ttl)
        self.time = float(time)
    def __str__( self ):
        return "< seq=" + str(self.seq) + ", ttl=" + str(self.ttl) + ", time=" + str(self.time) + "ms >"
    def __lt__(self, other):
        if( self.seq < other.seq ):
            return True
        return False
    
class PingInfo:
    replys = []
    file_name = ""
    def __init__( self, file_name ):
        self.file_name = file_name
        self.read_file()
        self.replys = sorted( self.replys )

    def __str__( self ):
        begin_str = "fileName= " + self.file_name + "\n"
        replys_str = ""
        Len = 10 if len(self.replys) > 10 else len(self.replys)
        for i in range (0, Len):
            replys_str += str(self.replys[i]) + "\n"
        return begin_str + replys_str

    def push( self, ping_info ):
        self.replys.append(ping_info)

    @staticmethod
    def read_while_end( str_, pos ):
        end_pos = pos
        while( not(str_[end_pos] == ' ' or str_[end_pos] == '\n') ):
            end_pos += 1
        return str_[pos:end_pos:]

    def decode_line( self, str_ ):
        seq_pos, ttl_pos, time_pos = str_.find("icmp_seq"), str_.find("ttl"), str_.find("time")
        self.push( EachReply(
            self.read_while_end( str_, seq_pos + 9 ),
            self.read_while_end( str_, ttl_pos + 4), 
            self.read_while_end( str_, time_pos + 5),
        ))

    def read_file( self ):
        with open(self.file_name, 'r') as file: 
            line = file.readline() 
            while line:
                # print(line)
                if not line.startswith('[') :
                    line = file.readline()
                    continue
                self.decode_line(line)
                line = file.readline()
