## Máquinas de Estado — Histograma de longitud de palabras

- Numero de equipo: 7
- Autores de la resolución:
  
     | Usuario github      | Legajo     | Apellido | Nombre       |
     |:-------------------:|:----------:|:--------:|:------------:|
     | DavidAlonsoCode     | 208.689-0  | Alonso   | David        |
     | Maia18Nadir         | 209.105-7  | Nadir    | Maia         |
     | Lgalban             | 204.653-2  | Galban   | Lucas        |
     | Agustinf019         | 213.691-0  | Fernandez| Agustin      |
     | biancusca           | 203.487-6  | Castoldi | Bianca       |
     | jenniferfabflores   | 177.676-9  | Flores   | Jennifer     |
     | ValentinaIaniero    | 209.725-4  | Ianiero  | Valentina    |
     | mayraegarcia        | 209.721-7  | Garcia   | Mayra        |
     | Mkoirach            | 215.403.1  | Koirach  | Matias       |


# Objetivos
- Aplicar los conceptos de modularización.
- Utilizar las herramientas de compilación y construcción de ejecutables estudiadas.
- Aplicar máquinas de estado para el procesamiento de texto.
- Implementar máquinas de estado con diferentes métodos.

# Temas
- Árboles de expresión.
- Representación de máquinas de estado.

# Tareas

## Árboles de Expresión
1. Estudiar el programa del ejemplo en la sección 1.5.4 Conteo de Palabras de [KR1988].
2. Dibujar el árbol de expresión para la inicialización de los contadores: `nl = nw = nc = 0`.
3. Dibujar el árbol de expresión para la expresión de control del segundo `if`: `c == ' ' || c == '\n' || c == '\t'`.

## Máquina de Estado
1. Describir en lenguaje dot [DOT] y dentro del archivo `histograma.gv` la máquina de estado que resuelve el problema planteado.
2. Formalizar la máquina de estados como una n-upla, basarse en el Capítulo #1 del Volumen #3 de [MUCH2012].

## Implementaciones de Máquinas de Estado

### Implementación #1: Una variable para el estado actual
- Escribir el programa `histograma-1-enum-switch.c` que siga la Implementación #1, variante `enum` y `switch`.
- Utilizar `typedef` y `enum` en vez de `define`, para declarar la variable estado como: `State s = Out;`.
- Utilizar `switch` en vez de `if`.

**Readme.md:**
- Indicar ventajas y desventajas de la versión de [KR1988] y de esta implementación.

### Implementación #2: Sentencias `goto`
- ¿Tiene alguna aplicación `goto` hoy en día? ¿Algún lenguaje moderno lo utiliza?
  Hoy en dia la aplicacion del 'goto' no es recomendable, es más, hoy en dia la utilizacion de esto esta fuera de las buenas practicas de programacion esto se debe a la dificultad de lectura, comprension y mantenibilidad del codigo. Hoy en día esta practica fue remplazada en los lenguajes modernos por otras estructuras que permiten realizar el mismo manejo pero de una forma mas facil de comprender. Sin embargo, existen codigos que aun lo siguen utilizando pero suelen ser a bajo nivel para optimizar los manejos de la memoria o incluso evitar el MultiLineaple flujo de datos al tener mas de un archivo abierto. 
- Escribir el programa `histograma-2-goto.c` que siga la Implementación #2.

### Implementación #3: Funciones Recursivas
- ¿Es necesario que las funciones accedan a contadores? Si es así, ¿cómo hacerlo?
- Leer la sección 1.10 Variables Externas y Alcance y 4.3 Variables Externas de [KR1988].

- Escribir el programa `histograma-3-rec.c` que siga la Implementación #3.


# Restricciones
- La implementación de la máquina de estado debe ser "seleccionable":
  - En tiempo de traducción desde el makefile.
  - En tiempo de ejecución mediante reemplazo de dynamic link library.
  - En tiempo de ejecución mediante argumentos de la línea de comandos.
- La solución debe estar modularizada: las máquinas de estado no deben conocer del graficador y viceversa.
- Desde `main.c` se coordina todo.

# Crédito Extra
- Parametrizar si el histograma se dibuja vertical u horizontalmente.



# Respuestas

1-b)
```
nl = nw = nc = 0

  =
 / \
nl  = 
   / \
 nw   =
     / \
    nc  0
```

1-c)
```
c == ' ' || c == '\n' || c == '\t'

       ||
      /  \
     ||   c == '\t'
    /  \
c == ' ' c == '\n'
```
2-b)
Maquina representada mediante una 5-upla:
```
M = (Q, Σ , T, q0 , A) 

Donde:
Q = {IN,OUT}
Σ = ASCII
->Blancos = {' ','\t','\n'}
->NoBlancos = ASCII - Blancos
q0 = OUT
A = {
-> b = Blanco
-> nb = No blanco
}
A(2da opcion){
-> a = Blanco/ε
-> b = Blanco/(++len[wl-1])^(wl=0)
-> c = NoBlanco/++wl
-> d = NoBlanco/++wl 
}
T:
```
Con "A":
T: Q -> Q x Accion
| T   | b                          | nb                         |
|-----|----------------------------|----------------------------|
| OUT | (OUT,ε)                    | (IN,++wl)                  |
| IN  | (OUT,(++len[wl-1])^(wl=0)) | (IN,++wl)                  |

Se lee = "Del estado OUT, por "b", transita al estado "OUT" y realiza la accion "ε"


Con "A(2da opcion)":
T: Q -> Q
| T   | a   | b   | c  | d  |
|-----|-----|-----|----|----|
| OUT | OUT | --- | IN | -- |
| IN  | --- | OUT | -- | IN |

Se lee = "Del estado OUT, por "a" (accion), transita al estado OUT"


3-a)
ii.
Ventajas:
La implementación es más clara y legible debido al uso de enum y switch, lo que facilita el mantenimiento del código.
La regularización de la estructura permite automatizar la construcción del programa.
Desventajas:
Es menos eficiente que la versión original presentada en [KR1988] debido a la sobrecarga de las estructuras enum y switch.
Puede resultar más difícil de optimizar en comparación con el uso directo de if y macros #define.

3-b)
iv.
Aunque goto es generalmente evitado debido a que puede hacer que el código sea más difícil de entender y mantener, sigue teniendo aplicaciones en situaciones donde se necesita salir de múltiples niveles de anidación o manejar errores y limpieza de recursos en sistemas embebidos (con una unica funcion en especifico).

La respuesta a la otra pregunta es "Sí", goto se utiliza en lenguajes mas modernos que C y C++, aunque se lo suele representar de otras maneras.
Igualmente su uso es limitado y generalmente desaconsejado en favor de estructuras de control más claras y seguras.

3-c)
ii. Si varias funciones necesitan leer o modificar un contador común, entonces sí, es necesario que las funciones accedan al contador. El uso de variables externas es la mejor forma de compartir datos entre funciones sin necesidad de pasarlos como argumentos en cada llamada. Para que las funciones accedan a contadores debemos:
  1. Definir la variable externa en un archivo.
  2. Declarar la variable externa en otros archivos que necesiten acceder a ella usando extern.
  3. Acceder y modificar la variable en cualquier función dentro de esos archivos.

3-d)
i. La implementacion X representa los estados y transiciones mediante funciones, las cuales son ejecuatadas mediante un puntero a funciones