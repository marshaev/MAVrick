import pygame
import telnetlib
import socket
import wread

'''
              ,
            __)\_  
      (\_.-'    o`-.
       \~~````\)~^^`    The great whitespace shark
      
_____   _____
|     | |     |
|  a  | |  b  |
|_____| |_____|
     _____
    |     |
    |  c  |
    |_____|
       |
       |
     __|__
    |     |
    |  r  |
    |_____|
	
	
Ranges of inputs:
	Collective:	0->80	(0=off, 80=full throttle [may be altered later])
	Pitch x:	-10->10 & 0<Collective+(Pitch_x*2)<100
	Pitch y:	-20->20 & 0<Collective+Pitch_y<100
	Roll:		-35->35
'''

class optelnet:
    TELEMETRY_LENGTH = 5
    DIAGNOSTICS_LENGTH = 6
    COLLECTIVE_CENTER = 127
    PITCH_CENTER = 50
    ROLL_CENTER = 50
    YAW_CENTER = 100
    
    
    receiveBuffer = ''
    s = None
    NULL = '\x00'
    stepCount = 0
    io = None
    
    
    def __init__(self, HOST, PORT):
        self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.s.connect((HOST, PORT))
        self.s.setblocking(0) #turn off blocking mode so won't freeze waiting for packets
        self.io = wread.wread()
        

    def sendPacket(self, packet):
        self.s.sendall(packet)

    def receivePacket(self):
        #Check for received data. Catch exception if none.
        try:
            self.receiveBuffer = self.receiveBuffer + self.s.recv(1024)
##            print self.receiveBuffer
##            crap = ''
##            for ch in self.receiveBuffer:
##                 crap = crap + ',' + str(ord(ch))
##            print(crap)
        except socket.error:
            pass
        while self.receiveBuffer:
            self.justiceleagueassemble()

    #Takes two chars and returns a 16-bit int.
    def eight2sixteen(self, msb, lsb):
##        print  str(ord(msb)) + ',' + str(ord(lsb))
        B = (int(ord(msb))<<8) | ord(lsb)
        return B


    #sorts incoming packets by header        
    def justiceleagueassemble(self):
        if self.receiveBuffer[0] == 'l':
            if len(self.receiveBuffer) >= self.TELEMETRY_LENGTH:
                self.decodeTelemetry(self.receiveBuffer[:self.TELEMETRY_LENGTH+1])
                self.receiveBuffer = self.receiveBuffer[self.TELEMETRY_LENGTH+1:]
            else:
                self.receiveBuffer = ''
        elif self.receiveBuffer[0] == 'd':
            if len(self.receiveBuffer) >= self.DIAGNOSTICS_LENGTH:
                self.decodeDiagnostics(self.receiveBuffer[:self.DIAGNOSTICS_LENGTH])
                self.receiveBuffer = self.receiveBuffer[self.DIAGNOSTICS_LENGTH:]
            else:
                self.receiveBuffer = ''
        elif self.receiveBuffer[0] == 'g':
##            print self.receiveBuffer
##            crap = ''
##            for ch in self.receiveBuffer:
##                 crap = crap + ',' + str(ord(ch))
##            print(crap)
            self.getterSorter()
        else:
            self.decodeTelemetry(self.receiveBuffer) #THIS IS TEMPORARY TEST CODE
            self.receiveBuffer = ''
##        print self.receiveBuffer

    def getterSorter(self):
        length = 0
        if self.receiveBuffer[1] == chr(129): #get IMU
            if(len(self.receiveBuffer) >= 9):
                pitch = self.eight2sixteen(self.receiveBuffer[2],self.receiveBuffer[3])/100
                roll = self.eight2sixteen(self.receiveBuffer[4],self.receiveBuffer[5])/100
                yaw = self.eight2sixteen(self.receiveBuffer[6],self.receiveBuffer[7])/100
                self.io.test(pitch,roll,yaw)
                self.receiveBuffer = self.receiveBuffer[9:]
        else:
            self.receiveBuffer = ''
##            print self.receiveBuffer
            

    #assembles and sends movement vector if in semi-auto
    #Params: thumbstick axes
    def velocityVector(self, vector):
        #'v'
        roll = int(round(self.ROLL_CENTER*(vector[0] + 1))) #scale joystick input
        pitch = int(round(self.PITCH_CENTER*(vector[1] +1)))
        collective = int(round(self.COLLECTIVE_CENTER*(vector[2] +1)))
        tail = int(round(self.YAW_CENTER*(vector[3] +1)))
##        print collective, tail, pitch, roll       
        packet = ('v'+ chr(collective) + chr(tail) + chr(pitch) + chr(roll) + '\x00')
##        print packet
##        print tail
        self.sendPacket(packet)

    #assembles and sends trim adjust vector
    #Params offset for each servo
    def servoTrim(self, a, b, c, r):
        #stuff here 's'
##        self.stepCount += 1
##        step = chr(self.stepCount)
##        packet = ('s' + chr(1) + chr(2) + chr(3) chr(4) + NULL)
##        self.sendPacket(packet)
        print self.stepCount

    

    #assembles and sends manual control vectors
    #Params: thumbstick axes 'm'
    def manualVector(self, vector):
        roll = int(round(self.ROLL_CENTER*(vector[0] + 1))) #scale joystick input
        pitch = int(round(self.PITCH_CENTER*(vector[1] +1)))
        collective = int(round(self.COLLECTIVE_CENTER*(vector[2] +1)))
        tail = int(round(self.YAW_CENTER*(vector[3] +1)))
        #print roll, pitch, collective, tail
        packet = ('m'+ chr(collective) + chr(tail) + chr(pitch) + chr(roll) + '\x00')
##        print packet
        self.sendPacket(packet)

    #sends offset to correct for drift
    #Params: 
    def velocityZero(self, roll, pitch, collective, tail):
        #stuff goes here 'z'
        print 'bleh'

    #Sends command to toggle extended telemetry packets.
    #Proper On/Off specification can be added once a GUI is implemented
    def toggleDiagnostic(self):
        #'d'
        if truthy:
            truthy = False
        else: truthy = True
        packet = ('d' + chr(truthy) + NULL) #chr(truthy) gives either 0x01 or 0x00
        self.sendPacket(packet)

    #make packet data human readable.
    def decodeTelemetry(self, tel):
        self.io.telemetry(tel)

    #make packet data human readable.
    def decodeDiagnostics(self, diag):
        #stuff here 'e'
        #a,b,c,r,yaw_k,huuuurrr
        pass
##        print ord(diag[3])
##        print ord(diag[4])
##        print ord(diag[5])
