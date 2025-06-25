% ----------------------- PUNTO 1 ----------------------- %

% jugador: nombre, civilizacion, tecnologias
jugador(ana, romanos, [herreria, forja, emplumado, laminas]).
jugador(beto, incas, [herreria, forja, fundicion]).
jugador(carola, romanos, [herreria]).
jugador(dimitri, romanos, [herreria, fundicion]).
% Se indica en la consigna que Elsa no juega esta partida, por lo tanto no se la agrega como jugadora
% dado que por el principio de universo cerrado, el motor toma como falso todo lo que no es verdadero. 

% ----------------------- PUNTO 2 ----------------------- %

esExpertoEnMetales(Jugador):-
    jugador(Jugador,_,Tecnologias),
    member(herreria, Tecnologias),
    member(forja, Tecnologias).

esExpertoEnMetales(Jugador):-
    jugador(Jugador,_,Tecnologias),
    member(fundicion, Tecnologias).

esExpertoEnMetales(Jugador):-
    jugador(Jugador,romanos,_).


% ---------------- PUNTO 3 - INTEGRANTE 1 ---------------- %

esPopular(Civilizacion):-
    jugador(Jugador,Civilizacion,_),
    jugador(OtroJugador,Civilizacion,_),
    Jugador\=OtroJugador.


% ---------------- PUNTO 4 - INTEGRANTE 2 ---------------- %

tecnologia(Tecnologia):- jugador(_,_,Tecnologias), member(Tecnologia,Tecnologias).

tieneAlcanceGlobal(Tecnologia):-
    tecnologia(Tecnologia), %unifico Tecnologia con el universo de tecnologias
    forall(jugador(_,_,Tecnologias),member(Tecnologia,Tecnologias)). %Se cumple si para todas las tecnologias del jugador, la Tecnologia pertenece a ellas

/*
"Tecnologia" esta siendo unificada/ligada con cada una de las tecnologias existentes en el juego:
primero obteniendo la lista de las disponibles (las relacionadas con Jugadores), y luego diciendoleque esta es una
que pertenece a la lista de Tecnologias existentes de los jugadores (a traves de member). No es valido ligar
Tecnologia haciendo "jugador(_,_,Tecnologia)" porque la estaria ligando con una lista, y no con cada una de
ellas, por eso es necesario el member
*/

% ---------------- PUNTO 5 - INTEGRANTE 3 ---------------- %

esCivilizacionLider(Civilizacion):-
    jugador(_,Civilizacion,_),
    findall(Tecnologias,jugador(_,Civilizacion,Tecnologias),TecnologiasCivilizacionConRepetidos),
    findall(Tecnologias,jugador(_,_,Tecnologias),TecnologiasJugadoresConRepetidos),
    flatten(TecnologiasJugadoresConRepetidos,TecnologiasJugadores),
    list_to_set(TecnologiasJugadores,TecnologiasJugadoresSinRep),
    flatten(TecnologiasCivilizacionConRepetidos,TecnologiasCivilizacion),
    list_to_set(TecnologiasCivilizacion,TecnologiasCivilizacionSinRep),
    length(TecnologiasCivilizacionSinRep,CantidadTecnologiasCivilizacion),
    length(TecnologiasJugadoresSinRep,CantidadTecnologiasJugadoresSinRep),
    CantidadTecnologiasCivilizacion = CantidadTecnologiasJugadoresSinRep.

/*
% Alternativa
% Una civilización es líder cuando se cumple que esa civilización alcanzó todas las tecnologías que alcanzaron las demás
esCivilizacionLider2(CivilizacionLider):-
    jugador(_,CivilizacionLider,_), %generador para inversibilidad
    forall(tecnologia(Tecnologia),civilizAlcanzoTecnolog(CivilizacionLider,Tecnologia)). % que todas las tecnologias sean alcanzadas por la civilizacion
    %not((tecnologia(Tecnologia),not(civilizAlcanzoTecnolog(CivilizacionLider,Tecnologia)))). % lo mismo que la 82 (No hay tegnologia que no haya sido alcanzada por la civilizacion)

% Una civilización alcanzó una tecnología cuando algún jugador de esa civilización la desarrolló.
civilizAlcanzoTecnolog(Civilizacion,Tecnologia):-
    jugador(_,Civilizacion,Tecnologias), % obtengo las tecnologias de esta civilizacion
    member(Tecnologia,Tecnologias). % me fijo que cada tecnologia alcanzada por otros esté dentro de las tecnologias de la civilizacion
*/

% ----------------------- PUNTO 6 ----------------------- %

campeon(Vida):- Vida >= 1 , Vida =< 100.
% unidades: jugador, unidad
unidades(ana, jineteACaballo).
unidades(ana, (piqueroConEscudo, 1)). %piquero: piquero,nivel
unidades(ana, (piqueroSinEscudo, 2)).
unidades(beto, campeon(100)). %campeon: campeon(vida)
unidades(beto, campeon(80)).
unidades(beto, (piqueroConEscudo, 1)).
unidades(beto, jineteACamello).
unidades(carola, (piqueroSinEscudo, 3)).
unidades(carola, (piqueroConEscudo, 2)).


% ---------------- PUNTO 7 - INTEGRANTE 3 ---------------- %

% unidad,vida
vidaDeUnidad(campeon(Vida),Vida).
vidaDeUnidad(jineteACaballo,90).
vidaDeUnidad(jineteACamello,80).
vidaDeUnidad((piqueroSinEscudo,1),50).
vidaDeUnidad((piqueroSinEscudo,2),65).
vidaDeUnidad((piqueroSinEscudo,3),70).
vidaDeUnidad((piqueroConEscudo,Nivel), Vida) :-
    vidaDeUnidad((piqueroSinEscudo,Nivel), VidaSinEscudo),
    Vida is VidaSinEscudo * 1.1. % 10% más de vida con escudo
/*
vidaDeUnidad((piqueroConEscudo,1),Vida):- vidaDeUnidad((piqueroSinEscudo,1),VidaSinEscudo), Vida is VidaSinEscudo*1.1.
vidaDeUnidad((piqueroConEscudo,2),Vida):- vidaDeUnidad((piqueroSinEscudo,2),VidaSinEscudo), Vida is VidaSinEscudo*1.1.
vidaDeUnidad((piqueroConEscudo,3),Vida):- vidaDeUnidad((piqueroSinEscudo,3),VidaSinEscudo), Vida is VidaSinEscudo*1.1.
*/

unidadConMasVida(Jugador,UnidadConMasVida):-
    unidades(Jugador,_),
    findall(Unidad,(unidades(Jugador,Unidad),vidaDeUnidad(Unidad,_)),Unidades), %lista de unidades del jugador
    findall(Vida,(unidades(Jugador,Unidad),vidaDeUnidad(Unidad,Vida)),Vidas), %lista de vidas de las unidades del jugador
    max_member(MayorVida,Vidas), %mayor de las vidas de la lista de vidas
    nth1(Posicion,Vidas,MayorVida), %posicion de la mayor vida en la lista de vidas
    nth1(Posicion,Unidades,UnidadConMasVida). %nombre de la unidad que esta en la posicion de mas vida

/* % No anda, probando para examen
unidadConMasVida2(Jugador,UnidadConMasVida):-
    unidades(Jugador,_),
    findall(vidaDeUnidad(Unidad,Vida),(unidades(Jugador,Unidad),vidaDeUnidad(Unidad,Vida)),UnidadesConVida). %lista de unidades del jugador
    mayorVida(UnidadesConVida,UnidadConMasVida). %mayor de las vidas de la lista de vidaDeUnidad
*/

/*
//Notar como cambia el comportamiento al poner unidades(Jugador,Unidad) antes o dentro del findall
//Al ponerlo antes, ligo Unidad a un unico valor, por lo que obtengo listas de un elemento
//Al ponerlo dentro, Unidad se liga con todas las unidades posibles del jugador
unidades(Jugador,Unidades):-
    unidades(Jugador,Unidad),
    findall(Unidad,(vidaDeUnidad(Unidad,_)),Unidades).
*/
/*
unidades(Jugador,Unidades):-
    findall(Unidad,(unidades(Jugador,Unidad),vidaDeUnidad(Unidad,_)),Unidades).
*/

% ---------------- PUNTO 8 - INTEGRANTE 2 ---------------- %

unidadLeGanaAOtra(Unidad,campeon(_)):-
    member(Unidad,[jineteACaballo,jineteACamello]).
unidadLeGanaAOtra(campeon(_),OtraUnidad):-
    member(OtraUnidad,[(piqueroSinEscudo, _),(piqueroConEscudo, _)]).
unidadLeGanaAOtra(Unidad,OtraUnidad):-
    member(Unidad,[(piqueroSinEscudo, _),(piqueroConEscudo, _)]),
    member(OtraUnidad,[jineteACaballo,jineteACamello]).
unidadLeGanaAOtra(jineteACamello,jineteACaballo).
unidadLeGanaAOtra(Unidad,OtraUnidad):-
    member(Unidad,[(piqueroSinEscudo, _),(piqueroConEscudo, _)]),
    member(OtraUnidad,[(piqueroSinEscudo, _),(piqueroConEscudo, _)]),
    vidaDeUnidad(Unidad,VidaUnidad),
    vidaDeUnidad(OtraUnidad,VidaOtraUnidad),
    VidaUnidad > VidaOtraUnidad.
unidadLeGanaAOtra(campeon(VidaUnidad),campeon(VidaOtraUnidad)):-
    between(1,100,VidaUnidad),
    between(1,100,VidaOtraUnidad),
    VidaUnidad > VidaOtraUnidad.

/* Pensando
unidadLeGanaAOtra(Unidad,OtraUnidad):-
    vidaDeUnidad(Unidad,VidaUnidad), vidaDeUnidad(OtraUnidad,VidaOtraUnidad),
    nombre(Unidad,NombreUnidad), nombre(OtraUnidad,NombreOtraUnidad),
    NombreUnidad = NombreOtraUnidad,   
    VidaUnidad > VidaOtraUnidad.
*/

% ---------------- PUNTO 9 - INTEGRANTE 1 ---------------- %

sobreviveAsedio(Jugador):-
    unidades(Jugador,_),
    cantidadDeUnidades(Jugador, piqueroSinEscudo, CantidadSinEscudo),
    cantidadDeUnidades(Jugador, piqueroConEscudo, CantidadConEscudo),
    CantidadConEscudo > CantidadSinEscudo.

cantidadDeUnidades(Jugador, Unidad, Cantidad):-
    findall(
        Unidad,
        unidades(Jugador, (Unidad,_)),
        Unidades
    ),
    length(Unidades, Cantidad).


% ----------------------- PUNTO 10a ----------------------- %

% arbolDeTecnologias: tecnologia, requisitos
arbolDeTecnologias(arado, [collera, molino]).
arbolDeTecnologias(collera, [molino]).
arbolDeTecnologias(placas, [malla, laminas, herreria]).
arbolDeTecnologias(malla, [laminas, herreria]).
arbolDeTecnologias(laminas, [herreria]).
arbolDeTecnologias(horno, [fundicion, forja, herreria]).
arbolDeTecnologias(fundicion, [forja, herreria]).
arbolDeTecnologias(forja, [herreria]).
arbolDeTecnologias(punzon, [emplumado, herreria]).
arbolDeTecnologias(emplumado, [herreria]).
arbolDeTecnologias(herreria,[]).
arbolDeTecnologias(molino,[]).


% ----------------------- PUNTO 10b ----------------------- %
/* %Mal hecho
puedeDesarrollar(Jugador, TecnologiaADesarrollar):-
    jugador(Jugador,_,Tecnologias),
    arbolDeTecnologias(TecnologiaADesarrollar, Requisitos),
    forall(
        (not(member(TecnologiaADesarrollar, Tecnologias)), member(Requisito, Requisitos)),
        member(Requisito, Tecnologias)
    ). */
puedeDesarrollar(Jugador, TecnologiaADesarrollar):-
    jugador(Jugador,_,Tecnologias),
    arbolDeTecnologias(TecnologiaADesarrollar, Requisitos),
    not(member(TecnologiaADesarrollar, Tecnologias)),
    forall(
        ( member(Requisito, Requisitos)),
        member(Requisito, Tecnologias)
    ).


% ----------------------- PUNTO 11a ----------------------- %

ordenValido(Jugador,OrdenValido):-
    jugador(Jugador,_,Tecnologias),
    listaDependencias(Tecnologias,ListaDependencias),
    listaDependencias(OrdenValidos,ListaDependencias),
    list_to_set(OrdenValidos,OrdenValido),
    length(OrdenValido,LongitudOrdenValido),
    length(Tecnologias,LongitudTecnologias),
    LongitudOrdenValido = LongitudTecnologias,
    miembrosPertenecen(OrdenValido,Tecnologias),
    flatten(ListaDependencias,ListaDependenciasAplanada),
    dependenciasValida(ListaDependenciasAplanada,Tecnologias).

listaDependencias([],[]).
listaDependencias([Tecnologia|Tecnologias],[Dependencia|Dependencias]):-
    arbolDeTecnologias(Tecnologia,Dependencia),
    listaDependencias(Tecnologias,Dependencias).

miembrosPertenecen([Cabeza|Cola],Lista):-
    member(Cabeza,Lista),
    miembrosPertenecen(Cola,Lista).
miembrosPertenecen([],_).

dependenciasValida([Dependencia|Dependencias],ListaTecnologias):-
    member(Dependencia,ListaTecnologias),
    miembrosPertenecen(Dependencias,ListaTecnologias). %por que no uso directamente miembrosPertenecen?

% ----------------------- PUNTO 11b ----------------------- %

%El resultado da false, porque no existe un orden valido para las tecnologias que tiene desarrolladas ya que deberia haber desarrollado forja antes de desarrollar fundicion.
% ----------------------- PUNTO 12 ----------------------- %

/* Dado un jugador defensor, encontrar el ejército que debo crear para ganarle a todo su ejército. El ejército atacante debe tener el mismo 
   tamaño, y suponer que las batallas son uno contra uno, cada integrante atacante ataca a un integrante defensor. */
ejercitoParaGanarleA(JugadorDefensor, UnidadesVencedoras):-
    %jugador(JugadorDefensor,_,_), %No conviene porque está dimitri y no tiene unidades
    unidades(JugadorDefensor,_),
    findall(UnidadDefensor, unidades(JugadorDefensor, UnidadDefensor), UnidadesDefensor), % Obtener las unidades del jugador defensor
    maplist(unidadLeGanaAOtra,UnidadesVencedoras,UnidadesDefensor). %obtener tales unidades que venzan a las unidades del defensor, en una lista
    
/* ANTIGUA VERSION
ejercitoParaGanarleA2(JugadorDefensor, UnidadesVencedoras):-
    findall(UnidadJuego, vidaDeUnidad(UnidadJuego, _), UnidadesJuego), % Obtener todas las unidades del juego
    findall(UnidadDefensor, unidades(JugadorDefensor, UnidadDefensor), UnidadesDefensor), % Obtener las unidades del jugador defensor
    length(UnidadesDefensor, TamanioRequerido), % Tamaño del ejército defensor
    length(UnidadesVencedoras, TamanioRequerido), % Tamaño del ejército atacante
    generarCombinacion(UnidadesJuego, TamanioRequerido, UnidadesVencedoras), % Generar combinaciones de unidades atacantes
    cumpleCondiciones(UnidadesVencedoras, UnidadesDefensor). % Verificar que el ejército atacante pueda vencer al defensor

% Generar combinaciones de unidades atacantes con el mismo tamaño que el ejército defensor
generarCombinacion(_, 0, []).
generarCombinacion(UnidadesJuego, N, [Unidad|Resto]):-
    N > 0,
    member(Unidad, UnidadesJuego), % Seleccionar una unidad del juego
    N1 is N - 1,
    generarCombinacion(UnidadesJuego, N1, Resto).

% Verificar que cada unidad atacante pueda vencer al menos a una unidad defensora
cumpleCondiciones([], _).
cumpleCondiciones([UnidadAtacante|RestoAtacantes], UnidadesDefensor):-
    member(UnidadDefensor, UnidadesDefensor),
    unidadLeGanaAOtra(UnidadAtacante, UnidadDefensor),
    cumpleCondiciones(RestoAtacantes, UnidadesDefensor).
*/
