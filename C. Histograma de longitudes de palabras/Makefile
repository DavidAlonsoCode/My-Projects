TARGET = histograma-1-enum-switch histograma-2-goto histograma-3-rec histograma-x Test1 Test2 Test3 TestX
OBJ = histograma-1-enum-switch.o histograma-2-goto.o histograma-3-rec.o Graficador.o main.o histograma-x.o Test.o
CC = gcc
CFLAGS = -std=c11 -Wall -pedantic-errors -g3
RM = rm -f #remover todos e ignorar los que no existen
#rm *.o --> remover todos los archivos ".o"

all: $(TARGET)

# Reglas para compilar cada histograma individualmente
1: histograma-1-enum-switch Test1
2: histograma-2-goto Test2
3: histograma-3-rec Test3
4: histograma-x TestX

histograma-1-enum-switch: histograma-1-enum-switch.o Graficador.o main.o
	$(CC) $(CFLAGS) histograma-1-enum-switch.o Graficador.o main.o -o histograma-1-enum-switch

histograma-2-goto: histograma-2-goto.o Graficador.o main.o
	$(CC) $(CFLAGS) histograma-2-goto.o Graficador.o main.o -o histograma-2-goto

histograma-3-rec: histograma-3-rec.o Graficador.o main.o
	$(CC) $(CFLAGS) histograma-3-rec.o Graficador.o main.o -o histograma-3-rec

histograma-x: histograma-x.o Graficador.o main.o
	$(CC) $(CFLAGS) histograma-x.o Graficador.o main.o -o histograma-x

#Tests
Test1: Test.o histograma-1-enum-switch.o
	$(CC) $(CFLAGS) Test.o histograma-1-enum-switch.o -o Test1

Test2: Test.o histograma-2-goto.o
	$(CC) $(CFLAGS) Test.o histograma-2-goto.o -o Test2

Test3: Test.o histograma-3-rec.o
	$(CC) $(CFLAGS) Test.o histograma-3-rec.o -o Test3

TestX: Test.o histograma-x.o
	$(CC) $(CFLAGS) Test.o histograma-x.o -o TestX


#Makefile sabe como crear los .o a traves de las dependencias, no hace falta aclararle la receta
histograma-1-enum-switch.o: histograma-1-enum-switch.c Graficador.h histograma.h
#	$(CC) $(CFLAGS) -c histograma-1-enum-switch.c -o histograma-1-enum-switch.o

histograma-2-goto.o: histograma-2-goto.c Graficador.h histograma.h
#	$(CC) $(CFLAGS) -c histograma-2-goto.c -o histograma-2-goto.o

histograma-3-rec.o: histograma-3-rec.c Graficador.h histograma.h
#	$(CC) $(CFLAGS) -c histograma-3-rec.c -o histograma-3-rec.o

histograma-x.o: histograma-x.c Graficador.h histograma.h
#	$(CC) $(CFLAGS) -c histograma-x.c -o histograma-x.o

Graficador.o: Graficador.c Graficador.h histograma.h
#	$(CC) $(CFLAGS) -c Graficador.c -o Graficador.o

main.o: main.c Graficador.h histograma.h
#	$(CC) $(CFLAGS) -c main.c -o main.o

Test.o: Test.c histograma.h
#	$(CC) $(CFLAGS) -c Test.c -o Test.o

# Regla para limpiar los archivos generados
.PHONY: clean
clean:
	$(RM) $(OBJ) $(TARGET) test.txt

#regla clean, que es del tipo "phony target", cuyo nombre no es el archivo de 
#salida, sino la receta