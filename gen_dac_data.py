# Copyright Hans Gaensbauer (c) 2020
# Edited by Eli Reed, Aug 28 2021

# Generates canned data for testing the Direct-Digital Synthesizer Arbitrary
# Waveform Generator

import numpy as np
import sys
from os.path import expanduser
from os.path import exists
from os import makedirs

#assumes arr is a 2d numpy array 
def write2file(arr, path, ptsperfile):
    numfiles = int(np.ceil(arr.shape[1]/ptsperfile))
    for chunk in range(numfiles):
        with open(path + "data" + str(chunk) +".dat", "w+") as f:
            # write the address
            for i in range(min(ptsperfile, arr.shape[1] - chunk * ptsperfile)):
                address = chunk * ptsperfile + i
                # flipped_address = address #reverse_bit(address)  ##don't reverse it
                f.write("0" + format(((address>>15) & 3), 'x') + ",")
                f.write(format(((address>>7) & 0xFF), 'x') + ",")
                f.write(format((address << 1) & 0xFF, 'x') + ",")
                f.write(format((arr[0,address] >> 8) & 0xFF, 'x')+ "," )
                f.write(format((arr[0,address]) & 0xFF, 'x')+ "," )
                f.write(format((arr[1,address] >> 8) & 0xFF, 'x')+ "," )
                f.write(format((arr[1,address]) & 0xFF, 'x')+ "\n" )
    print(str(numfiles) + " files successfully written")

def reverse_bit(num):
    result = 0
    for _ in range(17):
        result = (result << 1) + (num & 1)
        num >>= 1
    return result

# main entry point
def main():
    # extract command line arguments
    if len(sys.argv) == 1:
        path = expanduser('~/Desktop/DDS_data/')
        ptsperfile = 1200
    elif len(sys.argv) == 3:
        path = sys.argv[1]
        if type(path) != str:
            print("Filepath isn't a string.")
            exit(1)
        try:
            ptsperfile = int(sys.argv[2])
        except:
            print("Points per file must be an integer.")
            exit(1)

    else:
        print('Usage: python.exe gen_dac_data.py <path> <points per file>')
        exit(1)
    
    # make and clean our path if necessary
    if not exists(path):
        makedirs(path)
    if path[-1] != '/':
        path = path + '/' 

    # generate canned data
    arr1d = np.array([int(0x7FFF*(np.sin(i*2*np.pi/100.0)+1)) for i in range(24000)])
    arr2d = np.array([int(0x7FFF*(np.cos(i*2*np.pi/100.0)+1)) for i in range(24000)])

    # combine into 1 array
    arr = np.stack((arr1d, arr2d), axis=0)

    write2file(arr, path, ptsperfile)
    exit(0)

if __name__ == "__main__":
    main()
