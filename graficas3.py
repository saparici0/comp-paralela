import numpy as np
import matplotlib.pyplot as plt
import re
from scipy.interpolate import make_interp_spline


#datos = np.genfromtxt('reporte_3.txt', delimiter=',')[:,2]
data = []
with open('reporte_3.txt') as input:
    for line in input:
        data.append(re.findall("[0-9]+.[0-9]*", line))
datos = np.matrix(data)

for i in range(0,len(data),6):
    X = [ '('+bloques.strip() +','+ hilos.strip()+')' for _,bloques, hilos,_ in data[i:i+6] ]
    Y = np.squeeze(np.asarray(datos[i : i + 6, 3].astype(float)))
    plt.clf()
    plt.plot( X, Y)
    plt.title("CUDA Response Time "+data[i][0])
    plt.xlabel("Número de bloques, Número de hilos por bloque")
    plt.ylabel("Response Time (seg)")
    plt.savefig('graficas/CUDA Response Time'+data[i][0]+'.jpg')
"""
#Response Time
for kernel in range(3):
    plt.clf()
    plt.plot(x,datos[kernel*15 : kernel*15 + 5])
    plt.plot(x,datos[kernel*15 + 5 : kernel*15 + 10])
    plt.plot(x,datos[kernel*15 + 10 : kernel*15 + 15])
    plt.legend(['720p','1080p','4k'])
    plt.title("OMP Response Time Kernel "+str(kernel+1))
    plt.xlabel("Número de hilos")
    plt.ylabel("Response Time (seg)")
    plt.savefig('graficas/OMP Response Time Kernel '+str(kernel+1)+'.jpg')

#Speed Up
for kernel in range(3):
    plt.clf()
    plt.plot(x,datos[kernel*15] / datos[kernel*15 : kernel*15 + 5])
    plt.plot(x,datos[kernel*15 + 5 ] / datos[kernel*15 + 5 : kernel*15 + 10])
    plt.plot(x,datos[kernel*15 + 10] / datos[kernel*15 + 10 : kernel*15 + 15])
    plt.legend(['720p','1080p','4k'])
    plt.title("SpeedUp Kernel "+str(kernel+1))
    plt.xlabel("Número de hilos")
    plt.ylabel("SpeedUp")
    plt.savefig('graficas/Speed Up Kernel '+str(kernel+1)+'.jpg')


"""