import sys
import pygame
from pygame import *
import joy
import operator_telnet

HOST = '169.254.4.252'
PORT = 2000


def main():
    init()

    joyst = joy.joys()
    optel = operator_telnet.optelnet(HOST, PORT)
    joyst.setKPID()

    #make a window (empty for now, but event queue relies on it)
    win = display.set_mode((640, 480), RESIZABLE)
    display.set_caption("MAVrick Aero")
    clock = pygame.time.Clock()

    going = True
    print 'GO!'
    while going:
        #Check the pygame queue for quit action
        if pygame.event.peek(pygame.QUIT):
            going = False
        ###############################
        optel.sendPacket('g' + chr(129) + '\x00')
        ###############################
        eventstrings = joyst.checkEvents()
        if eventstrings:
            for x in eventstrings:
##                print x
                optel.sendPacket(x)
        optel.velocityVector(joyst.getAxes())
##        print 'manualVector'
##        optel.manualVector(joyst.getAxes())
##        print 'receivePacket'
        optel.receivePacket()

        #limit to (argument)/s
        clock.tick(15)
##        clock.tick(4)

    pygame.quit()        


if __name__ == '__main__':
    main()
