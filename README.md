# CerraduraConRitmo

Se trata de integrar un sistema de detección de golpes con un piezoeléctrico para crear un ritmo que actúe como llave, cuando la puerta se golpea con el ritmo correcto, se activa un solenoide que mueve la cerradura abriéndola.
El microcontrolador tiene un botón que activa el modo grabación, tras pulsar, tenemos un máximo de 10 segundos para empezar a grabar el ritmo clave, una vez el ritmo sea correcto, se guarda en la memoria interna y será utilizado como clave de entrada. En caso de no golpear, la memoria interna será borrada, eliminando cualquier clave.
El funcionamiento se basa en guardar el tiempo entre golpes en un vector que será la clave, con cada golpe se mide el tiempo entre el pulso actual y el anterior, comparándolo con el vector clave y dándolo como válido si se encuentra dentro de unos márgenes de error.

Proyecto basado en [Secret Knock Detecting Door Lock](https://www.instructables.com/id/Secret-Knock-Detecting-Door-Lock/) de Grathio 
