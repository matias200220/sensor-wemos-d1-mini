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

