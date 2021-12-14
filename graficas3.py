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

#Response Time
for i in range(0,len(data),6):
    X = [ '('+bloques.strip() +','+ hilos.strip()+')' for _,bloques, hilos,_ in data[i:i+6] ]
    Y = np.squeeze(np.asarray(datos[i : i + 6, 3].astype(float)))
    plt.clf()
    plt.plot( X, Y)
    plt.title("CUDA Response Time "+data[i][0])
    plt.xlabel("Número de bloques, Número de hilos por bloque")
    plt.ylabel("Response Time (seg)")
    plt.savefig('graficas/CUDA Response Time'+data[i][0]+'.jpg')

#Speed Up
secuencial = [0.882095, #720p
            2.113801, #1080p
            8.847241, #4k
            30.082776, #8k
            71.120695] #12k
for i in range(0,len(data),6):
    X = [ '('+bloques.strip() +','+ hilos.strip()+')' for _,bloques, hilos,_ in data[i:i+6] ]
    Y = np.squeeze(np.asarray(datos[i : i + 6, 3].astype(float)))
    plt.clf()
    plt.plot( X, (secuencial[i//6]) / Y)
    plt.title("CUDA Speed Up "+data[i][0])
    plt.xlabel("Número de bloques, Número de hilos por bloque")
    plt.ylabel("Speed Up")
    plt.savefig('graficas/CUDA Speed Up '+data[i][0]+'.jpg')
