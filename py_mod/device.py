import usb.core
import time
import math
import numpy as np
import usb.util

'''
# Used to sample the pixel register synchronously

mouse = Mouse() # to indicate parameters when in need
while True:
    print(mouse.sample())
'''
class Mouse:
    def __init__(self, idVendor=0x046d, idProduct=0xc077, pixel_register=0x0D, avg_register=0x0B):
        self.device = usb.core.find(idVendor=idVendor, idProduct=idProduct)

        if self.device.is_kernel_driver_active(0):
            self.device.detach_kernel_driver(0)

        self.device.set_configuration()
        self.pixel_register = pixel_register
        self.avg_register = avg_register
    
    def write_register(self):
        self.device.ctrl_transfer(
            bmRequestType = 0x40, #Write
            bRequest = 0x01,
            wValue = 0x0000,
            wIndex = self.pixel_register, #PIX_GRAB register value
            data_or_wLength = None
        )
    
    def read_register(self, register):
        response = self.device.ctrl_transfer(bmRequestType = 0xC0, #Read
            bRequest = 0x01,
            wValue = 0x0000,
            wIndex = register,
            data_or_wLength = 1
        )
        return response
    
    def sample(self):
        self.write_register()
        pixel = self.read_register(self.pixel_register)
        pixel_sample = (time.time(), pixel)
        avg = self.read_register(self.avg_register)
        avg_sample = (time.time(), avg)
        return pixel_sample, avg_sample
