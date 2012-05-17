from datetime import datetime

class wread:
    
    logfile = "telemetry.csv"
    telemetry_list = []
    extended_list = []
    misc_list = []


    def __init__(self):
        pass

    def __del__(self): #object destructor
        self.write2file()

    def write2file(self):
        print 'wread a book'
        FILE = open(self.logfile, 'w')
        FILE.write('Time,roll,pitch\n')
        for pack in self.telemetry_list:
            FILE.write(pack)
##        for misc in self.misc_list:
##            FILE.write(misc)
##            print misc
        FILE.close()


    def telemetry(self, tel):
        time = str(datetime.time(datetime.now()))
        try:
            pitch = (self.eight2sixteen(tel[1],tel[2])/100 - 180)
            #EMERGENCY MAGIC NUMBER ALERT: 6 IS MAGICAL
            roll = (self.eight2sixteen(tel[3],tel[4])/100 - 180 + 6)
            telem_csv = time + str(pitch) + ','
            telem_csv = time + ',' + str(roll) + ',' + str(pitch) + '\n'
            self.telemetry_list.append(telem_csv)
##            print telem_csv #+ ':' + str(len(self.telemetry_list))
        except IndexError:
            pass

    def test(self,a,b,c):
        time = str(datetime.time(datetime.now()))
        tel_csv = time + ',' + str(a) + ',' + str(b) + ',' + str(c) + '\n'
        self.misc_list.append(tel_csv)
##        print tel_csv

    #Takes two chars and returns a 16-bit int.
    def eight2sixteen(self, msb, lsb):
        B = int(ord(msb))<<8 | ord(lsb)
        return B
