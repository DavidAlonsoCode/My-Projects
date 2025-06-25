module Library where
import PdePreludat


------------------- DOMINIO
data Ciudad = Ciudad {
    nombre :: String,
    anioDeFundacion :: Number,
    atraccionesPrincipales :: [String],
    costoDeVida :: Number
} deriving Show

------------------- PUNTO 1: VALOR DE UNA CIUDAD

valorDeUnaCiudad :: Ciudad -> Number
valorDeUnaCiudad (Ciudad _ anioDeFundacion atraccionesPrincipales costoDeVida)
    | anioDeFundacion < 1800 = 5 * (1800 - anioDeFundacion)
    | null atraccionesPrincipales = costoDeVida * 2
    | otherwise = costoDeVida * 3


------------------- PUNTO 2: CARACTERISTICAS DE LAS CIUDADES

-- INTEGRANTE 1: Alguna atraccion copada
esVocal :: Char -> Bool
esVocal = (`elem` "aeiouAEIOU")

tieneAtraccionCopada :: Ciudad -> Bool
tieneAtraccionCopada ciudad = ((>0) . length . atraccionesQueEmpiezanConVocal ) (atraccionesPrincipales ciudad)

atraccionesQueEmpiezanConVocal  :: [String] -> [String]
atraccionesQueEmpiezanConVocal  [] = []
atraccionesQueEmpiezanConVocal  (x:xs)
    | (esVocal . head) x = x : atraccionesQueEmpiezanConVocal  xs
    | otherwise = atraccionesQueEmpiezanConVocal  xs

-- INTEGRANTE 2: Ciudad sobria
esSobria :: Number -> Ciudad -> Bool
esSobria cantidadLetaras ciudad = ((length) (atraccionesPrincipales ciudad) == (length . atraccionesConTantaCantidadDeLetras cantidadLetaras) (atraccionesPrincipales ciudad))


atraccionesConTantaCantidadDeLetras :: Number -> [String] -> [String]
atraccionesConTantaCantidadDeLetras _ [] = []
atraccionesConTantaCantidadDeLetras cantidad (x:xs)
    | ((> cantidad) . length) x = x : atraccionesConTantaCantidadDeLetras cantidad xs
    | otherwise = atraccionesConTantaCantidadDeLetras cantidad xs


-- INTEGRANTE 3: Ciudad con nombre raro
tieneNombreRaro :: Ciudad -> Bool
tieneNombreRaro ciudad = ((<5).length) (nombre ciudad)


------------------- PUNTO 3: EVENTOS

-- Sumar una nueva atracción
agregarAtraccion :: String -> Ciudad -> Ciudad
agregarAtraccion nuevaAtraccion ciudad = ciudad {
    costoDeVida = costoDeVida ciudad * 1.2,
    atraccionesPrincipales = nuevaAtraccion : atraccionesPrincipales ciudad
}

-- INTEGRANTE 1: Crisis
atravesarCrisis :: Ciudad -> Ciudad
atravesarCrisis ciudad = ciudad {
    costoDeVida = costoDeVida ciudad * 0.9,
    atraccionesPrincipales =  disminuirAtracciones ciudad
}

disminuirAtracciones :: Ciudad -> [String]
disminuirAtracciones ciudad
    | null (atraccionesPrincipales ciudad) = []
    | otherwise = (init . atraccionesPrincipales) ciudad


-- INTEGRANTE 2: Remodelacion
remodelar :: Number -> Ciudad -> Ciudad
remodelar incremento ciudad = ciudad {
    nombre = "New " ++ nombre ciudad,
    costoDeVida = costoDeVida ciudad * (1 + (incremento/100))
}


-- INTEGRANTE 3: Reevaluación
reevaluacion :: Number -> Ciudad -> Ciudad
reevaluacion letrasSobria ciudad
    | esSobria letrasSobria ciudad = ciudad {costoDeVida = costoDeVida ciudad *1.1}
    | otherwise = ciudad {costoDeVida = costoDeVida ciudad -3}


------------------- PUNTO 4: LA TRANSFORMACION NO PARA

-- (reevaluacion 13. atravesarCrisis . remodelar 10 . agregarAtraccion "paseo costero") baradero


------------------- PUNTO 5: UN AÑO PARA RECORDAR

-- Punto 5.1: Los años pasan (Todos los integrantes)

type Evento = Ciudad -> Ciudad

data Anio = Anio {
    anio :: Number,
    eventos :: [Evento]
} deriving Show

aplicarEvento :: Ciudad -> Evento -> Ciudad
aplicarEvento ciudad evento = evento ciudad

pasarDeAnio :: Anio -> Ciudad -> Ciudad
pasarDeAnio anioQuePaso ciudad = foldl aplicarEvento ciudad (eventos anioQuePaso)
--fold está haciendo multiples veces "aplicarEvento ciudad (evento)" y los eventos van iterando


-- Punto 5.2: Algo mejor (Todos los integrantes)
--Criterio comparacion debe decir si la ciudad mejoró en el aspecto en el que el evento influyó
--entonces debe recibir el aspecto, el evento. y comparar la ciudad en ese aspecto con y sin el evento

mejoroCiudadMedianteCriterio :: Ciudad -> (Ciudad -> Number) -> Evento -> Bool
mejoroCiudadMedianteCriterio ciudad criterio evento = (> criterio ciudad). criterio . evento $ ciudad

cantidadAtracciones :: Ciudad -> Number
cantidadAtracciones = length . atraccionesPrincipales


-- Plantilla para punto 5.3/5.4/5.5
aplicarEventoQue :: (Ciudad -> t -> Evento -> Bool) -> t -> Anio -> Ciudad -> Ciudad
aplicarEventoQue funcionBooleana criterio anioQuePasa ciudad =
    foldl aplicarEvento ciudad . filter (funcionBooleana ciudad criterio) $ eventos anioQuePasa

{-
-- Punto 5.3: Costo de vida que suba (Integrante 1)
aplicarEventosQueSubenCostoDeVida anioQuePasa ciudad = aplicarEventoQue mejoroCiudadMedianteCriterio costoDeVida anioQuePasa ciudad 

-- Punto 5.4: Costo de vida que baje (Integrante 2)
aplicarEventosQueBajanCostoDeVida anioQuePasa ciudad = aplicarEventoQue (not.mejoroCiudadMedianteCriterio) costoDeVida anioQuePasa ciudad 

-- Punto 5.5: Valor que suba (Integrante 3)
aplicarEventoQueSubeValor anioQuePasa ciudad = aplicarEventoQue mejoroCiudadMedianteCriterio valorDeUnaCiudad anioQuePasa ciudad 
-}

aplicarEventosSegun :: (Evento -> Bool) -> Anio -> Ciudad -> Ciudad
aplicarEventosSegun finalidad anioQuePasa ciudad =
    foldl aplicarEvento ciudad . filter finalidad $ eventos anioQuePasa

-- Punto 5.3: Costo de vida que suba (Integrante 1)

aplicarEventosQueSubenCostoDeVida :: Anio -> Ciudad -> Ciudad
aplicarEventosQueSubenCostoDeVida anioQuePasa ciudad =
    aplicarEventosSegun (mejoroCiudadMedianteCriterio ciudad costoDeVida) anioQuePasa ciudad


-- Punto 5.4: Costo de vida que baje (Integrante 2)

aplicarEventosQueBajanCostoDeVida :: Anio -> Ciudad -> Ciudad
aplicarEventosQueBajanCostoDeVida anioQuePasa ciudad =
    aplicarEventosSegun (not . mejoroCiudadMedianteCriterio ciudad costoDeVida) anioQuePasa ciudad


-- Punto 5.5: Valor que suba (Integrante 3)
aplicarEventoQueSubeValor :: Anio -> Ciudad -> Ciudad
aplicarEventoQueSubeValor anioQuePasa ciudad =
    aplicarEventosSegun (mejoroCiudadMedianteCriterio ciudad valorDeUnaCiudad) anioQuePasa ciudad
{-
recibo la lista de eventos del año dado, la cual recibe filter, y la itera de forma que cada evento lo paso como parametro
de "mejoroCiudadMed....", la cual recibe previamente 2 parametros, la ciudad y el criterio. Pero a la vez
"mejoroCiudadMed..." es una funcion booleana, por lo que es un parametro de filter, el cual recibe una funcion y 
una lista, entonces termino filtrando todos los elementosde la lista de tal forma que me quede unicamente con 
los que aumente el valor de la ciudad. Una vez hecho esto, toda esta lista pasa como parametro de foldl-> el cual recibe una
funcion(aplicarEvento, la cual recibe una ciudad y un evento), una semilla la cual es la ciudad, y la
lista nombrada... Esta misma lista es la que va iterando y le da los elementos a aplicarEvento 
-}

------------------- PUNTO 6: FUNCIONES A LA ORDEN

-- Punto 6.1: Eventos ordenados (Integrante 1)

eventosOrdenados :: Anio -> Ciudad -> Bool
eventosOrdenados anioQuePasa ciudad = comprobarOrden (eventos anioQuePasa) ciudad
    where
        comprobarOrden [] _ = False
        comprobarOrden [evento] ciudad = True
        comprobarOrden (evento1:evento2:eventos) ciudad = costoDeVida (evento1 ciudad) <= costoDeVida (evento2 ciudad) && comprobarOrden (evento2:eventos) ciudad


-- Punto 6.2: Ciudades ordenadas (Integrante 2)

ciudadesOrdenadas :: Evento -> [Ciudad] -> Bool
ciudadesOrdenadas _ [] = False
ciudadesOrdenadas evento ciudades = comprobarOrden evento ciudades
    where
        comprobarOrden _ [ciudad] = True
        comprobarOrden evento (ciudad1:ciudad2:ciudades) = costoDeVida (evento ciudad1) <= costoDeVida (evento ciudad2) && comprobarOrden evento (ciudad2:ciudades)


-- Punto 6.3: Años ordenados (Integrante 3)

aniosOrdenados :: [Anio] -> Ciudad -> Bool
aniosOrdenados listaAnios ciudad = comprobarOrden listaAnios ciudad
    where
        comprobarOrden [] _ = False
        comprobarOrden (anio:[]) _ = True
        comprobarOrden (anio1:anio2:anios) ciudad = costoDeVida (pasarDeAnio anio1 ciudad) < costoDeVida (pasarDeAnio anio2 ciudad) && comprobarOrden (anio2:anios) ciudad


{- -------------- EJERCICIO 6.3 mal interpretado --------------------------------
-- [anio2021,anio2022,anio2023]
-- [anio2022,anio2021,anio2023]

aniosOrdenados :: [Anio] -> Ciudad -> Bool
aniosOrdenados listaAnios ciudad = comprobarOrden (concat.map eventos $ listaAnios) ciudad
    where 
        comprobarOrden [] _ = False
        comprobarOrden (evento:[]) _ = True
        comprobarOrden (evento1:evento2:es) ciudad = costoDeVida (evento1 ciudad) < costoDeVida (evento2 ciudad) && comprobarOrden (evento2:es) ciudad

"concat.map eventos $ listaAnios" me devuelve una lista con todos los eventos de cada año
"(evento1:evento2:es)" representa el primer evento, el segundo, y los demas
-}

------------------- PUNTO 7: AL INFINITO Y MÁS ALLÁ

-- Punto 7.1: Eventos ordenados (Integrante 1)

anio2024 :: Anio
anio2024 = Anio 2024 ([atravesarCrisis, reevaluacion 7] ++ map remodelar [1..])

-- Si se utiliza el anio2024 con la funcion eventosOrdenados del punto 6.1, habrá una respuesta por consola que sera False.
-- Debido a la evaluación diferida (lazy evaluation) que tiene el paradigma se puede trabajar con estructuras infinitas
-- siempre y cuando se asegure que el algoritmo converge. En este caso, el algoritmo converge a False ya que el anio2024
-- no tiene los eventos ordenados (la reevaluación genera más costoDeVida que la remodelación)


-- Punto 7.2: Ciudades ordenadas (Integrante 2)
--Definicon de lista de ciudades “disco rayado” en Spec.hs

-- discoRayado :: [Ciudad]
-- discoRayado = [azul, nullish] ++ cycle [caletaOlivia, baradero]


-- Sí, debido a la evaluación perezosa. Dado que "Azul" tiene un costo de vida mayor que "Nullish",
-- la primera comparación dará False, la función ciudadesOrdenadas dejará de evaluar la lista y dará como resultado False


-- Punto 7.3: Años ordenados (Integrante 3)

--Hecho en Spec por no poder traer los años a Library

amoxicilina = cura "infección"
bicarbonato = cura "picazón"
ibuprofeno = cura "dolor" . cura "hinchazón"
sugestion sintomas = []
todosLosMedicamentos = [sugestion,amoxicilina,bicarbonato,ibuprofeno]


cura sintoma = filter (/= sintoma)

--enfermedad = sintomas
malMovimiento = ["dolor"]
varicela = repeat "picazón"

mejorMedicamentoPara sintomas = find (curaTodosLos sintomas) todosLosMedicamentos
curaTodosLos sintomas medicamento = medicamento sintomas == []

find f = head . filter f