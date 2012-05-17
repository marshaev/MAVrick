import pygame
from pygame import *
import operator_telnet

class joys:

    joystick_count = 0
    throttleHold = 0
    stepCount = 0
    collectiveTrim = 0
    
    def __init__(self):
        # Count the joysticks the computer has
        self.joystick_count=pygame.joystick.get_count()
        if self.joystick_count == 0:
            # No joysticks!
            print ("Error, I didn't find any joysticks.")
        else:
            # Use joystick #0 and initialize it
            self.my_joystick = pygame.joystick.Joystick(0)
            self.my_joystick.init()

        #don't clutter event queue with axis motion because will be polled explicitely.
        pygame.event.set_blocked(JOYAXISMOTION)


    def getAxes(self):
        tail = pygame.joystick.Joystick(0).get_axis(0) #left_horiz thumb
        collective = -(pygame.joystick.Joystick(0).get_axis(1)) #left_vert thumb
        pitch = -(pygame.joystick.Joystick(0).get_axis(3)) #right_vert thumb
        roll = pygame.joystick.Joystick(0).get_axis(4) #right_horiz thumb
        vector = (roll, pitch, collective, tail)
        return vector

    def checkEvents(self): #Button checking
        for e in event.get():
            eventstrings = []
            if e.type == JOYBUTTONDOWN:
                if e.dict['button'] == 0:
                    #button A
##                    eventstrings.append(self.servoTrim(1, 2, 3, 4))
                    eventstrings.append('g' + chr(129) + '\x00')
                elif e.dict['button'] == 1:
                    #button B
                    #kill switch
                    eventstrings.append(self.throttle(-100))
                elif e.dict['button'] == 2:
                    #button X
                    eventstrings.append('d' + 'd' + '\0')
                    print 'Button X'
                elif e.dict['button'] == 3:
                    #button Y
                    eventstrings.append(self.setKPID())
                    print 'Button Y'
                elif e.dict['button'] == 4:
                    #left top trigger button
                    #decrease throttle by 25 percent
                    eventstrings.append(self.throttle(-5))
                elif e.dict['button'] == 5:
                    #right top trigger button
                    #increase throttle by 25%
                    eventstrings.append(self.throttle(5))
                elif e.dict['button'] == 6:
                    #back button
                    print 'Button back'
                elif e.dict['button'] == 7:
                    #start button
                    print 'Button start'
                elif e.dict['button'] == 8:
                    #left thumbstick button
##                    eventstrings.append('g' + chr(129) + '\0')
                    print 'Button left thumbstick'
                elif e.dict['button'] == 9:
                    #right thumbstick button
                    print 'Button right thumbstick'
            if e.type == JOYHATMOTION:
                hat = e.dict['value']
                if hat[0] == 1: #right
                    pass
                elif hat[0] == -1: #left
                    pass
                elif hat[1] == 1:#up
                    trimCollective(1)
                elif hat[1] == -1: #down
                    trimCollective(-1)
            return eventstrings
        

    def toCharPair(self, Jack): #takes an int, returns two chars representing 16 bit value. Positive only.
        Jack = Jack * 100
        lsb = chr(int(Jack) & 0xFF) # Get bottom byte
        msb = chr((int(Jack)>>8) & 0xFF) #Get second byte
        return msb+lsb
        
        

    def setKPID(self):
        KPp = 1.0 #pitch - proportional
        KIp = 0.0 #pitch - integral
        KDp = 0.0 #pitch - derivative
        KPr = 1.5 #roll  - proportional
        KIr = 0.0 #roll  - integral
        KDr = 0.0 #roll  - derivative
        KPy = 1.0 #yaw   - proportional
        KIy = 0.3 #yaw   - integral
        KDy = 0.01 #yaw   - derivative

        Beta = 0.02 #Gain for IMU convergence

        Kpp = self.toCharPair(KPp)
        Kip = self.toCharPair(KIp)
        Kdp = self.toCharPair(KDp)
        Kpr = self.toCharPair(KPr)
        Kir = self.toCharPair(KIr)
        Kdr = self.toCharPair(KDr)
        Kpy = self.toCharPair(KPy)
        Kiy = self.toCharPair(KIy)
        Kdy = self.toCharPair(KDy)
        beta = self.toCharPair(Beta)

        pitch = Kpp + Kip + Kdp
        roll = Kpr + Kir + Kdr
        yaw = Kpy + Kiy + Kdy

        packet = 's' + '\x01' + pitch + roll + yaw + beta + '\x00'
        print len(packet)
        return packet


    #assembles and sends the throttle hold packet
    #Params: the throttle percentage value
    def throttle(self, hold):
        #'t'
        self.throttleHold += hold
        if self.throttleHold > 100: #max of 100% throttle
            self.throttleHold = 100
        elif self.throttleHold < 0: #min of 0% throttle
            self.throttleHold = 0
        print 'Throttle: ' + str(self.throttleHold) + '%'
        return 't'+chr(self.throttleHold)+'\x00'

    def servoTrim(self, a, b, c, d):
        self.stepCount += 1
        step = chr(self.stepCount)
        packet = ('s' + chr(1) + chr(2) + chr(3) + chr(4))
        print int(self.stepCount)
        return packet
    
    def trimCollective(self, coll):
        self.collectiveTrim = self.collectiveTrim + coll
        if self.collectiveTrim < 0:
            self.collectiveTrim = 0
        elif self.collectiveTrim > 255:
            self.collectiveTrim = 255
        packet = 'r' + chr(self.collectiveTrim) + '\x00'
        print packet
        return packet
        
