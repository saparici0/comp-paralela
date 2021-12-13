#!/bin/sh
rm reporte_3.txt

make practica3

#Kernel 1
./practica3 img/720p.jpg 1 cg 16 >> reporte_3.txt
./practica3 img/720p.jpg 1 cg 32 >> reporte_3.txt
./practica3 img/720p.jpg 1 cg 64 >> reporte_3.txt
./practica3 img/720p.jpg 1 cg 128 >> reporte_3.txt
./practica3 img/720p.jpg 1 cg 256 >> reporte_3.txt
./practica3 img/720p.jpg 1 cg 512 >> reporte_3.txt

./practica3 img/1080p.jpg 1 cg 16 >> reporte_3.txt
./practica3 img/1080p.jpg 1 cg 32 >> reporte_3.txt
./practica3 img/1080p.jpg 1 cg 64 >> reporte_3.txt
./practica3 img/1080p.jpg 1 cg 128 >> reporte_3.txt
./practica3 img/1080p.jpg 1 cg 256 >> reporte_3.txt
./practica3 img/1080p.jpg 1 cg 512 >> reporte_3.txt

./practica3 img/4k.jpg 1 cg 16 >> reporte_3.txt
./practica3 img/4k.jpg 1 cg 32 >> reporte_3.txt
./practica3 img/4k.jpg 1 cg 64 >> reporte_3.txt
./practica3 img/4k.jpg 1 cg 128 >> reporte_3.txt
./practica3 img/4k.jpg 1 cg 256 >> reporte_3.txt
./practica3 img/4k.jpg 1 cg 512 >> reporte_3.txt

./practica3 img/8k.jpg 1 cg 16 >> reporte_3.txt
./practica3 img/8k.jpg 1 cg 32 >> reporte_3.txt
./practica3 img/8k.jpg 1 cg 64 >> reporte_3.txt
./practica3 img/8k.jpg 1 cg 128 >> reporte_3.txt
./practica3 img/8k.jpg 1 cg 256 >> reporte_3.txt
./practica3 img/8k.jpg 1 cg 512 >> reporte_3.txt

./practica3 img/12k.jpg 1 cg 16 >> reporte_3.txt
./practica3 img/12k.jpg 1 cg 32 >> reporte_3.txt
./practica3 img/12k.jpg 1 cg 64 >> reporte_3.txt
./practica3 img/12k.jpg 1 cg 128 >> reporte_3.txt
./practica3 img/12k.jpg 1 cg 256 >> reporte_3.txt
./practica3 img/12k.jpg 1 cg 512 >> reporte_3.txt