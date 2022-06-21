Software de manejo del sistema MAPESYNTH
--

En esta carpeta se incluye todo el sw necesario para hacer funcionar el sistema, así como varios ejemplos de manejo del mismo. Cada ejemplo se incluye en un fichero .c 

Los siguentes ficheros son necesarios para el funcionamiento de los ejemplos:

- **mapesynth.c / mapesynth.h**: librería de funciones de manejo del sistema. Incluye las funciones de bajo nivel (HAL) y las funciones de manejo en alto nivel, así como algunas funciones de manejo de la pantalla.
- **formas.h / formas.c**: formas de onda digitalizadas, y definiciones de constantes para calcular las fases al muestrear las formas con las frecuencias de las notas de la escala occidental temperada (LA=440Hz). Solo será necesario si se va a usar el sistema como sintetizador
- **nokia5110.c / nokia5110.h**: librería de funciones de manejo de la pantalla. Basada en la existente en la página 43oh.com


Los ejemplos disponibles son los siguientes:

- **MIDI_CTRL1.c**: El sistema funciona como un secuenciador, reproduciendo canciones almacenadas. Se puede elegir la canción, cambiar la velocidad y seleccionar el canal MIDI al que se mande la secuencia.
- **MIDI_CTRL2.c**:El sistema funciona como un controlador MIDI interpuesto entre un teclado y un sintetizador. Cuando recibe una nota, la retransmite, pudiendo modificar la nota, el valor de la velocidad y el pitch. También podrá incluir mensajes de Pedal Sustain On/Off.
- **MIDI_SYNTH1.c**: Funciona como sintetizador aditivo básico, sumando cualquiera de las formas almacenadas, pudiendo variar en el momento de la suma su amplitud y fase
- **MIDI_SYNTH2.C**: versión del anterior, incorporando la modulación ADSR. Se pueden variar los parámetros de tiempos de ataque, decay, release y valor de Sustain.  
- **MIDI_SYNTH3.C**: Versión del anterior, incorporando la posibilidad de grabar formas de onda ya mezcladas y recuperarlas más adelante. Hace uso de la memoria FRAM del dispositivo, y cambia ligeramente el uso de los vectores de forma de onda.
