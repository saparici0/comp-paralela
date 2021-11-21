import numpy as np
import matplotlib.pyplot as plt
from scipy.interpolate import make_interp_spline


datos = np.genfromtxt('reporte_2.txt',delimiter=',')[:,1]
x = ['1','2','4','8','16']

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
    plt.title("OMP Response Time Kernel "+str(kernel+1))
    plt.xlabel("Número de hilos")
    plt.ylabel("Response Time (seg)")
    plt.savefig('graficas/Speed Up Kernel '+str(kernel+1)+'.jpg')


