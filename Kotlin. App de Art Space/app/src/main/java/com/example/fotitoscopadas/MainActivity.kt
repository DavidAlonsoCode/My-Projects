package com.example.fotitoscopadas

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.compose.foundation.BorderStroke
import androidx.compose.foundation.Image
import androidx.compose.foundation.ScrollState
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.aspectRatio
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.safeDrawingPadding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.wrapContentSize
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.Button
import androidx.compose.material3.ButtonColors
import androidx.compose.material3.ButtonDefaults
import androidx.compose.material3.CenterAlignedTopAppBar
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.tooling.preview.Preview
import com.example.fotitoscopadas.ui.theme.FotitosCopadasTheme
import androidx.compose.material3.TopAppBarDefaults.topAppBarColors
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.runtime.saveable.rememberSaveable
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.draw.blur
import androidx.compose.ui.draw.shadow
import androidx.compose.ui.graphics.Brush
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.Shape
import androidx.compose.ui.graphics.fromColorLong
import androidx.compose.ui.layout.Layout
import androidx.compose.ui.modifier.modifierLocalConsumer
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.TextStyle
import androidx.compose.ui.text.font.FontStyle
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import kotlinx.coroutines.launch
import org.w3c.dom.Text

@OptIn(ExperimentalMaterial3Api::class)
class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()
        setContent {
            FotitosCopadasTheme {
                Scaffold(
                    topBar = {
                        CenterAlignedTopAppBar(
                            colors = topAppBarColors(
                                containerColor = Color(0xFFFFC8BA),
                                titleContentColor = Color.Black
                            ),
                            title = {
                                Text(
                                    text = "Maravillas mundiales"
                                )
                            }
                        )
                    }
                ) { innerPadding ->
                    DisposicionGeneral(
                        modifier = Modifier.padding(innerPadding)
                    )
                }
            }
        }
    }
}

// EL MODELO DE DATOS
data class MaravillaData(
    val titulo: Int,
    val imagen: Int,
    val descripcion: Int
)

// LA "BASE DE DATOS" (Hardcoded por ahora)
fun obtenerMaravillas(): List<MaravillaData> = listOf(
        MaravillaData(R.string.titulo_bahia_de_halong, R.drawable.bahia_de_halong, R.string.descrip_bahia_de_halong),
        MaravillaData(R.string.titulo_isla_jeju, R.drawable.isla_jeju, R.string.descrip_isla_jeju),
        MaravillaData(R.string.titulo_table_mountain, R.drawable.table_mountain, R.string.descrip_table_mountain),
        MaravillaData(R.string.titulo_isla_de_comodo, R.drawable.isla_de_komodo, R.string.descrip_isla_de_comodo),
        MaravillaData(R.string.titulo_mi_novia, R.drawable.mi_novia, R.string.descrip_mi_novia)
    )

@Preview(showBackground = true)
@Composable
fun DisposicionGeneral(
    modifier: Modifier = Modifier
) {
    //Logica del programa
    val maravillas = remember { obtenerMaravillas() }
    var indiceActual by rememberSaveable { mutableStateOf(0) } //indices de 0 a 4 (o sea son 5)

    // Guardo el estado del scroll en una variable para poder controlarlo
    val estadoDelScroll = rememberScrollState()
    // Creo un "lanzador" de tareas para poder animar el scroll
    val coroutineScope = rememberCoroutineScope()

    fun irAlSiguiente() {
        if(indiceActual < maravillas.lastIndex) indiceActual++ else indiceActual = 0
        coroutineScope.launch { estadoDelScroll.scrollTo(0) }
    }

    val irAlAnterior = {
        if(indiceActual > 0) indiceActual-- else indiceActual = maravillas.lastIndex
        coroutineScope.launch { estadoDelScroll.scrollTo(0) }
    }

    PantallaContenido(
        maravilla = maravillas[indiceActual],
        textoPagina = "${indiceActual+1}/${maravillas.size}",
        scrollState = estadoDelScroll,
        onSiguiente = ::irAlSiguiente, //dos maneras distintas de hacerlo
        onAnterior = { irAlAnterior() },
        modifier = modifier
    )

}

@Composable
fun PantallaContenido(
    maravilla: MaravillaData,     // Recibe DATOS ya procesados
    textoPagina: String,          // Recibe TEXTO ya formateado
    scrollState: ScrollState,     // Recibe el estado del scroll
    onSiguiente: () -> Unit,      // Recibe EVENTOS
    onAnterior: () -> Unit,
    modifier: Modifier = Modifier
){
    Column(
        verticalArrangement = Arrangement.SpaceBetween, //arrangement siempre es a lo largo del objeto
        horizontalAlignment = Alignment.CenterHorizontally,
        modifier = modifier
            .fillMaxSize()
            .verticalScroll(scrollState)
            .safeDrawingPadding()
    ) {
        Text(
            text = stringResource(maravilla.titulo),
            fontStyle = FontStyle.Italic,
            fontSize = 24.sp,
            fontWeight = FontWeight.ExtraBold,
            modifier = Modifier.padding(vertical = 24.dp)
        )
        Image(
            painter = painterResource(maravilla.imagen),
            contentDescription = null,
            modifier = Modifier
                .fillMaxWidth()
                .padding(horizontal = 12.dp)
                .shadow(
                    elevation = 30.dp, // tiene sombra pero se nota poco
                    shape = RoundedCornerShape(16.dp)
                )
                .aspectRatio(1f)
        )
        Surface( //no hacia falta. Podia usar ".background"
            color = Color(0xFFFFC8BA),
            modifier = Modifier
                .padding(all = 24.dp)
                .shadow(
                    elevation = 1.dp,
                    shape = RoundedCornerShape(24.dp)
                )
        ) {
            Text(
                text = stringResource(maravilla.descripcion),
                fontStyle = FontStyle.Italic,
                fontSize = 16.sp,
                color = Color.Black,
                modifier = Modifier.padding(all = 20.dp)
            )
        }
        Text(
            text = textoPagina,
            color = Color.Black,
            modifier = Modifier
                .padding(bottom = 20.dp)
                .background(
                    color = Color(0xFFFFC8BA),
                    shape = RoundedCornerShape(16.dp))
                .padding(horizontal = 12.dp, vertical = 4.dp)
        )
        Row(){
            ConfigurarButton(
                onClick = onAnterior,
                texto = "Anterior",
                modifier = Modifier.padding(bottom = 20.dp, start = 20.dp)
            )
            Spacer(modifier = Modifier.weight(1f))
            ConfigurarButton(
                onClick = onSiguiente,
                texto = "Siguiente",
                modifier = Modifier.padding(bottom = 20.dp, end = 20.dp)
            )
        }
    }
}

@Composable
fun ConfigurarButton(
    onClick: () -> Unit,
    texto: String,
    modifier: Modifier = Modifier
){
    Button(
        onClick = onClick,
        colors = ButtonDefaults.buttonColors(
            containerColor = Color(0xFFFFC8BA)
        ),
        modifier = modifier
    ) {
        Text(
            text = texto,
            fontSize = 20.sp
        )
    }
}

/*
@Preview(showBackground = true)
@Composable
fun FotitosCopadasPreview() {
    FotitosCopadasTheme {
        DisposicionGeneral()
    }
}*/

//MODELO VIEJO (dentro de DisposicionGeneral)
// Sin lista ni data class, ni lambdas "irAl...", ni "PantallaContenido"
/*
var diapositivaActual by rememberSaveable { mutableStateOf(1) }
var tituloActual: Int
var imagenActual: Int
var descripcionActual: Int

when (diapositivaActual) {
    1 -> {
        tituloActual = R.string.titulo_bahia_de_halong
        imagenActual = R.drawable.bahia_de_halong
        descripcionActual = R.string.descrip_bahia_de_halong
    }
    2 -> {
        tituloActual = R.string.titulo_isla_jeju
        imagenActual = R.drawable.isla_jeju
        descripcionActual = R.string.descrip_isla_jeju
    }
    3 -> {
        tituloActual = R.string.titulo_table_mountain
        imagenActual = R.drawable.table_mountain
        descripcionActual = R.string.descrip_table_mountain
    }
    4 -> {
        tituloActual = R.string.titulo_isla_de_comodo
        imagenActual = R.drawable.isla_de_komodo
        descripcionActual = R.string.descrip_isla_de_comodo
    }
    else -> {
        tituloActual = R.string.titulo_mi_novia
        imagenActual = R.drawable.mi_novia
        descripcionActual = R.string.descrip_mi_novia
    }
}
*/