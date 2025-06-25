module Spec where
import PdePreludat
import Library
import Test.Hspec

-- CIUDADES

baradero :: Ciudad
baradero = Ciudad "baradero" 1615 ["parque del este", "museo alejandro barbich"] 150

nullish :: Ciudad
nullish = Ciudad "nullish" 1800 [] 140

caletaOlivia :: Ciudad
caletaOlivia = Ciudad "caleta olivia" 1901 ["el gorosito", "faro costanera"] 120

maipu :: Ciudad
maipu = Ciudad "Maipú" 1878 ["Fortín Kakel"] 115

azul :: Ciudad
azul = Ciudad "Azul" 1832 ["Teatro Español","Parque Municipal Sarmiento","Costanera Cacique Catriel"] 190

-- ANIOS

anio2015 :: Anio
anio2015 = Anio 2015 []

anio2021 :: Anio
anio2021 = Anio 2021 [atravesarCrisis, agregarAtraccion "Playa"]

anio2022 :: Anio
anio2022 = Anio 2022 [atravesarCrisis, remodelar 5, reevaluacion 7]

anio2023 :: Anio
anio2023 = Anio 2023 [atravesarCrisis, agregarAtraccion "Parque", remodelar 10, remodelar 20]


-- TESTS

correrTests :: IO ()
correrTests = hspec $ do  


------------------- PUNTO 1: VALOR DE UNA CIUDAD

  describe "Punto 1: Valor de una ciudad" $ do
    it "El valor de la ciudad Baradero debe ser 925" $ do
      valorDeUnaCiudad baradero `shouldBe` 925
    it "El valor de la ciudad Nullish debe ser 280" $ do
      valorDeUnaCiudad nullish `shouldBe` 280
    it "El valor de la ciudad Caleta Olivia debe ser 360" $ do
      valorDeUnaCiudad caletaOlivia `shouldBe` 360


------------------- PUNTO 2: CARACTERISTICAS DE LAS CIUDADES

-- INTEGRANTE 1: ALGUNA ATRACCION COPADA
  describe "Punto 2: Caracteristicas de las ciudades" $ do
    it "Baradero no tiene ninguna atraccion copada" $ do
      tieneAtraccionCopada baradero `shouldBe` False
    it "Nullish no tiene ninguna atraccion copada" $ do
      tieneAtraccionCopada nullish `shouldBe` False
    it "Caleta Olivia tiene alguna atraccion copada" $ do
      tieneAtraccionCopada caletaOlivia `shouldBe` True


-- INTEGRANTE 2: Ciudad sobria
  describe "Punto 2: Ciudad sobria" $ do
    it "Baradero es sobria con ataracciones de más de 14 letras" $ do 
      esSobria 14 baradero `shouldBe` True
    it "Baradero no es sobria con ataracciones de más de 15 letras" $ do 
      esSobria 15 baradero `shouldBe` False
    it "Nullish no es sobria con ataracciones de más de 5 letras" $ do
      esSobria 5 nullish `shouldBe` True


-- INTEGRANTE 3: Ciudad con nombre raro
  describe "Punto 2: Ciudad con nombre raro" $ do
    it "Maipú no tiene nombre raro" $ do 
      tieneNombreRaro maipu `shouldBe` False
    it "Azul tiene nombre raro" $ do 
      azul `shouldSatisfy` tieneNombreRaro


------------------- PUNTO 3: EVENTOS

-- Sumar una nueva atracción
  describe "Punto 3: Sumar una nueva atraccion" $ do
    it "Azul suma una atraccion y tiene 228 de costo de vida" $ do
      costoDeVida (agregarAtraccion "Balneario Municipal Alte. Guillermo Brown" azul) `shouldBe` 228
    it "Azul suma una atraccion y tiene 4 atracciones" $ do
      (length . atraccionesPrincipales . agregarAtraccion "Balneario Municipal Alte. Guillermo Brown") azul `shouldBe` 4


-- INTEGRANTE 1: Crisis
  describe "Punto 3: Atravesar una crisis" $ do
    it "Azul atraviesa una crisis y queda con 2 atracciones" $ do
      (length . atraccionesPrincipales . atravesarCrisis) azul `shouldBe` 2
    it "Azul atraviesa una crisis y el costo de vida disminuye un 10%" $ do
      (costoDeVida . atravesarCrisis) azul `shouldBe` 171
    

-- INTEGRANTE 2: Remodelacions
  describe "Punto 3: Remodelación " $ do
   it "Azul sufre una remodelacion su costo de vida aumenta un 50% " $ do
    nombre (remodelar 50 azul) `shouldBe` "New Azul"
    costoDeVida (remodelar 50 azul) `shouldBe` 285


-- INTEGRANTE 3: Reevaluación
  describe "Punto 3: Reevaluación" $ do
    it "Azul es reevaluada con 14 letras y su costo de vida baja 3 puntos" $ do
      (costoDeVida . reevaluacion 14) azul `shouldBe` 187
    it "Azul es reevaluada con 13 letras y su costo de vida sube 10%" $ do
      (costoDeVida . reevaluacion 13) azul `shouldBe` 209


------------------- PUNTO 5: UN AÑO PARA RECORDAR

-- Punto 5.1: Los años pasan (Todos los integrantes)
  describe "Punto 5.1: Los años pasan" $ do
    it "Azul atraviesa el año 2022 y los eventos de ese año la modifican" $ do
      nombre (pasarDeAnio anio2022 azul) `shouldBe` "New Azul"
      costoDeVida (pasarDeAnio anio2022 azul) `shouldBe` 197.505
      atraccionesPrincipales (pasarDeAnio anio2022 azul) `shouldBe` ["Teatro Español", "Parque Municipal Sarmiento"]
    it "Azul atraviesa el año 2015 y los eventos de ese año la modifican" $ do
      costoDeVida (pasarDeAnio anio2015 azul) `shouldBe` 190


-- Punto 5.2: Algo mejor (Todos los integrantes)
  describe "Punto 5.2: Algo mejor" $ do
    it "Azul no mejora el costo de vida al atravesar una crisis" $ do
      mejoroCiudadMedianteCriterio azul costoDeVida atravesarCrisis  `shouldBe` False
    it "Azul mejora costo de vida al agregar atraccion: Monasterio Trapense" $ do
      mejoroCiudadMedianteCriterio azul costoDeVida (agregarAtraccion "Monasterio Trapense") `shouldBe` True
    it "Azul mejora cantidad de atracciones al agregar atraccion: Monasterio Trapense" $ do
      mejoroCiudadMedianteCriterio azul cantidadAtracciones (agregarAtraccion "Monasterio Trapense") `shouldBe` True


-- Punto 5.3: Costo de vida que suba (Integrante 1)
  describe "Punto 5.3: Costo de vida que suba" $ do
    it "Azul atraviesa el año 2022 y los eventos que suben su costo de vida la modifican" $ do
      nombre (aplicarEventosQueSubenCostoDeVida anio2022 azul) `shouldBe` "New Azul"
      costoDeVida (aplicarEventosQueSubenCostoDeVida anio2022 azul) `shouldBe` 219.45


-- Punto 5.4: Costo de vida que baje (Integrante 2)
  describe "Punto 5.4: Costo de vida que baje" $ do
    it "Azul atraviesa el año 2022 y solo se aplican los eventos que bajan el costo de vida" $ do
      nombre (aplicarEventosQueBajanCostoDeVida anio2022 azul) `shouldBe` "Azul"
      costoDeVida (aplicarEventosQueBajanCostoDeVida anio2022 azul) `shouldBe` 171
      atraccionesPrincipales (aplicarEventosQueBajanCostoDeVida anio2022 azul) `shouldBe` ["Teatro Español","Parque Municipal Sarmiento"]


-- Punto 5.5: Valor que suba (Integrante 3)
  describe "Punto 5.5: Valor que suba" $ do
    it "nullish atraviesa el año 2022 y los eventos que suben su valor la modifican" $ do
      costoDeVida (aplicarEventoQueSubeValor anio2022 nullish) `shouldBe` 161.7


------------------- PUNTO 6: FUNCIONES A LA ORDEN

-- Punto 6.1: Eventos ordenados (Integrante 1)
  describe "Punto 6.1: Eventos ordenados" $ do
    it "Azul atraviesa el año 2022 que tiene los eventos ordenados en costo de vida creciente" $ do
      eventosOrdenados anio2022 azul `shouldBe` True
    it "Azul atraviesa el año 2023 que no tiene los eventos ordenados en costo de vida creciente" $ do
      eventosOrdenados anio2023 azul `shouldBe` False


-- Punto 6.2: Ciudades ordenadas (Integrante 2)
  describe "Punto 6.2: Ciudades ordenadas" $ do
    it "Caleta Olivia, Nullish, Baradero y Azul atraviesan una remodelación al 10% y estan ordenadas por costo de vida creciente" $ do
      ciudadesOrdenadas (remodelar 10) [caletaOlivia, nullish, baradero, azul] `shouldBe` True
    it "Caleta Olivia, Azul y Baraderoa traviesan una remodelación al 10% y no estan ordenadas por costo de vida creciente" $ do
      ciudadesOrdenadas (remodelar 10) [caletaOlivia, azul, baradero] `shouldBe` False


-- Punto 6.3: Años ordenados (Integrante 3)
  describe "Punto 6.3: Años ordenados" $ do
    it "Baradero atraviesa los años 2021,2022,2023 y al comparar el impacto de cada uno en el costo de vida, este no es creciente" $ do
      aniosOrdenados [anio2021,anio2022,anio2023] baradero `shouldBe` False
    it "Baradero atraviesa los años 2022,2021,2023 y al comparar el impacto de cada uno en el costo de vida, este es creciente" $ do
      aniosOrdenados [anio2022,anio2021,anio2023] baradero `shouldBe` True


------------------- PUNTO 7: AL INFINITO Y MAS ALLA

--(Integrante 2)
discoRayado :: [Ciudad]
discoRayado = [azul, nullish] ++ cycle [caletaOlivia, baradero]

--(Integrante 3)
listaAniosSinFin = [anio2021,anio2022] ++ repeat anio2023
{-
Si, hay un resultado, ya que la funcion en 6.3 compara los costos de vida de la cuidad atravesando 2 años distintos
y se fija si van en forma creciente. Y al tener el mismo costoDeVida (por el año 2023) el programa cortaría con "False"
Y debido al concepto de Lazy Evaluation, el motor no va a calcular la lista infinita, sino que va a calcular hasta que
el resultado de False, y ese resultado lo obtiene cuando se compara la ciudad afectada 2 veces con el año 2023
-}