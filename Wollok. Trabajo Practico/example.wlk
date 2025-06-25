object lossarnach {
    method puedeAtravesar(guerrero) = true
    method atravesar(guerrero) {
        guerrero.modificarCantidadDeVida(1)
    }
}
object caminoGondor{
    const zonas = []
    method zonas() = zonas
    method agregarZona(zona){
        if(not zonas.contains(zona)) // para no agegar una zona dos veces (revisar a futuro)
            zonas.add(zona)
    }
    method eliminarZona(zona){
        zonas.remove(zona)
    }
    method puedeAtravesarCamino(guerrero) = zonas.all({ zona => zona.puedeAtravesar(guerrero) }) //puede el guerrero atravesar cada zona de zonas del camino gondor? //all retorna un booleano
    method atraviesaCamino(guerrero) = zonas.forEach({ zona => zona.atravesar(guerrero) })//forEach trabaja con cada elemento
}


/*------------------ -------- ------------------*/
/*------------------ PARTE 2 ---------*---------*/
/*------------------ -------- ------------------*/



/*------------------ ARSENAL ------------------*/
//No se a que se refiere el enunciado antes del arsenal, cuando dice  
//"guerreros capaces de ir en contra de su beneficio", supongo que sera para despues

class Espada{
    const multiplicadorDePoder
    const origen
    method multiplicadorDePoder() = (multiplicadorDePoder.max(1)).min(20) //lo acoto entre 1 y 20
    method poder() = self.multiplicadorDePoder() * origen.valor()
}

object elfica {
    method valor() = 30
}

object enana {
    method valor() = 20
}

object humana {
    method valor() = 15
}

class Baculo {
    const property poder
}

class Daga inherits Espada{
    override method poder() = (self.multiplicadorDePoder() * origen.valor()) * 0.5
}

class Arco{
    var property tension = 40
    const longitud
    method poder() = 0.1 * (tension * longitud) 
}

class Hacha{
    const longitudMango
    const pesoHojaMetalica
    method poder() = longitudMango * pesoHojaMetalica
}


/*------------------ SOBRE LOS GUERREROS ------------------*/

class Guerrero {
    const property armas = []
    var vidaActual = 0
    const property items = []
    
    override method initialize(){
        if(!vidaActual.between(0,100)) throw new Exception(message="La vida esta mal!")
    }

    method vidaActual() = vidaActual
    method vidaActual(nuevaCantidad) { // setea cantidadDeVida igual al valor enviado por parámetro
        if(nuevaCantidad.between(0,100)) {
            vidaActual = nuevaCantidad
        }
    }
    method modificarCantidadDeVida(valor) { // modifica cantidadDeVida sumando o restando el valor enviado por parámetro
        vidaActual = ((vidaActual + valor).min(100)).max(0)
    }

    method estaFueraDeCombate() = vidaActual == 0

    method cantidadDeItems() {
        return items.count{true}
    }
    method cantidadDeItem(algo) {
        return items.count{item => item == algo}
    }
    method removerItem(item){
        self.items().remove(item)
    }
    method agregarItem(item){
        self.items().add(item)
    }

    method poder()
    method sumaDelPoderDeLasArmas() = armas.sum({ arma => arma.poder()})
}

class Hobbit inherits Guerrero {
    const cantidadDeItems = 0
    override method poder() =
        vidaActual + self.sumaDelPoderDeLasArmas() + cantidadDeItems
}

class Enano inherits Guerrero {
    const factorDePoder

    override method poder() =
        vidaActual + factorDePoder * self.sumaDelPoderDeLasArmas()
}

class Elfo inherits Guerrero {
    var property destrezaBase = elfo.destrezaBase()
    const destrezaPropia

    override method poder() =
        vidaActual + self.sumaDelPoderDeLasArmas() * (self.destrezaBase() + destrezaPropia)
}
object elfo {
  var property destrezaBase = 2 
}
class Humano inherits Guerrero {
    const limitadorDePoder

    override method poder() =
        vidaActual * self.sumaDelPoderDeLasArmas() / limitadorDePoder
}

class Maiar inherits Guerrero {
    var property factorDePoderBasico = 15
    var property factorDePoderBajoAmenaza = 300

    override method poder() =
        if (vidaActual>=10) self.definirPoder(factorDePoderBasico)
        else self.definirPoder(factorDePoderBajoAmenaza)
    
    method definirPoder(factorActual) = vidaActual * factorActual + self.sumaDelPoderDeLasArmas()*2

    method agregarArma(arma) { armas.add(arma) }

    method eliminarArmas() = armas.clear()

    //metodos para zonas
    method puedeAtravesar(zona) = zona.puedeAtravesar(self)
    method atravesar(zona) {zona.atravesar(self) }

    //metodos para caminos
    method puedeAtravesarCamino(camino) = camino.puedeAtravesarCamino(self)
    method atravesarCamino(camino) {camino.atraviesaCamino(self)}
}


object tomBombadil {
  const cantidadDeVida = 0
  const armas = []

  method cantidadDeVida() = cantidadDeVida
  method poder() = 10000000
  method armas() = armas

  //metodos para zonas
  method puedeAtravesar(zona) = true
  method atravesar(zona) {}
  
  //metodos para caminos
  method puedeAtravesarCamino(camino) = true
  method atravesarCamino(camino) {}
}

object gollum inherits Hobbit {
    override method poder() {
        return super() / 2
    }
}
/*------------------ Los Caminos de la Tierra Media ------------------*/
class Grupo {
    const property guerreros = []
    method guerrerosEnCombate() = guerreros.filter({guerrero => not guerrero.estaFueraDeCombate()})
    method cantidadDeItems(algo){
        return guerreros.sum{ guerrero => guerrero.cantidadDeItem(algo)}
    }
    method atravesarZona(zona){
        zona.atravesarZona(self)
    }
}

class Region {
    const property nombre
    const zonas = []
    method agregarZona(zona){
        zonas.add(zona)
    }
    method perteneceACamino(camino) {
       return zonas.any({zona => camino.contains(zona)})
    }
}

class ZonasNoLimitrofesException inherits Exception{}

class Camino {
    var property zonas 
    method puedePasar(grupo) {
        return zonas.all{zona => zona.puedePasarPorZona(grupo)}
    }

    //Tercera entrega - "Más sobre caminos"
    method sonZonasLimitrofes() {
        if( zonas.any{ zona1 => zonas.any{ zona2 => zona1.limitaCon(zona2)}}) true
        else { throw new ZonasNoLimitrofesException (message = "Al menos una zona no es limítrofe de las otras")}
    }
    /*
    method puedePasar(grupo) = zonas.all{zona => zona.puedePasarPorZona(grupo)}
    method atravesarCamino(grupo) = zonas.forEach({ zona => zona.atravesar(grupo) })//forEach trabaja con cada elemento
    */
    method regionesQueAtraviesa() {
      const regiones = []
      regionesExistentes.forEach({region =>if(region.perteneceACamino(self.zonas())) regiones.add(region.nombre()) })
      return regiones
    }
}

class NoSePuedePasarPorZonaException inherits Exception{}
class GuerreroFueraDeCombate inherits Exception{}


class Zona{
    //Tercera entrega - "Más sobre caminos"
    const property limites = []

    method agregarLimite(zonaLimitrofe) {
        limites.add(zonaLimitrofe)
        zonaLimitrofe.limites().add(self)
    }

    method limitaCon(otraZona) = limites.contains(otraZona)



    //METODOS CON GRUPO----------------------------------------------------------------
    method atravesarZona(grupo){
        if (self.puedePasarPorZona(grupo)) grupo.guerreros().forEach({guerrero => self.atravesarZonaGuerrero(guerrero)})
        else throw new NoSePuedePasarPorZonaException(message = "Error al pasar por zona")
    }
    
    method puedePasarPorZona(grupo) = self.cumpleRequerimientoItem(grupo) and self.cumpleRequerimientoDeGuerrero(grupo)
    method cumpleRequerimientoItem(grupo) = true        //Por defecto las zonas no tienen requerimientos de      
    method cumpleRequerimientoDeGuerrero(grupo) = true    // items ni de guerrero hasta override
    //---------------------------------------------------------------------------------

    ////METODOS CON GUERRERO----------------------------------------------------------------
    method atravesarZonaGuerrero(guerrero) {
        if (!self.estaFueraDeCombate(guerrero)){ //no sufren efecto de zona los que ya estan fuera de combate 
            self.aplicarEfecto(guerrero)
            if (self.estaFueraDeCombate(guerrero)){ //chequea si alguno quedo fuera de combate despues del efecto
                throw new GuerreroFueraDeCombate(message = "Guerrero fuera de Combate")
            }
        }
        
    }
    method estaFueraDeCombate(guerrero) = guerrero.estaFueraDeCombate()
    method aplicarEfecto(guerrero)
    //---------------------------------------------------------------------------------------
}

object belfalas inherits Zona{
     override method aplicarEfecto(guerrero){
        guerrero.agregarItem("Lembas")
    }
}
object lebennin inherits Zona{
    override method cumpleRequerimientoDeGuerrero(grupo) = 
        grupo.guerrerosEnCombate().any{ guerrero => guerrero.poder() == 1500}
    //se eliminan a los guerreros que estan fuera de combate, y despues se analiza si se cumple la condicion (en este caso que alguno tenga poder 1500)
    //se filtran para que no sean tomados en cuenta para las restricciones
    override method aplicarEfecto(guerrero){
        //no pasa nada
    }

    method puedeAtravesar(guerrero) = guerrero.poder() > 1500
    method atravesar(guerrero) {
        // Lebennin no afecta al guerrero, no pasa nada
    }
}

object minasTirith inherits Zona {
    override method cumpleRequerimientoItem(grupo) = grupo.cantidadDeItems("Lembas") == 10 // El grupo debe tener 10 Lembas.

    override method aplicarEfecto(guerrero){
        guerrero.modificarCantidadDeVida(-15)
    }

    method puedeAtravesar(guerrero) = guerrero.armas() != []
    method atravesar(guerrero) {
        guerrero.modificarCantidadDeVida(-10)
    }
}

object bosqueDeFangorn inherits Zona {
    override method cumpleRequerimientoDeGuerrero(grupo) = 
        grupo.guerrerosEnCombate().any{ guerrero => !guerrero.armas().isEmpty()}
    //se eliminan a los guerreros que estan fuera de combate, y despues se analiza si se cumple la condicion (en este caso que alguno tenga armas)

    override method aplicarEfecto(guerrero){
        guerrero.removerItem("capa elfica")
    }
}

object edoras inherits Zona {   
    override method aplicarEfecto(guerrero){
        guerrero.agregarItem("vino caliente")
    }
}

object estemnet inherits Zona {
    override method cumpleRequerimientoItem(grupo) = grupo.cantidadDeItems("capa elfica") == 3

    override method aplicarEfecto(guerrero){
        guerrero.vidaActual(guerrero.vidaActual()*2)
    }
}

/*------------------ -------- ------------------*/
/*------------------ PARTE 3 -------------------*/
/*------------------ -------- ------------------*/

/*------------------ Complejizacion de zonas ------------------*/

//Se modificaron cosas arriba de "Parte 3"

const gondor = new Region(zonas = [lamedon,belfalas,lebennin,minasTirith],nombre = "Gondor")
const rohan = new Region(zonas = [bosqueDeFangorn,edoras,estemnet], nombre = "Rohan")
const caminito =  new Camino(zonas = [bosqueDeFangorn, edoras, lamedon, belfalas, lebennin, minasTirith])
const regionesExistentes = [gondor,rohan]

object lamedon inherits Zona{
    override method aplicarEfecto(guerrero){
        guerrero.vidaActual(guerrero.vidaActual() * 1.30) //incrementa su vida en un 30%
    }
}

object losPradosDeParthGalen inherits Zona{
    override method aplicarEfecto(guerrero){
        guerrero.modificarCantidadDeVida(-21)
    }
}

/*------------------ Más sobre caminos ------------------*/

 // Se resolvió en la clase Camino y en la clase Zona