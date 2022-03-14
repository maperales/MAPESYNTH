# MAPESYNTH
Documentación y ficheros de diseño y programación del sistema Mapesynth
-
En este repositorio se encuentra toda la información necesaria para poder replicar el sistema MAPESYNTH completo.

El sistema MAPESYNTH (Midi Advanced Programmable Educational Synthesizer) es un dispositivo que, convenientemente programado, puede hacer de instrumento MIDI, controlador MIDI, secuenciador MIDI y sintetizador MIDI, o cualquier combinación de las anteriores. Al ser de código abierto, puede ser reprogramado para cumplir otras funciones.

![image](https://user-images.githubusercontent.com/6535003/158177157-f62d671b-8c2c-4f03-8f4c-e0b264b439fa.png)



El hardware tiene la estructura de un Boosterpack tipo XL (40 pines), que se puede acomodar sobre multitud de Launchpads de Texas Instruments. En concreto, los ejemplos software y el diseño están enfocados al uso del microcontrolador MSP430FR5994, por lo que será necesario disponer del Launchpad MSP-EXP430FR5994 para su manejo. Los ficheros de diseño se encuentran en Eagle, aunque la etapa final se ha realizado en EasyEDA. Se ofrece igualmente el fichero de EasyEDA, por si sólo se desea fabricar tal cual, sin ningún cambio.

El software se ha desarrollado en Code Composer Studio, disponible de manera gratuita en la página de Texas Instruments. Probablemente se pueda reutilizar con algunos cambios menores con otros compiladores de la familia MSP430, aunque no se ha probado.


