/*
  ____          _____               _ _           _       
 |  _ \        |  __ \             (_) |         | |      
 | |_) |_   _  | |__) |_ _ _ __ _____| |__  _   _| |_ ___ 
 |  _ <| | | | |  ___/ _` | '__|_  / | '_ \| | | | __/ _ \
 | |_) | |_| | | |  | (_| | |   / /| | |_) | |_| | ||  __/
 |____/ \__, | |_|   \__,_|_|  /___|_|_.__/ \__, |\__\___|
         __/ |                               __/ |        
        |___/                               |___/         
    
____________________________________
/ Si necesitas ayuda, contáctame en \
\ https://parzibyte.me               /
 ------------------------------------
        \   ^__^
         \  (oo)\_______
            (__)\       )\/\
                ||----w |
                ||     ||
Creado por Parzibyte (https://parzibyte.me).
------------------------------------------------------------------------------------------------
Si el código es útil para ti, puedes agradecerme siguiéndome: https://parzibyte.me/blog/sigueme/
Y compartiendo mi blog con tus amigos
También tengo canal de YouTube: https://www.youtube.com/channel/UCroP4BTWjfM0CkGB6AFUoBg?sub_confirmation=1
------------------------------------------------------------------------------------------------
*/
#include <ESP8266WiFiMulti.h>
#include <ESP8266WebServer.h>
#include "DHT.h"
#include <Arduino.h>

#define PIN_CONEXION_DHT D1
#define TIPO_SENSOR DHT22
#define LED_DE_ESTADO 2

ESP8266WiFiMulti wifiMulti;
ESP8266WebServer servidor(80);
DHT sensor(PIN_CONEXION_DHT, TIPO_SENSOR);

int ultimaVezLeido = 0;
long intervaloLectura = 5000; // Debería ser mayor que 2000
unsigned long ultimaLecturaExitosa = 0;

float humedad, temperatura = 0;

// Prototipos de funciones
void rutaRaiz();
void rutaNoEncontrada();
void rutaJson();
void indicarErrorDht();
void indicarExitoDht();

void setup(void)
{
  pinMode(LED_DE_ESTADO, OUTPUT);
  digitalWrite(LED_DE_ESTADO, LOW);

  sensor.begin();
  // Aquí puedes agregar varias redes. La tarjeta se conectará a la más cercana
  wifiMulti.addAP("RED", "CONTRASEÑA");
  // wifiMulti.addAP("Otra red", "Contraseña");

  // Esperar conexión WiFi
  while (wifiMulti.run() != WL_CONNECTED)
  {
    delay(250);
  }

  //  Configurar rutas
  servidor.on("/", rutaRaiz);
  servidor.on("/api", rutaJson);
  servidor.onNotFound(rutaNoEncontrada);

  // Iniciar servidor
  servidor.begin();
  digitalWrite(LED_DE_ESTADO, HIGH);
}

void loop(void)
{
  // Si el intervalo se ha alcanzado, leer la temperatura
  if (ultimaVezLeido > intervaloLectura)
  {
    float nuevaHumedad = sensor.readHumidity();
    float nuevaTemperatura = sensor.readTemperature();
    // Si los datos son correctos, actualizamos las globales
    if (isnan(nuevaTemperatura) || isnan(nuevaHumedad))
    {
      indicarErrorDht();
      ultimaVezLeido = 0;
      return;
    }
    ultimaLecturaExitosa = millis();
    humedad = nuevaHumedad;
    temperatura = nuevaTemperatura;
    ultimaVezLeido = 0;
    indicarExitoDht();
  }
  delay(1);
  ultimaVezLeido += 1;
  // Responder las solicitudes entrantes en caso de que haya
  servidor.handleClient();
}

// Servir toda la página web, desde la misma se consultará a la API que está aquí mismo
void rutaRaiz()
{
  servidor.send(200, "text/html", "<!DOCTYPE html><html lang='es'><head> <meta charset='UTF-8'> <meta name='viewport' content='width=device-width, initial-scale=1.0'> <title>Sensor de temperatura - By Parzibyte</title> <link rel='stylesheet' href='https://unpkg.com/bulma@0.9.1/css/bulma.min.css'> <link rel='stylesheet' href='https://stackpath.bootstrapcdn.com/font-awesome/4.7.0/css/font-awesome.min.css'></head><body> <section id='app' class='hero is-link is-fullheight'> <div class='hero-body'> <div class='container'> <div class='columns has-text-centered'> <div class='column'> <h1 style='font-size: 2.5rem'>Termómetro</h1> <i class='fa' :class='claseTermometro' style='font-size: 4rem;'></i> </div></div><div class='columns'> <div class='column has-text-centered'> <h2 class='is-size-4 has-text-warning'>Temperatura</h2> <h2 class='is-size-1'>{{temperatura}}°C</h2> </div><div class='column has-text-centered'> <h2 class='is-size-4 has-text-warning'>Humedad</h2> <h2 class='is-size-1'>{{humedad}}%</h2> </div></div><div class='columns'> <div class='column'> <p>Última lectura: Hace <strong class='has-text-white'>{{ultimaLectura}}</strong> segundo(s)</p><p class='is-size-5'><i class='fa fa-code'></i> con <i class='fa fa-heart has-text-danger'></i> por <a target='_blank' class='has-text-warning' href='https://parzibyte.me/blog'>Parzibyte</a></p></div></div></div></div></section> <script src='https://unpkg.com/vue@2.6.12/dist/vue.min.js'> </script> <script>const INTERVALO_REFRESCO=5000; new Vue({el: '#app', data: ()=> ({ultimaLectura: 0, temperatura: 0, humedad: 0,}), mounted(){this.refrescarDatos();}, methods:{async refrescarDatos(){try{const respuestaRaw=await fetch('./api'); const datos=await respuestaRaw.json(); this.ultimaLectura=datos.u; this.temperatura=datos.t; this.humedad=datos.h; setTimeout(()=>{this.refrescarDatos();}, INTERVALO_REFRESCO);}catch (e){setTimeout(()=>{this.refrescarDatos();}, INTERVALO_REFRESCO);}}}, computed:{claseTermometro(){if (this.temperatura <=5){return 'fa-thermometer-empty';}else if (this.temperatura > 5 && this.temperatura <=13){return 'fa-thermometer-quarter';}else if (this.temperatura > 13 && this.temperatura <=21){return 'fa-thermometer-half';}else if (this.temperatura > 21 && this.temperatura <=30){return 'fa-thermometer-three-quarters';}else{return 'fa-thermometer-full';}}}}); </script></body></html>");
}

// Manejador de 404
void rutaNoEncontrada()
{
  servidor.send(404, "text/plain", "No encontrado");
}

// Nuestra pequeña API
void rutaJson()
{
  // Calcular última lectura exitosa en segundos
  unsigned long tiempoTranscurridoEnMilisegundos = millis() - ultimaLecturaExitosa;
  int tiempoTranscurrido = tiempoTranscurridoEnMilisegundos / 1000;
  // Búfer para escribir datos en JSON
  char bufer[50];
  // Crear la respuesta pasando las variables globales
  // La salida será algo como:
  // {"t":14.20,"h":79.20,"l":5.00}
  sprintf(bufer, "{\"t\":%.2f,\"h\":%.2f,\"u\":%d}", temperatura, humedad, tiempoTranscurrido);
  // Responder con ese JSON
  servidor.send(200, "application/json", bufer);
}

/*
 Patrones para parpadear LED
*/
void indicarErrorDht()
{
  int x = 0;
  for (x = 0; x < 5; x++)
  {
    digitalWrite(LED_DE_ESTADO, LOW);
    delay(50);
    digitalWrite(LED_DE_ESTADO, HIGH);
    delay(50);
  }
}

void indicarExitoDht()
{
  digitalWrite(LED_DE_ESTADO, LOW);
  delay(50);
  digitalWrite(LED_DE_ESTADO, HIGH);
}
