# *Arreglos & Dimensiones — Total de Ventas*

#### Objetivos
- Demostrar capacidad de construcción de tipos compuestos mediante la aplicación sucesiva de producto cartesiano.
- Aplicación de ciclos for *clásicos* y for de *intervalos* (for-range).
- Aplicación de rendireccionamiento de flujos estándar.

#### Temas
- Producto cartesiano.
- Secuencia finitas.
- Tipos std::array<T,N>.
- Inferencia de tipo con *auto*.
- Streams (flujos).
- Redireccionamiento de entrada y salida.
- Interfaz fluida.
- for clásico.
- for intervalo.
  
#### Problema
- Esta son una serie de problemas que parten de una necesidad general: *"Dado los importes, mostrar las ventas totales"*, y que después se particulariza en necesidades puntuales.
    1. Dados los importes mostrar ventas totales.
    2. Dado los importes y meses (0 a 11), mostrar ventas totales por mes.
    3. Dado los importes, meses, y números de los tres vendedores (0, 1, 2), mostrar total de ventas por mes y vendedor.
    4. Dados los importes, meses, y números de los tres vendedores, y números de las cuatro regiones (0, 1, 2, 4), mostrar total de ventas por mes, vendedor y región.


#### Restricciones
- Se deben crear archivos con set de datos diferentes para cada solución:
    - Test0.txt
    - Test1.txt
    - Test2.txt
    - Test3.txt
- Los datos se extraen de *cin*, no vienen en ningún orden en particular, los importes son enteros el resultado se envía a *cout*.
- Aplicar std::array<T,N> y no T[N].
- Aplicar operación •.at(•) y no •[•].
- Total de importes: Las cuatro necesidades rondan en presentar el total de importes, hay varias soluciones posibles, las que se deben implementar son las que usan una única variable de múltiples dimensiones, son las que están marcadas con una estrella (★) a continuación.
    - Soluciones posibles a la necesidad #1:
        - ★ 1 variable entera (cero dimensiones).
    - Soluciones posibles a la necesidad #2:
        - 12 variables enteras (cero dimensiones, no aprovecha patrón).
        - ★ 1 variable arreglo de 12 enteros (una dimensión, aprovecha patrón).
    - Soluciones posibles a la necesidad #3:
        - 6 variables enteras.
        - 3 variables arreglos de 12 enteros.
        - ★ 1 variable arreglo de 3 arreglos de 12 enteros (dos dimensiones).
    - Soluciones posibles a la necesidad #4:
        - 144 variables enteras.
        - 12 variables arreglos de 3 arreglos de 12 enteros.
        - 4 variables arreglos de 3 arreglos de 12 enteros.
        - ★ 1 variable arreglo de 4 arreglos de 3 arreglos de 12 enteros (tres dimensiones).
- Cada solución debe estar en archivos fuente diferentes:
    - Dim0.cpp
    - Dim1.cpp
    - Dim2.cpp
    - Dim3.cpp

#### Tareas
- Por cada necesdidad:
    1. Diseñar el set de datos para la prueba.
    2. Implementar la solución.
    3. Ejecutar la solución con la redirección de la entrada para que lea del set de datos de prueba. Por ejemplo: ./Dim0 < Test0.txt>.
    4. Escribir las definiciones.

## Autores
- [Acosta, Tomas](https://github.com/Acosta-Tomas)
- [Alonso, David](https://github.com/DavidAlonsoCode)
- [Mardardjian, Joshua](https://github.com/lmardardjian)
- [Martinez, Ariel](https://github.com/Mart270)
- [Mendivil, Matias](https://github.com/MatiasMendivil)
- [Nadir, Maia](https://github.com/Maia18Nadir)
- [Romano, Mauro](https://github.com/mauroeromano)
- [Romero, Vanesa](https://github.com/vanenromero)
- [Simó, Bruno](https://github.com/BrunoSimoCaballero)

