# Copyright Hans Gaensbauer (c) 2020

import numpy as np

path = "C:/Users/hansg/Desktop/HS2/DDS_Config/data/"
ptsperfile = 1200

#assumes arr is a 2d numpy array 
def write2file(arr):
    numfiles = int(np.ceil(arr.shape[1]/ptsperfile))
    f = open(path+"data1.txt", "w+")
    for chunk in range(numfiles):
        f = open(path + "data" + str(chunk) +".txt", "w+")
        for i in range(min(ptsperfile, arr.shape[1] - chunk * ptsperfile)):       #write the address
            address = chunk * ptsperfile + i
            flipped_addr = address #reverse_bit(address)  ##don't reverse it
            f.write("0" + format(((flipped_addr>>15) & 3), 'x') + ",")
            f.write(format(((flipped_addr>>7) & 0xFF), 'x') + ",")
            f.write(format((flipped_addr << 1) & 0xFF, 'x') + ",")
            f.write(format((arr[0,address] >> 8) & 0xFF, 'x')+ "," )
            f.write(format((arr[0,address]) & 0xFF, 'x')+ "," )
            f.write(format((arr[1,address] >> 8) & 0xFF, 'x')+ "," )
            f.write(format((arr[1,address]) & 0xFF, 'x')+ ",\n" )
        f.close()
    print(str(numfiles) + " files successfully written")

def reverse_bit(num):
    result = 0
    for _ in range(17):
        result = (result << 1) + (num & 1)
        num >>= 1
    return result

arr1d = np.array([int(0x7FFF*(np.sin(i*2*np.pi/100.0)+1)) for i in range(24000)])
arr2d = np.array([int(0x7FFF*(np.cos(i*2*np.pi/100.0)+1)) for i in range(24000)])
arr = np.stack((arr1d, arr2d), axis=0)
write2file(arr)