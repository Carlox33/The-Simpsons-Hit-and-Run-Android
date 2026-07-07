# 📝 Changelog - Simpsons Hit & Run Port for Android

🌍 **Available languages**

[🇪🇸 Español](#-español) | [🇬🇧 English](#-english) | [🇫🇷 Français](#-français) | [🇩🇪 Deutsch](#-deutsch)

---

# 🇪🇸 Español

## Versión 0.91.0.1 - Navegación táctil en el libro de estadísticas

La versión **0.91.0.1** añade la posibilidad de navegar de forma correcta  en la pantalla del libro de estadísticas por nivel, permitiendo cambiar entre niveles pulsando la mitad derecha o izquierda de la pantalla.

Ahora, al pulsar la **mitad derecha**, se muestran las estadísticas del siguiente nivel, mientras que al pulsar la **mitad izquierda**, se muestran las estadísticas del nivel anterior.

Esto anteriormente no estaba disponible, ya que este menú utiliza los botones L1/LT y R1/RT para moverse en el menú.

## Versión 0.91 - Minijuego partida extra habilitado
La versión **0.91** habilita el minijuego partida extra, permitiendo disfrutarlo teniendo un layuout táctil dentro de este minijuego adaptado para el mismo.

## Versión 0.90 - Cinemáticas corregidas y 60 FPS estable

La versión **0.90** es una actualización importante centrada en mejorar el rendimiento general y la fidelidad del port Android respecto al comportamiento original del juego.

Esta versión introduce un nuevo sistema de limitación estable a **60 FPS**, corrige problemas visuales durante las cinemáticas y mejora el comportamiento del juego en varios apartados relacionados con la sincronización interna y la carga del mundo 3D.

---

## ⚡ Nuevo límite estable a 60 FPS

Una de las mejoras principales de esta versión es la implementación de un límite de **60 FPS** mucho más estable y preciso.

En versiones anteriores, debido a una pequeña imprecisión en el cálculo de los FPS, el juego podía ejecutarse aproximadamente a **62 FPS** en lugar de mantenerse exactamente en **60 FPS**.

Aunque esta diferencia pudiera parecer pequeña, afectaba a varias partes internas del juego, ya que el motor original está muy ligado al ritmo de actualización consiguiendo de esta forma  una experiencia más estable y fiel al comportamiento del juego en plataformas originales.

---

## 🚀 Mejoras de rendimiento y estabilidad

Gracias al nuevo sistema de limitación estable a **60 FPS**, esta versión también incluye mejoras importantes de rendimiento y estabilidad.

Entre las mejoras conseguidas se encuentran:

* ✔ La pantalla de carga al entrar al mundo 3D ahora es más rápida.
* ✔ El juego mantiene un ritmo de ejecución más estable.
* ✔ Se reducen pequeñas irregularidades provocadas por una tasa de FPS incorrecta.
* ✔ El comportamiento interno del juego es más fiel a la versión original.
* ✔ La física del juego se comporta de forma  correcta y estable.
* ✔ Se reduce el exceso de brillo que estaba presente en versiones anteriores.

---

## 🎬 Cinemáticas corregidas

En esta versión también se han corregido los problemas visuales relacionados con las cinemáticas.

Anteriormente, algunas cinemáticas podían mostrar restos del frame anterior o provocar pequeños fallos visuales al comenzar la reproducción.

En la versión **0.90**, el juego limpia correctamente el frame anterior antes de mostrar la cinemática, evitando que se arrastren imágenes previas en pantalla.

Además, se han añadido barras negras laterales durante la reproducción, mejorando la presentación visual de las cinemáticas y evitando problemas de escalado o visualización en Android.

Esta corrección se ha implementado de forma segura utilizando las propias herramientas gráficas del motor del juego.


---

## ✨ Resumen de la versión 0.90

* ✔ Nuevo límite estable y preciso a **60 FPS**.
* ✔ Corregido el problema por el que el juego podía ejecutarse aproximadamente a **62 FPS**.
* ✔ Mejorada la estabilidad general del juego.
* ✔ Mejoras de rendimiento relacionadas con la sincronización interna del motor.
* ✔ La carga al mundo 3D ahora es más rápida.
* ✔ Física del juego más fiel al comportamiento original.
* ✔ Reducido el exceso de brillo presente en versiones anteriores.
* ✔ Cinemáticas corregidas.
* ✔ Limpieza correcta del frame anterior antes de reproducir una cinemática.
* ✔ Añadidas barras negras laterales durante las cinemáticas.

##  Versión 0.87.4

La versión **0.87.4** es una actualización centrada en mejorar la experiencia de juego en Android, especialmente para los usuarios que juegan con **controles táctiles**, y en seguir puliendo la convivencia entre controles táctiles, mando externo y opciones de configuración dentro del juego.

Esta actualización incluye cambios importantes relacionados con el **minimapa**, la configuración de vibración del mando, la inversión de cámara y varias optimizaciones internas del sistema táctil/gamepad.

---

## 🗺️ Minimapa desplazado al usar controles táctiles

Una modificación **super importante** de esta actualización es la **implementación de un sistema para mover el minimapa del juego** cuando el usuario juega con controles táctiles.

En versiones anteriores, la posición original del minimapa podía interferir visualmente con algunos iconos de los controles táctiles, haciendo que ciertas zonas de la pantalla se sintieran más cargadas o menos cómodas durante la partida.

En la versión **0.87.4**, el minimapa se desplaza hacia una posición superior derecha de la pantalla cuando se utilizan controles táctiles, ofreciendo una mejor experiencia visual y evitando que los iconos táctiles interfieran con el minimapa.

---

## ⚙️ Opciones de vibración e inversión de cámara dentro del juego

En esta versión se ha añadido la posibilidad de activar o desactivar dentro del propio juego algunas opciones importantes del port:

* Activar o desactivar la **vibración del mando**.
* Activar o desactivar la opción de **invertir la cámara**.

Para conseguir esto, el usuario debe tener el archivo
```txt
 Android/data/com.c4rlox.simpsons/files/art/frontend/scrooby/ingame.p3d mofidicado
```
En concreto la pantalla del menu de **Settings** de la versión de consolas, ya que la versión original de PC de 2003 no incluía estas dos opciones dentro de su menú de configuración.

Esto permite que el usuario pueda modificar estos ajustes de una forma más cómoda, sin depender únicamente de cambios manuales antes de iniciar el juego.

En caso de que el usuario no tenga aplicada esta modificación mencionada del menu settings del archivo ingame.p3d podrá realizar el cambio antes de arrancar el juego modificando este archivo.
```txt
Android/data/com.c4rlox.simpsons/files/Simpsons_configuration/Simpsons_configuration.txt
```

---

## 🎮 Respeto de la posición original del minimapa al usar mando externo

El sistema ha sido diseñado para respetar al máximo la experiencia original del juego de 2003.

Por este motivo, si el usuario conecta un **mando externo**, el minimapa vuelve a su posición original.

Esto significa que el minimapa solo se desplaza cuando realmente es necesario, es decir, cuando el jugador está usando controles táctiles.

De esta forma, la versión Android adapta la interfaz cuando se juega con pantalla táctil, pero conserva la posición original del HUD al completo cuando se juega con mando externo.

---

## 📱 Ajuste del botón Start táctil

El botón **Start táctil** ha sido movido a la zona izquierda de la pantalla.

Gracias a este ajuste, la distribución de los controles táctiles queda mejor integrada con el nuevo layout del minimapa.

---



## 🎮 Optimización de los controles táctiles

Esta versión incluye optimizaciones internas en el sistema de controles táctiles.

Aunque probablemente el usuario final no perciba de forma directa la mayoría de estas optimizaciones, son importantes para mantener una arquitectura más limpia, más estable y más eficiente.
---

## 🎮 Optimización del sistema táctil/gamepad

También se ha optimizado el sistema encargado de gestionar la convivencia entre **controles táctiles** y **mando externo**.


Cuando el usuario juega con mando, el sistema puede evitar trabajo innecesario relacionado con los controles táctiles. Esto ayuda a que la lógica interna sea más eficiente y evita que ambos sistemas compitan entre sí.

---

## 🔌 Mejoras al conectar y desconectar mando in-game

Se ha optimizado el sistema de conexión y desconexión de mandos externos dentro del juego.

Esta mejora afecta especialmente a los casos en los que el usuario conecta o desconecta un mando mientras ya está dentro de la partida.

En esta versión, el paso de **modo gamepad** a **modo controles táctiles** al desconectar el mando es más rápido que en versiones anteriores.

Esto permite que el jugador pueda volver a usar los controles táctiles con menos espera después de desconectar el mando externo.

---

## ⚡ Mejoras pensadas para dispositivos más limitados

Cabe mencionar que estas optimizaciones, salvo la mejora relacionada con la rapidez al desconectar el mando y volver al modo táctil, probablemente no sean muy visibles para el usuario final en dispositivos potentes.

Sin embargo, son cambios importantes para seguir puliendo el port poco a poco.

La idea es que pequeñas optimizaciones acumuladas puedan acabar beneficiando especialmente a dispositivos más limitados, como algunas **Smart TV**, teléfonos antiguos o dispositivos Android con menos potencia.

Aunque cada mejora por separado pueda parecer pequeña, mantener el sistema cada vez más limpio, estable y eficiente ayuda a mejorar la calidad general del port a largo plazo.

---

## ✨ Resumen de la versión 0.87.4

* ✔ Añadida la posibilidad de activar o desactivar la vibración del mando desde el juego.
* ✔ Añadida la posibilidad de activar o desactivar la inversión de cámara desde el juego.
* ✔ **Minimapa desplazado** cuando se utilizan controles táctiles. [**Controles táctiles**](TOUCH_CONTROLS.md)
* ✔ El minimapa vuelve a su posición original cuando se juega con mando externo.
* ✔ Movido el botón **Start táctil** a la zona izquierda para no interferir con el minimapa.
* ✔ Movido ligeramente a la derecha el botón de **freno** en vehículo.
* ✔ Aumentado el tamaño del botón de **freno** en vehículo.
* ✔ Optimizados los controles táctiles.
* ✔ Optimizado el sistema táctil/gamepad cuando el usuario juega con mando externo.
* ✔ Optimizado el sistema de conexión y desconexión de mando in-game.
* ✔ Mayor rapidez al pasar de modo gamepad a modo controles táctiles al desconectar el mando dentro del juego.
* ✔ Mejoras internas pensadas para seguir puliendo el rendimiento en dispositivos Android más limitados.

##  Versión 0.87.2

La versión **0.87.2** es una actualización centrada en mejorar la comodidad de los controles táctiles, añadir nuevas opciones de configuración y corregir algunos comportamientos no deseados detectados tras la versión 0.87.

Aunque se trata de una actualización menor, incluye cambios importantes para hacer que la experiencia de juego en Android sea más cómoda, precisa y configurable.

---

## 🎮 Mejoras en el joystick virtual a pie

Se ha mejorado el comportamiento del **joystick virtual cuando el personaje va a pie**.

En versiones anteriores, el joystick táctil tenía una zona muerta demasiado grande, lo que podía hacer que el movimiento del personaje se sintiera menos preciso o algo incómodo en determinadas situaciones.

En esta versión se ha reducido esa zona muerta excesiva, consiguiendo que el control a pie sea más cómodo, más sensible y más preciso al jugar directamente desde la pantalla táctil.

---

## 🎮 Mejoras en la detección entre mando externo y controles táctiles

Se ha mejorado el sistema de detección del mando externo, tardando ahora menos tiempo en ser reconocido y mapeado, por lo tanto el usuario al conectar el mando a mitad de partida puede empezar a usarlo unos segunos antes que anteriormente en otras versiones.

Esto ha sido posible por una mejora en todo el sistema que se encarga de detectar si existe mando externo o no, para de esa forma 
decidir si mostramos controles táctiles en pantalla o por si lo contrario desabilitamos los controles táctiles y le damos prioridad al mando externo.

Esta mejora en la arquitectura también produjo que al iniciar partida con mando no tengamos que ver los controles táctiles hasta que pulsemos por primera vez un botón como ocurría antes.

Se corrigió un bug que existía de si entrabamos al juego con el mando conectado y despues tocabamos, los controles táctiles aparecían y respondían a los inputs táctiles, esto era no deseado cuando ya existe un mando conectado.

---

## ⚙️ Nueva opción de configuración en la pantalla de pausa

Se ha añadido una nueva opción de **configuración** dentro de la pantalla de pausa in-game.

Desde esta nueva sección, el usuario puede modificar algunos ajustes del port a su gusto sin necesidad de salir completamente de la experiencia de juego.

Esto permite que la versión de Android sea más flexible y se adapte mejor a las preferencias de cada jugador.

---

## 📝 Configuración adicional mediante archivo externo

Debido a que el usuario aporta los assets de la versión original de PC, y dicha versión de 2003 no incluía opciones de menú para activar o desactivar la vibración ni para invertir la cámara, estos ajustes se han añadido mediante un archivo de configuración externo.

El archivo se encuentra en la siguiente ruta:

```txt
Android/data/com.c4rlox.simpsons/files/Simpsons_configuration/Simpsons_configuration.txt
```

Desde este archivo, antes de iniciar el juego, el usuario puede elegir:

* Activar o desactivar la vibración del gamepad.
* Invertir o no la cámara en el eje X.

De esta forma, se añaden opciones modernas de configuración respetando las limitaciones de los assets originales de la versión de PC.

---

## 📳 Corrección de vibración no deseada del teléfono

Se ha corregido un bug que provocaba que el teléfono vibrase en algunos dispositivos.

Este comportamiento no era intencionado ni formaba parte de ninguna función prevista del port. La vibración del teléfono era un efecto no deseado que solo ocurría en ciertos dispositivos, por lo que ha sido eliminada.

La vibración configurable se mantiene únicamente para mandos compatibles mediante la opción correspondiente del archivo de configuración.

---

## ✨ Resumen de la versión 0.87.2

* Mejorado el joystick virtual a pie.
* Reducida la zona muerta excesiva del joystick táctil.
* Movimiento a pie más cómodo y preciso.
* Añadida opción de configuración en la pantalla de pausa in-game.
* Añadido archivo de configuración externo para opciones no disponibles en los assets originales de PC.
* Añadida opción para activar o desactivar la vibración del gamepad.
* Añadida opción para invertir la cámara en el eje X.
* Corregido el bug que provocaba vibración no deseada del teléfono en algunos dispositivos.
* Corregido el bug de estar habilitado controles táctiles y mando externo cuando el mando externo era conectado antes de entrar al juego.

---
##  Versión 0.87

La versión **0.87** representa una de las actualizaciones más importantes del port de *The Simpsons: Hit & Run* para Android hasta la fecha.

Esta versión está centrada principalmente en mejorar la experiencia de juego en dispositivos móviles, haciendo que el port sea más cómodo, más accesible y mucho más disfrutable para todos aquellos jugadores que quieran jugar directamente desde la pantalla táctil.

---

## 🎮 Controles táctiles totalmente integrados

La gran novedad de esta versión es la incorporación de **controles táctiles completamente integrados con el sistema original del juego**.

No se trata de una simple capa de botones colocada por encima del juego. Los controles táctiles han sido integrados respetando la arquitectura interna del motor original, utilizando el propio sistema de input del juego y las herramientas que el motor utilizaba para dibujar elementos en pantalla.

Esto permite que la experiencia sea mucho más limpia, fluida y coherente, manteniendo el mejor rendimiento posible y consiguiendo que los controles táctiles se sientan como una parte real del juego.

Los controles táctiles son inteligentes y pueden adaptarse al contexto del jugador, cambiando automáticamente entre layouts de menú, a pie y en vehículo. Además, el port puede detectar si existe un mando físico conectado, mostrando u ocultando los controles táctiles según sea necesario.

Para conocer todos los detalles sobre esta nueva arquitectura, los iconos, los layouts y el archivo de compatibilidad táctil, puedes visitar:

* 📱 [Controles táctiles](TOUCH_CONTROLS.md)

---

## 📱 Mejor soporte de pantalla completa

También se ha añadido una mejora importante relacionada con el modo **pantalla completa**.

En algunos dispositivos Android, el fullscreen no se aplicaba correctamente, lo que podía provocar que el juego no aprovechara toda la pantalla o que aparecieran barras del sistema de forma no deseada.

Con esta actualización se ha mejorado el soporte de pantalla completa para ofrecer una experiencia más inmersiva, limpia y agradable en aquellos dispositivos donde antes no funcionaba correctamente.

---

## ✨ Resumen de la versión 0.87

* Añadidos controles táctiles totalmente integrados con el sistema original del juego.
* Añadidos layouts inteligentes para menú, personaje a pie y vehículo.
* Añadida detección de mando físico para mostrar u ocultar automáticamente los controles táctiles.
* Añadidos iconos táctiles personalizados.
* Añadido archivo de compatibilidad táctil para dispositivos con coordenadas táctiles normalizadas de forma diferente.
* Mejorado el soporte de pantalla completa en dispositivos donde no se aplicaba correctamente.

La versión **0.87** es un paso enorme para hacer que el port sea mucho más cómodo y natural en Android. Es una actualización pensada para que jugar en móvil se sienta mejor, más integrado y mucho más cercano a una experiencia nativa.

---

# 🇬🇧 English

## Version 0.91.0.1 - Touch Navigation in the Statistics Book

Version **0.91.0.1** adds the ability to navigate correctly on the level statistics book screen, allowing you to switch between levels by tapping the right or left half of the screen.

Now, tapping the **right half** displays the statistics for the next level, while tapping the **left half** displays the statistics for the previous level.

## Version 0.91 - Extra match mini-game enabled
Version **0.91** enables the extra match mini-game, allowing you to enjoy it with a touch layout adapted for this mini-game.

## Version 0.90 — Fixed cutscenes and stable 60 FPS

Version **0.90** is an important update focused on improving the overall performance and fidelity of the Android port compared to the original behavior of the game.

This version introduces a new stable **60 FPS** limiting system, fixes visual issues during cutscenes, and improves the game’s behavior in several areas related to internal synchronization and 3D world loading.

---

## ⚡ New stable 60 FPS limit

One of the main improvements in this version is the implementation of a much more stable and precise **60 FPS** limit.

In previous versions, due to a small inaccuracy in the FPS calculation, the game could run at approximately **62 FPS** instead of staying exactly at **60 FPS**.

Although this difference may seem small, it affected several internal parts of the game, since the original engine is closely tied to the update rate, resulting in a more stable experience that is more faithful to the behavior of the game on the original platforms.

---

## 🚀 Performance and stability improvements

Thanks to the new stable **60 FPS** limiting system, this version also includes important performance and stability improvements.

The improvements achieved include:

* ✔ The loading screen when entering the 3D world is now faster.
* ✔ The game maintains a more stable execution rhythm.
* ✔ Small irregularities caused by an incorrect FPS rate have been reduced.
* ✔ The internal behavior of the game is more faithful to the original version.
* ✔ The game physics now behave correctly and consistently.
* ✔ The excessive brightness present in previous versions has been reduced.

---

## 🎬 Fixed cutscenes

This version also fixes the visual issues related to cutscenes.

Previously, some cutscenes could show remnants of the previous frame or cause small visual glitches when playback started.

In version **0.90**, the game correctly clears the previous frame before displaying the cutscene, preventing previous images from being dragged onto the screen.

In addition, black side bars have been added during playback, improving the visual presentation of the cutscenes and preventing scaling or display issues on Android.

This fix has been safely implemented using the game engine’s own graphics tools.

---

## ✨ Version 0.90 summary

* ✔ New stable and precise **60 FPS** limit.
* ✔ Fixed the issue where the game could run at approximately **62 FPS**.
* ✔ Improved overall game stability.
* ✔ Performance improvements related to the engine’s internal synchronization.
* ✔ Loading into the 3D world is now faster.
* ✔ Game physics are more faithful to the original behavior.
* ✔ Reduced the excessive brightness present in previous versions.
* ✔ Fixed cutscenes.
* ✔ Correct clearing of the previous frame before playing a cutscene.
* ✔ Added black side bars during cutscenes.



##  Version 0.87.4

Version **0.87.4** is an update focused on improving the gameplay experience on Android, especially for users who play with **touch controls**, while continuing to polish the coexistence between touch controls, external gamepads and in-game configuration options.

This update includes important changes related to the **minimap**, gamepad vibration settings, camera inversion and several internal optimizations to the touch/gamepad system.

---

## 🗺️ Minimap moved when using touch controls

A **very important** modification in this update is the **implementation of a system to move the in-game minimap** when the user plays with touch controls.

In previous versions, the original position of the minimap could visually interfere with some touch control icons, making certain areas of the screen feel more crowded or less comfortable during gameplay.

In version **0.87.4**, the minimap is moved to the upper-right area of the screen when touch controls are used, providing a better visual experience and preventing the touch icons from interfering with the minimap.

---

## ⚙️ Gamepad vibration and camera inversion options inside the game

In this version, the ability to enable or disable some important port options directly inside the game has been added:

* Enable or disable **gamepad vibration**.
* Enable or disable the option to **invert the camera**.

To achieve this, the user must have the following file modified:

```txt
Android/data/com.c4rlox.simpsons/files/art/frontend/scrooby/ingame.p3d
```

More specifically, the **Settings** menu screen from the console version must be used, since the original 2003 PC version did not include these two options inside its configuration menu.

This allows the user to modify these settings in a more convenient way, without relying only on manual changes before starting the game.

If the user does not have this mentioned modification applied to the Settings menu inside the ingame.p3d file, the change can still be made before launching the game by modifying this file:

```txt
Android/data/com.c4rlox.simpsons/files/Simpsons_configuration/Simpsons_configuration.txt
```

---

## 🎮 Respecting the original minimap position when using an external gamepad

The system has been designed to respect the original 2003 game experience as much as possible.

For this reason, if the user connects an **external gamepad**, the minimap returns to its original position.

This means that the minimap is only moved when it is actually necessary, that is, when the player is using touch controls.

In this way, the Android version adapts the interface when playing with the touchscreen, but preserves the full original HUD position when playing with an external gamepad.

---

## 📱 Touch Start button adjustment

The **touch Start** button has been moved to the left side of the screen.

Thanks to this adjustment, the touch control layout is better integrated with the new minimap layout.

---

## 🎮 Touch control optimization

This version includes internal optimizations to the touch control system.

Although the final user will probably not directly notice most of these optimizations, they are important for keeping the architecture cleaner, more stable and more efficient.

---

## 🎮 Touch/gamepad system optimization

The system responsible for managing the coexistence between **touch controls** and an **external gamepad** has also been optimized.

When the user plays with a gamepad, the system can avoid unnecessary work related to touch controls. This helps make the internal logic more efficient and prevents both systems from competing with each other.

---

## 🔌 Improvements when connecting and disconnecting a gamepad in-game

The system for connecting and disconnecting external gamepads inside the game has been optimized.

This improvement especially affects cases where the user connects or disconnects a gamepad while already inside the game.

In this version, switching from **gamepad mode** to **touch controls mode** after disconnecting the gamepad is faster than in previous versions.

This allows the player to use the touch controls again with less waiting time after disconnecting the external gamepad.

---

## ⚡ Improvements designed for more limited devices

It is worth mentioning that these optimizations, except for the improvement related to faster switching back to touch mode after disconnecting the gamepad, will probably not be very noticeable for the final user on powerful devices.

However, they are important changes to keep polishing the port little by little.

The idea is that accumulated small optimizations may eventually benefit more limited devices, such as some **Smart TVs**, older phones or less powerful Android devices.

Even if each improvement may seem small on its own, keeping the system cleaner, more stable and more efficient helps improve the overall quality of the port in the long term.

---

## ✨ Version 0.87.4 summary

* ✔ Added the ability to enable or disable gamepad vibration from inside the game.
* ✔ Added the ability to enable or disable camera inversion from inside the game.
* ✔ **Minimap moved** when using touch controls. [**Touch controls**](TOUCH_CONTROLS.md)
* ✔ The minimap returns to its original position when playing with an external gamepad.
* ✔ Moved the **touch Start** button to the left side to avoid interfering with the minimap.
* ✔ Moved the **brake** button in vehicles slightly to the right.
* ✔ Increased the size of the **brake** button in vehicles.
* ✔ Optimized touch controls.
* ✔ Optimized the touch/gamepad system when the user plays with an external gamepad.
* ✔ Optimized the in-game gamepad connection and disconnection system.
* ✔ Faster switching from gamepad mode to touch controls mode after disconnecting the gamepad inside the game.
* ✔ Internal improvements designed to continue polishing performance on more limited Android devices.



##  Version 0.87.2

Version **0.87.2** is an update focused on improving the comfort of the touch controls, adding new configuration options, and fixing some unwanted behaviors detected after version 0.87.

Although this is a minor update, it includes important changes to make the Android gameplay experience more comfortable, precise, and configurable.

---

## 🎮 Improvements to the Virtual Joystick on Foot

The behavior of the **virtual joystick when the character is on foot** has been improved.

In previous versions, the touch joystick had an excessively large dead zone, which could make character movement feel less precise or somewhat uncomfortable in certain situations.

In this version, that excessive dead zone has been reduced, making on-foot controls more comfortable, more responsive, and more precise when playing directly from the touch screen.

---

## 🎮 Improvements to Detection Between External Gamepad and Touch Controls

The external gamepad detection system has been improved, allowing the controller to be recognized and mapped more quickly. As a result, when the user connects a gamepad during gameplay, it can now be used a few seconds earlier than in previous versions.

This was made possible by improving the entire system responsible for detecting whether an external gamepad is available. Based on that detection, the game decides whether to show the touch controls on screen or disable them and give priority to the external gamepad.

This architectural improvement also prevents the touch controls from appearing when starting the game with a gamepad already connected, instead of waiting until the user presses a gamepad button for the first time as happened before.

A bug has also been fixed where, if the player entered the game with a gamepad already connected and then touched the screen, the touch controls would appear and respond to touch input. This behavior was not intended when an external gamepad was already available.

---

## ⚙️ New Configuration Option in the Pause Screen

A new **configuration** option has been added inside the in-game pause screen.

From this new section, users can modify some port settings according to their preferences without having to completely leave the gameplay experience.

This makes the Android version more flexible and better adapted to each player’s preferences.

---

## 📝 Additional Configuration Through an External File

Because the user provides the assets from the original PC version, and that 2003 version did not include menu options to enable or disable vibration or invert the camera, these settings have been added through an external configuration file.

The file is located at the following path:

```txt
Android/data/com.c4rlox.simpsons/files/Simpsons_configuration/Simpsons_configuration.txt
```

From this file, before launching the game, the user can choose:

* Enable or disable gamepad vibration.
* Invert or not invert the camera on the X axis.

This allows modern configuration options to be added while respecting the limitations of the original PC version assets.

---

## 📳 Fix for Unwanted Phone Vibration

A bug has been fixed that caused the phone itself to vibrate on some devices.

This behavior was not intentional and was not part of any planned feature of the port. The phone vibration was an unwanted effect that only occurred on certain devices, so it has now been removed.

Configurable vibration remains available only for compatible gamepads through the corresponding option in the configuration file.

---

## ✨ Version 0.87.2 Summary

* Improved the virtual joystick when playing on foot.
* Reduced the excessive dead zone of the touch joystick.
* Made on-foot movement more comfortable and precise.
* Added a configuration option in the in-game pause screen.
* Added an external configuration file for options not available in the original PC assets.
* Added an option to enable or disable gamepad vibration.
* Added an option to invert the camera on the X axis.
* Fixed a bug that caused unwanted phone vibration on some devices.
* Fixed a bug where touch controls and an external gamepad could both remain active when the gamepad was connected before entering the game.



##  Version 0.87

Version **0.87** represents one of the most important updates so far for the Android port of *The Simpsons: Hit & Run*.

This version mainly focuses on improving the gameplay experience on mobile devices, making the port more comfortable, more accessible and much more enjoyable for players who want to play directly using the touchscreen.

---

## 🎮 Fully integrated touch controls

The biggest new feature in this version is the addition of **touch controls fully integrated with the original game system**.

This is not just a simple layer of buttons placed on top of the game. The touch controls have been integrated while respecting the internal architecture of the original engine, using the game’s own input system and the tools the engine used to draw elements on screen.

This allows the experience to feel much cleaner, smoother and more coherent, while maintaining the best possible performance and making the touch controls feel like a real part of the game.

The touch controls are intelligent and can adapt to the player’s context, automatically switching between menu, on-foot and vehicle layouts. The port can also detect whether a physical controller is connected, showing or hiding the touch controls when needed.

For full details about this new architecture, the icons, the layouts and the touch compatibility file, please visit:

* 📱 [Touch controls](TOUCH_CONTROLS.md)

---

## 📱 Improved fullscreen support

An important improvement has also been added regarding **fullscreen mode**.

On some Android devices, fullscreen was not applied correctly, which could cause the game to not use the entire screen or make system bars appear when they should not.

With this update, fullscreen support has been improved to provide a more immersive, cleaner and more pleasant experience on devices where it was not working correctly before.

---

## ✨ Version 0.87 summary

* Added fully integrated touch controls using the original game system.
* Added intelligent layouts for menu, on-foot gameplay and vehicle gameplay.
* Added physical controller detection to automatically show or hide the touch controls.
* Added custom touch icons.
* Added a touch compatibility file for devices that normalize touch coordinates differently.
* Improved fullscreen support on devices where it was not applied correctly.

Version **0.87** is a huge step toward making the port feel much more comfortable and natural on Android. It is an update designed to make playing on mobile feel better, more integrated and much closer to a native experience.

---

# 🇫🇷 Français

## Version 0.91.0.1 - Navigation tactile dans le livre des statistiques

La version **0.91.0.1** ajoute la possibilité de naviguer correctement dans l'écran du livre des statistiques par niveau, permettant de passer d'un niveau à l'autre en appuyant sur la moitié droite ou gauche de l'écran.

Désormais, en appuyant sur la **moitié droite**, les statistiques du niveau suivant s'affichent, tandis qu'en appuyant sur la **moitié gauche**, les statistiques du niveau précédent s'affichent.

## Version 0.91 - Mini-jeu partie supplémentaire activé
La version **0.91** active le mini-jeu partie supplémentaire, permettant d’en profiter avec une disposition tactile adaptée à ce mini-jeu.

## Version 0.90 — Cinématiques corrigées et 60 FPS stables

La version **0.90** est une mise à jour importante axée sur l’amélioration des performances générales et de la fidélité du portage Android par rapport au comportement original du jeu.

Cette version introduit un nouveau système de limitation stable à **60 FPS**, corrige les problèmes visuels pendant les cinématiques et améliore le comportement du jeu dans plusieurs domaines liés à la synchronisation interne et au chargement du monde 3D.

---

## ⚡ Nouvelle limite stable à 60 FPS

L’une des principales améliorations de cette version est la mise en place d’une limite de **60 FPS** beaucoup plus stable et précise.

Dans les versions précédentes, en raison d’une légère imprécision dans le calcul des FPS, le jeu pouvait fonctionner à environ **62 FPS** au lieu de rester exactement à **60 FPS**.

Même si cette différence peut sembler faible, elle affectait plusieurs parties internes du jeu, car le moteur original est fortement lié au rythme de mise à jour, permettant ainsi d’obtenir une expérience plus stable et plus fidèle au comportement du jeu sur les plateformes originales.

---

## 🚀 Améliorations des performances et de la stabilité

Grâce au nouveau système de limitation stable à **60 FPS**, cette version inclut également d’importantes améliorations des performances et de la stabilité.

Parmi les améliorations obtenues, on trouve :

* ✔ L’écran de chargement lors de l’entrée dans le monde 3D est maintenant plus rapide.
* ✔ Le jeu maintient un rythme d’exécution plus stable.
* ✔ Les petites irrégularités provoquées par un taux de FPS incorrect ont été réduites.
* ✔ Le comportement interne du jeu est plus fidèle à la version originale.
* ✔ La physique du jeu se comporte désormais de manière correcte et stable.
* ✔ L’excès de luminosité présent dans les versions précédentes a été réduit.

---

## 🎬 Cinématiques corrigées

Dans cette version, les problèmes visuels liés aux cinématiques ont également été corrigés.

Auparavant, certaines cinématiques pouvaient afficher des restes de l’image précédente ou provoquer de petits défauts visuels au début de la lecture.

Dans la version **0.90**, le jeu efface correctement l’image précédente avant d’afficher la cinématique, évitant ainsi que des images précédentes restent visibles à l’écran.

De plus, des barres noires latérales ont été ajoutées pendant la lecture, améliorant la présentation visuelle des cinématiques et évitant les problèmes de mise à l’échelle ou d’affichage sur Android.

Cette correction a été implémentée de manière sûre en utilisant les propres outils graphiques du moteur du jeu.

---

## ✨ Résumé de la version 0.90

* ✔ Nouvelle limite stable et précise à **60 FPS**.
* ✔ Correction du problème qui pouvait faire fonctionner le jeu à environ **62 FPS**.
* ✔ Amélioration de la stabilité générale du jeu.
* ✔ Améliorations des performances liées à la synchronisation interne du moteur.
* ✔ Le chargement vers le monde 3D est maintenant plus rapide.
* ✔ Physique du jeu plus fidèle au comportement original.
* ✔ Réduction de l’excès de luminosité présent dans les versions précédentes.
* ✔ Cinématiques corrigées.
* ✔ Effacement correct de l’image précédente avant la lecture d’une cinématique.
* ✔ Ajout de barres noires latérales pendant les cinématiques.


##  Version 0.87.4

La version **0.87.4** est une mise à jour centrée sur l’amélioration de l’expérience de jeu sur Android, en particulier pour les utilisateurs qui jouent avec les **commandes tactiles**, tout en continuant à améliorer la coexistence entre les commandes tactiles, les manettes externes et les options de configuration dans le jeu.

Cette mise à jour inclut des changements importants liés à la **mini-carte**, à la configuration de la vibration de la manette, à l’inversion de la caméra et à plusieurs optimisations internes du système tactile/manette.

---

## 🗺️ Mini-carte déplacée lors de l’utilisation des commandes tactiles

Une modification **très importante** de cette mise à jour est l’**implémentation d’un système permettant de déplacer la mini-carte du jeu** lorsque l’utilisateur joue avec les commandes tactiles.

Dans les versions précédentes, la position d’origine de la mini-carte pouvait interférer visuellement avec certaines icônes des commandes tactiles, ce qui rendait certaines zones de l’écran plus chargées ou moins confortables pendant la partie.

Dans la version **0.87.4**, la mini-carte est déplacée vers la zone supérieure droite de l’écran lorsque les commandes tactiles sont utilisées, offrant ainsi une meilleure expérience visuelle et empêchant les icônes tactiles d’interférer avec la mini-carte.

---

## ⚙️ Options de vibration de la manette et d’inversion de la caméra dans le jeu

Dans cette version, la possibilité d’activer ou de désactiver directement dans le jeu certaines options importantes du port a été ajoutée :

* Activer ou désactiver la **vibration de la manette**.
* Activer ou désactiver l’option permettant d’**inverser la caméra**.

Pour cela, l’utilisateur doit disposer du fichier suivant modifié :

```txt
Android/data/com.c4rlox.simpsons/files/art/frontend/scrooby/ingame.p3d
```

Plus précisément, il faut utiliser l’écran du menu **Settings** de la version console, car la version PC originale de 2003 n’incluait pas ces deux options dans son menu de configuration.

Cela permet à l’utilisateur de modifier ces paramètres de manière plus confortable, sans dépendre uniquement de modifications manuelles avant de lancer le jeu.

Si l’utilisateur n’a pas appliqué cette modification mentionnée du menu Settings dans le fichier ingame.p3d, le changement peut toujours être effectué avant de lancer le jeu en modifiant ce fichier :

```txt
Android/data/com.c4rlox.simpsons/files/Simpsons_configuration/Simpsons_configuration.txt
```

---

## 🎮 Respect de la position originale de la mini-carte lors de l’utilisation d’une manette externe

Le système a été conçu pour respecter autant que possible l’expérience originale du jeu de 2003.

Pour cette raison, si l’utilisateur connecte une **manette externe**, la mini-carte revient à sa position d’origine.

Cela signifie que la mini-carte n’est déplacée que lorsque cela est réellement nécessaire, c’est-à-dire lorsque le joueur utilise les commandes tactiles.

Ainsi, la version Android adapte l’interface lorsque l’on joue avec l’écran tactile, mais conserve la position complète du HUD original lorsque l’on joue avec une manette externe.

---

## 📱 Ajustement du bouton Start tactile

Le bouton **Start tactile** a été déplacé vers la partie gauche de l’écran.

Grâce à cet ajustement, la disposition des commandes tactiles s’intègre mieux au nouveau layout de la mini-carte.

---

## 🎮 Optimisation des commandes tactiles

Cette version inclut des optimisations internes du système de commandes tactiles.

Même si l’utilisateur final ne remarquera probablement pas directement la plupart de ces optimisations, elles sont importantes pour maintenir une architecture plus propre, plus stable et plus efficace.

---

## 🎮 Optimisation du système tactile/manette

Le système chargé de gérer la coexistence entre les **commandes tactiles** et une **manette externe** a également été optimisé.

Lorsque l’utilisateur joue avec une manette, le système peut éviter du travail inutile lié aux commandes tactiles. Cela permet de rendre la logique interne plus efficace et d’éviter que les deux systèmes entrent en concurrence.

---

## 🔌 Améliorations lors de la connexion et de la déconnexion d’une manette en jeu

Le système de connexion et de déconnexion des manettes externes dans le jeu a été optimisé.

Cette amélioration concerne surtout les cas où l’utilisateur connecte ou déconnecte une manette alors qu’il est déjà dans la partie.

Dans cette version, le passage du **mode manette** au **mode commandes tactiles** après la déconnexion de la manette est plus rapide que dans les versions précédentes.

Cela permet au joueur de réutiliser les commandes tactiles avec moins d’attente après avoir déconnecté la manette externe.

---

## ⚡ Améliorations pensées pour les appareils plus limités

Il convient de mentionner que ces optimisations, à l’exception de l’amélioration liée au retour plus rapide au mode tactile après la déconnexion de la manette, ne seront probablement pas très visibles pour l’utilisateur final sur des appareils puissants.

Cependant, ce sont des changements importants pour continuer à améliorer le port petit à petit.

L’idée est que l’accumulation de petites optimisations puisse finir par bénéficier particulièrement aux appareils plus limités, comme certaines **Smart TV**, les téléphones anciens ou les appareils Android moins puissants.

Même si chaque amélioration peut sembler petite séparément, maintenir un système de plus en plus propre, stable et efficace aide à améliorer la qualité générale du port sur le long terme.

---

## ✨ Résumé de la version 0.87.4

* ✔ Ajout de la possibilité d’activer ou de désactiver la vibration de la manette depuis le jeu.
* ✔ Ajout de la possibilité d’activer ou de désactiver l’inversion de la caméra depuis le jeu.
* ✔ **Mini-carte déplacée** lors de l’utilisation des commandes tactiles. [**Commandes tactiles**](TOUCH_CONTROLS.md)
* ✔ La mini-carte revient à sa position d’origine lorsque l’on joue avec une manette externe.
* ✔ Déplacement du bouton **Start tactile** vers la gauche afin de ne pas interférer avec la mini-carte.
* ✔ Déplacement léger vers la droite du bouton de **frein** en véhicule.
* ✔ Augmentation de la taille du bouton de **frein** en véhicule.
* ✔ Optimisation des commandes tactiles.
* ✔ Optimisation du système tactile/manette lorsque l’utilisateur joue avec une manette externe.
* ✔ Optimisation du système de connexion et de déconnexion de manette en jeu.
* ✔ Passage plus rapide du mode manette au mode commandes tactiles après la déconnexion de la manette dans le jeu.
* ✔ Améliorations internes pensées pour continuer à améliorer les performances sur les appareils Android plus limités.


##  Version 0.87.2

La version **0.87.2** est une mise à jour centrée sur l’amélioration du confort des contrôles tactiles, l’ajout de nouvelles options de configuration et la correction de certains comportements indésirables détectés après la version 0.87.

Bien qu’il s’agisse d’une mise à jour mineure, elle inclut des changements importants afin de rendre l’expérience de jeu sur Android plus confortable, plus précise et plus configurable.

---

## 🎮 Améliorations du joystick virtuel à pied

Le comportement du **joystick virtuel lorsque le personnage est à pied** a été amélioré.

Dans les versions précédentes, le joystick tactile avait une zone morte trop importante, ce qui pouvait rendre les déplacements du personnage moins précis ou quelque peu inconfortables dans certaines situations.

Dans cette version, cette zone morte excessive a été réduite, ce qui rend le contrôle à pied plus confortable, plus réactif et plus précis lorsque l’on joue directement depuis l’écran tactile.

---

## 🎮 Améliorations de la détection entre manette externe et contrôles tactiles

Le système de détection de la manette externe a été amélioré, ce qui permet désormais à la manette d’être reconnue et mappée plus rapidement. Ainsi, lorsque l’utilisateur connecte une manette en pleine partie, il peut commencer à l’utiliser quelques secondes plus tôt que dans les versions précédentes.

Cela a été possible grâce à une amélioration de l’ensemble du système chargé de détecter si une manette externe est disponible ou non. En fonction de cette détection, le jeu décide s’il doit afficher les contrôles tactiles à l’écran ou les désactiver afin de donner la priorité à la manette externe.

Cette amélioration de l’architecture permet également d’éviter que les contrôles tactiles apparaissent au lancement d’une partie avec une manette déjà connectée, au lieu d’attendre que l’utilisateur appuie pour la première fois sur un bouton de la manette, comme cela se produisait auparavant.

Un bug a également été corrigé : lorsque le joueur entrait dans le jeu avec une manette déjà connectée puis touchait l’écran, les contrôles tactiles apparaissaient et répondaient aux entrées tactiles. Ce comportement n’était pas souhaité lorsqu’une manette externe était déjà disponible.

---

## ⚙️ Nouvelle option de configuration dans l’écran de pause

Une nouvelle option de **configuration** a été ajoutée dans l’écran de pause en jeu.

Depuis cette nouvelle section, l’utilisateur peut modifier certains paramètres du port selon ses préférences sans avoir à quitter complètement l’expérience de jeu.

Cela rend la version Android plus flexible et mieux adaptée aux préférences de chaque joueur.

---

## 📝 Configuration supplémentaire via un fichier externe

Étant donné que l’utilisateur fournit les assets de la version PC originale, et que cette version de 2003 n’incluait pas d’options de menu pour activer ou désactiver la vibration ni pour inverser la caméra, ces paramètres ont été ajoutés via un fichier de configuration externe.

Le fichier se trouve dans le chemin suivant :

```txt
Android/data/com.c4rlox.simpsons/files/Simpsons_configuration/Simpsons_configuration.txt
```

Depuis ce fichier, avant de lancer le jeu, l’utilisateur peut choisir :

* Activer ou désactiver la vibration de la manette.
* Inverser ou non la caméra sur l’axe X.

De cette manière, des options de configuration modernes sont ajoutées tout en respectant les limites des assets originaux de la version PC.

---

## 📳 Correction de la vibration indésirable du téléphone

Un bug provoquant la vibration du téléphone sur certains appareils a été corrigé.

Ce comportement n’était pas intentionnel et ne faisait partie d’aucune fonctionnalité prévue du port. La vibration du téléphone était un effet indésirable qui ne se produisait que sur certains appareils, elle a donc été supprimée.

La vibration configurable reste disponible uniquement pour les manettes compatibles via l’option correspondante du fichier de configuration.

---

## ✨ Résumé de la version 0.87.2

* Amélioration du joystick virtuel lorsque le personnage est à pied.
* Réduction de la zone morte excessive du joystick tactile.
* Déplacement à pied plus confortable et plus précis.
* Ajout d’une option de configuration dans l’écran de pause en jeu.
* Ajout d’un fichier de configuration externe pour les options non disponibles dans les assets originaux de la version PC.
* Ajout d’une option pour activer ou désactiver la vibration de la manette.
* Ajout d’une option pour inverser la caméra sur l’axe X.
* Correction du bug provoquant une vibration indésirable du téléphone sur certains appareils.
* Correction du bug où les contrôles tactiles et une manette externe pouvaient rester actifs en même temps lorsque la manette était connectée avant d’entrer dans le jeu.



##  Version 0.87

La version **0.87** représente l’une des mises à jour les plus importantes à ce jour pour le port Android de *The Simpsons: Hit & Run*.

Cette version se concentre principalement sur l’amélioration de l’expérience de jeu sur appareils mobiles, afin de rendre le port plus confortable, plus accessible et beaucoup plus agréable pour les joueurs qui souhaitent jouer directement avec l’écran tactile.

---

## 🎮 Commandes tactiles entièrement intégrées

La grande nouveauté de cette version est l’ajout de **commandes tactiles entièrement intégrées au système original du jeu**.

Il ne s’agit pas simplement d’une couche de boutons placée par-dessus le jeu. Les commandes tactiles ont été intégrées en respectant l’architecture interne du moteur original, en utilisant le propre système d’entrée du jeu ainsi que les outils que le moteur utilisait pour afficher les éléments à l’écran.

Cela permet d’obtenir une expérience beaucoup plus propre, fluide et cohérente, tout en conservant les meilleures performances possibles et en donnant aux commandes tactiles l’impression de faire réellement partie du jeu.

Les commandes tactiles sont intelligentes et peuvent s’adapter au contexte du joueur, en changeant automatiquement entre les dispositions du menu, du personnage à pied et du véhicule. Le port peut également détecter si une manette physique est connectée, afin d’afficher ou de masquer les commandes tactiles selon la situation.

Pour découvrir tous les détails sur cette nouvelle architecture, les icônes, les dispositions et le fichier de compatibilité tactile, vous pouvez consulter :

* 📱 [Commandes tactiles](TOUCH_CONTROLS.md)

---

## 📱 Meilleur support du mode plein écran

Une amélioration importante a également été ajoutée concernant le mode **plein écran**.

Sur certains appareils Android, le plein écran ne s’appliquait pas correctement, ce qui pouvait empêcher le jeu d’utiliser toute la surface de l’écran ou faire apparaître des barres système de manière indésirable.

Avec cette mise à jour, le support du plein écran a été amélioré afin d’offrir une expérience plus immersive, plus propre et plus agréable sur les appareils où il ne fonctionnait pas correctement auparavant.

---

## ✨ Résumé de la version 0.87

* Ajout de commandes tactiles entièrement intégrées au système original du jeu.
* Ajout de dispositions intelligentes pour le menu, le personnage à pied et les véhicules.
* Ajout de la détection de manette physique pour afficher ou masquer automatiquement les commandes tactiles.
* Ajout d’icônes tactiles personnalisées.
* Ajout d’un fichier de compatibilité tactile pour les appareils qui normalisent les coordonnées tactiles différemment.
* Amélioration du support du plein écran sur les appareils où il ne s’appliquait pas correctement.

La version **0.87** est une énorme avancée pour rendre le port beaucoup plus confortable et naturel sur Android. C’est une mise à jour pensée pour rendre le jeu sur mobile plus agréable, mieux intégré et beaucoup plus proche d’une expérience native.

---

# 🇩🇪 Deutsch

## Version 0.91.0.1 - Touch-Navigation im Statistikbuch

Version **0.91.0.1** fügt die Möglichkeit hinzu, korrekt auf dem Bildschirm des Statistikbuchs pro Level zu navigieren, sodass man zwischen den Levels wechseln kann, indem man auf die rechte oder linke Hälfte des Bildschirms tippt.

Wenn man nun auf die **rechte Hälfte** tippt, werden die Statistiken des nächsten Levels angezeigt, während beim Tippen auf die **linke Hälfte** die Statistiken des vorherigen Levels angezeigt werden.

## Version 0.91 - Minispiel Extrarunde aktiviert
Version **0.91** aktiviert das Minispiel Extrarunde und ermöglicht es, es mit einem für dieses Minispiel angepassten Touch-Layout zu genießen.

## Version 0.90 — Zwischensequenzen korrigiert und stabile 60 FPS

Version **0.90** ist ein wichtiges Update, das sich auf die Verbesserung der allgemeinen Leistung und der Genauigkeit des Android-Ports im Vergleich zum ursprünglichen Verhalten des Spiels konzentriert.

Diese Version führt ein neues stabiles Begrenzungssystem auf **60 FPS** ein, behebt visuelle Probleme während der Zwischensequenzen und verbessert das Verhalten des Spiels in mehreren Bereichen, die mit der internen Synchronisierung und dem Laden der 3D-Welt zusammenhängen.

---

## ⚡ Neue stabile Begrenzung auf 60 FPS

Eine der wichtigsten Verbesserungen dieser Version ist die Implementierung einer deutlich stabileren und präziseren Begrenzung auf **60 FPS**.

In früheren Versionen konnte das Spiel aufgrund einer kleinen Ungenauigkeit bei der FPS-Berechnung mit ungefähr **62 FPS** laufen, anstatt exakt bei **60 FPS** zu bleiben.

Auch wenn dieser Unterschied klein erscheinen mag, wirkte er sich auf mehrere interne Bereiche des Spiels aus, da die ursprüngliche Engine stark an die Aktualisierungsrate gebunden ist. Dadurch entsteht nun ein stabileres Erlebnis, das dem Verhalten des Spiels auf den ursprünglichen Plattformen treuer entspricht.

---

## 🚀 Leistungs- und Stabilitätsverbesserungen

Dank des neuen stabilen Begrenzungssystems auf **60 FPS** enthält diese Version auch wichtige Verbesserungen bei Leistung und Stabilität.

Zu den erzielten Verbesserungen gehören:

* ✔ Der Ladebildschirm beim Betreten der 3D-Welt ist jetzt schneller.
* ✔ Das Spiel hält einen stabileren Ausführungsrhythmus ein.
* ✔ Kleine Unregelmäßigkeiten, die durch eine falsche FPS-Rate verursacht wurden, wurden reduziert.
* ✔ Das interne Verhalten des Spiels ist der Originalversion treuer.
* ✔ Die Spielphysik verhält sich korrekt und stabil.
* ✔ Die übermäßige Helligkeit aus früheren Versionen wurde reduziert.

---

## 🎬 Zwischensequenzen korrigiert

In dieser Version wurden auch die visuellen Probleme im Zusammenhang mit den Zwischensequenzen behoben.

Zuvor konnten einige Zwischensequenzen Reste des vorherigen Frames anzeigen oder kleine visuelle Fehler beim Start der Wiedergabe verursachen.

In Version **0.90** löscht das Spiel den vorherigen Frame korrekt, bevor die Zwischensequenz angezeigt wird, wodurch verhindert wird, dass vorherige Bilder auf dem Bildschirm sichtbar bleiben.

Außerdem wurden während der Wiedergabe schwarze Seitenbalken hinzugefügt, was die visuelle Darstellung der Zwischensequenzen verbessert und Skalierungs- oder Anzeigeprobleme auf Android verhindert.

Diese Korrektur wurde sicher umgesetzt, indem die eigenen Grafikwerkzeuge der Spiel-Engine verwendet wurden.

---

## ✨ Zusammenfassung der Version 0.90

* ✔ Neue stabile und präzise Begrenzung auf **60 FPS**.
* ✔ Das Problem wurde behoben, bei dem das Spiel mit ungefähr **62 FPS** laufen konnte.
* ✔ Die allgemeine Stabilität des Spiels wurde verbessert.
* ✔ Leistungsverbesserungen im Zusammenhang mit der internen Synchronisierung der Engine.
* ✔ Das Laden in die 3D-Welt ist jetzt schneller.
* ✔ Die Spielphysik entspricht dem ursprünglichen Verhalten genauer.
* ✔ Die übermäßige Helligkeit aus früheren Versionen wurde reduziert.
* ✔ Zwischensequenzen korrigiert.
* ✔ Korrektes Löschen des vorherigen Frames vor dem Abspielen einer Zwischensequenz.
* ✔ Schwarze Seitenbalken während der Zwischensequenzen hinzugefügt.


##  Version 0.87.4

Version **0.87.4** ist ein Update, das sich auf die Verbesserung des Spielerlebnisses auf Android konzentriert, insbesondere für Nutzer, die mit **Touch-Steuerung** spielen, und gleichzeitig die Zusammenarbeit zwischen Touch-Steuerung, externem Gamepad und Konfigurationsoptionen im Spiel weiter verbessert.

Dieses Update enthält wichtige Änderungen an der **Minikarte**, den Vibrationseinstellungen des Gamepads, der Kamerainvertierung sowie mehrere interne Optimierungen des Touch-/Gamepad-Systems.

---

## 🗺️ Verschobene Minikarte bei Verwendung der Touch-Steuerung

Eine **sehr wichtige** Änderung in diesem Update ist die **Implementierung eines Systems zum Verschieben der Minikarte im Spiel**, wenn der Nutzer mit Touch-Steuerung spielt.

In früheren Versionen konnte die ursprüngliche Position der Minikarte visuell mit einigen Symbolen der Touch-Steuerung kollidieren, wodurch bestimmte Bereiche des Bildschirms während des Spielens überladener oder weniger komfortabel wirkten.

In Version **0.87.4** wird die Minikarte bei Verwendung der Touch-Steuerung in den oberen rechten Bereich des Bildschirms verschoben. Dadurch wird eine bessere visuelle Erfahrung geboten und verhindert, dass die Touch-Symbole die Minikarte überlagern.

---

## ⚙️ Optionen für Gamepad-Vibration und Kamerainvertierung im Spiel

In dieser Version wurde die Möglichkeit hinzugefügt, einige wichtige Optionen des Ports direkt im Spiel zu aktivieren oder zu deaktivieren:

* **Gamepad-Vibration** aktivieren oder deaktivieren.
* Die Option zum **Invertieren der Kamera** aktivieren oder deaktivieren.

Dafür muss der Nutzer die folgende Datei modifiziert haben:

```txt
Android/data/com.c4rlox.simpsons/files/art/frontend/scrooby/ingame.p3d
```

Konkret muss der **Settings**-Menübildschirm der Konsolenversion verwendet werden, da die originale PC-Version von 2003 diese beiden Optionen nicht in ihrem Konfigurationsmenü enthielt.

Dadurch kann der Nutzer diese Einstellungen komfortabler ändern, ohne ausschließlich auf manuelle Änderungen vor dem Start des Spiels angewiesen zu sein.

Falls der Nutzer diese erwähnte Änderung am Settings-Menü in der Datei ingame.p3d nicht angewendet hat, kann die Änderung weiterhin vor dem Start des Spiels durch Bearbeiten dieser Datei vorgenommen werden:

```txt
Android/data/com.c4rlox.simpsons/files/Simpsons_configuration/Simpsons_configuration.txt
```

---

## 🎮 Beibehaltung der originalen Minikartenposition bei Verwendung eines externen Gamepads

Das System wurde so entwickelt, dass es das ursprüngliche Spielerlebnis von 2003 so weit wie möglich respektiert.

Aus diesem Grund kehrt die Minikarte zu ihrer ursprünglichen Position zurück, wenn der Nutzer ein **externes Gamepad** anschließt.

Das bedeutet, dass die Minikarte nur dann verschoben wird, wenn es wirklich notwendig ist, also wenn der Spieler die Touch-Steuerung verwendet.

So passt die Android-Version die Benutzeroberfläche beim Spielen über den Touchscreen an, behält jedoch die vollständige ursprüngliche HUD-Position bei, wenn mit einem externen Gamepad gespielt wird.

---

## 📱 Anpassung der Touch-Start-Taste

Die **Touch-Start**-Taste wurde auf die linke Seite des Bildschirms verschoben.

Dank dieser Anpassung ist das Layout der Touch-Steuerung besser in das neue Layout der Minikarte integriert.

---

## 🎮 Optimierung der Touch-Steuerung

Diese Version enthält interne Optimierungen am Touch-Steuerungssystem.

Auch wenn der Endnutzer die meisten dieser Optimierungen wahrscheinlich nicht direkt bemerken wird, sind sie wichtig, um die Architektur sauberer, stabiler und effizienter zu halten.

---

## 🎮 Optimierung des Touch-/Gamepad-Systems

Auch das System, das die Zusammenarbeit zwischen **Touch-Steuerung** und **externem Gamepad** verwaltet, wurde optimiert.

Wenn der Nutzer mit einem Gamepad spielt, kann das System unnötige Arbeit im Zusammenhang mit der Touch-Steuerung vermeiden. Dadurch wird die interne Logik effizienter und es wird verhindert, dass beide Systeme miteinander konkurrieren.

---

## 🔌 Verbesserungen beim Verbinden und Trennen eines Gamepads im Spiel

Das System zum Verbinden und Trennen externer Gamepads innerhalb des Spiels wurde optimiert.

Diese Verbesserung betrifft besonders Fälle, in denen der Nutzer ein Gamepad verbindet oder trennt, während er sich bereits im Spiel befindet.

In dieser Version erfolgt der Wechsel vom **Gamepad-Modus** zum **Touch-Steuerungsmodus** nach dem Trennen des Gamepads schneller als in früheren Versionen.

Dadurch kann der Spieler die Touch-Steuerung nach dem Trennen des externen Gamepads mit weniger Wartezeit wieder verwenden.

---

## ⚡ Verbesserungen für leistungsschwächere Geräte

Es ist erwähnenswert, dass diese Optimierungen, mit Ausnahme der Verbesserung des schnelleren Wechsels zurück in den Touch-Modus nach dem Trennen des Gamepads, für den Endnutzer auf leistungsstarken Geräten wahrscheinlich nicht sehr sichtbar sein werden.

Dennoch sind es wichtige Änderungen, um den Port Schritt für Schritt weiter zu verbessern.

Die Idee ist, dass sich viele kleine Optimierungen im Laufe der Zeit besonders für leistungsschwächere Geräte auszahlen können, zum Beispiel für einige **Smart TVs**, ältere Smartphones oder weniger leistungsstarke Android-Geräte.

Auch wenn jede einzelne Verbesserung für sich genommen klein erscheinen mag, hilft ein immer saubereres, stabileres und effizienteres System dabei, die allgemeine Qualität des Ports langfristig zu verbessern.

---

## ✨ Zusammenfassung der Version 0.87.4

* ✔ Möglichkeit hinzugefügt, die Gamepad-Vibration im Spiel zu aktivieren oder zu deaktivieren.
* ✔ Möglichkeit hinzugefügt, die Kamerainvertierung im Spiel zu aktivieren oder zu deaktivieren.
* ✔ **Minikarte verschoben** bei Verwendung der Touch-Steuerung. [**Touch-Steuerung**](TOUCH_CONTROLS.md)
* ✔ Die Minikarte kehrt zu ihrer ursprünglichen Position zurück, wenn mit einem externen Gamepad gespielt wird.
* ✔ Die **Touch-Start**-Taste wurde nach links verschoben, damit sie die Minikarte nicht stört.
* ✔ Die **Bremstaste** im Fahrzeug wurde leicht nach rechts verschoben.
* ✔ Die Größe der **Bremstaste** im Fahrzeug wurde erhöht.
* ✔ Touch-Steuerung optimiert.
* ✔ Touch-/Gamepad-System optimiert, wenn der Nutzer mit einem externen Gamepad spielt.
* ✔ System zum Verbinden und Trennen von Gamepads im Spiel optimiert.
* ✔ Schnellerer Wechsel vom Gamepad-Modus zum Touch-Steuerungsmodus nach dem Trennen des Gamepads im Spiel.
* ✔ Interne Verbesserungen, um die Leistung auf leistungsschwächeren Android-Geräten weiter zu optimieren.


##  Version 0.87.2

Version **0.87.2** ist ein Update, das sich auf die Verbesserung des Komforts der Touch-Steuerung, das Hinzufügen neuer Konfigurationsoptionen und die Behebung einiger unerwünschter Verhaltensweisen konzentriert, die nach Version 0.87 festgestellt wurden.

Obwohl es sich um ein kleineres Update handelt, enthält es wichtige Änderungen, um das Spielerlebnis auf Android komfortabler, präziser und besser konfigurierbar zu machen.

---

## 🎮 Verbesserungen am virtuellen Joystick zu Fuß

Das Verhalten des **virtuellen Joysticks, wenn sich die Figur zu Fuß bewegt**, wurde verbessert.

In früheren Versionen hatte der Touch-Joystick eine zu große Totzone, wodurch sich die Bewegung der Figur in bestimmten Situationen weniger präzise oder etwas unangenehm anfühlen konnte.

In dieser Version wurde diese übermäßige Totzone reduziert, wodurch die Steuerung zu Fuß komfortabler, reaktionsschneller und präziser ist, wenn direkt über den Touchscreen gespielt wird.

---

## 🎮 Verbesserungen bei der Erkennung zwischen externem Gamepad und Touch-Steuerung

Das Erkennungssystem für externe Gamepads wurde verbessert, sodass der Controller nun schneller erkannt und zugeordnet wird. Dadurch kann der Benutzer ein während des Spiels verbundenes Gamepad einige Sekunden früher verwenden als in vorherigen Versionen.

Dies wurde durch eine Verbesserung des gesamten Systems ermöglicht, das dafür zuständig ist, zu erkennen, ob ein externes Gamepad verfügbar ist oder nicht. Auf Grundlage dieser Erkennung entscheidet das Spiel, ob die Touch-Steuerung auf dem Bildschirm angezeigt oder deaktiviert wird, um dem externen Gamepad Vorrang zu geben.

Diese Verbesserung der Architektur verhindert außerdem, dass die Touch-Steuerung beim Starten eines Spiels mit bereits verbundenem Gamepad angezeigt wird, anstatt erst zu warten, bis der Benutzer zum ersten Mal eine Taste auf dem Gamepad drückt, wie es zuvor der Fall war.

Außerdem wurde ein Fehler behoben, bei dem die Touch-Steuerung erschien und auf Touch-Eingaben reagierte, wenn der Spieler das Spiel mit bereits verbundenem Gamepad startete und anschließend den Bildschirm berührte. Dieses Verhalten war nicht beabsichtigt, wenn bereits ein externes Gamepad verfügbar war.

---

## ⚙️ Neue Konfigurationsoption im Pause-Bildschirm

Im Pause-Bildschirm während des Spiels wurde eine neue **Konfigurationsoption** hinzugefügt.

Über diesen neuen Abschnitt kann der Benutzer einige Einstellungen des Ports nach seinen eigenen Vorlieben anpassen, ohne das Spielerlebnis vollständig verlassen zu müssen.

Dadurch wird die Android-Version flexibler und besser an die Vorlieben jedes Spielers angepasst.

---

## 📝 Zusätzliche Konfiguration über eine externe Datei

Da der Benutzer die Assets der originalen PC-Version bereitstellt und diese Version aus dem Jahr 2003 keine Menüoptionen zum Aktivieren oder Deaktivieren der Vibration oder zum Invertieren der Kamera enthielt, wurden diese Einstellungen über eine externe Konfigurationsdatei hinzugefügt.

Die Datei befindet sich unter folgendem Pfad:

```txt
Android/data/com.c4rlox.simpsons/files/Simpsons_configuration/Simpsons_configuration.txt
```

Über diese Datei kann der Benutzer vor dem Start des Spiels Folgendes auswählen:

* Gamepad-Vibration aktivieren oder deaktivieren.
* Kamera auf der X-Achse invertieren oder nicht invertieren.

Auf diese Weise werden moderne Konfigurationsoptionen hinzugefügt, während die Einschränkungen der originalen PC-Version-Assets respektiert werden.

---

## 📳 Behebung unerwünschter Telefonvibration

Ein Fehler wurde behoben, der auf einigen Geräten dazu führte, dass das Telefon selbst vibrierte.

Dieses Verhalten war nicht beabsichtigt und gehörte zu keiner geplanten Funktion des Ports. Die Telefonvibration war ein unerwünschter Effekt, der nur auf bestimmten Geräten auftrat und daher entfernt wurde.

Die konfigurierbare Vibration bleibt ausschließlich für kompatible Gamepads über die entsprechende Option in der Konfigurationsdatei verfügbar.

---

## ✨ Zusammenfassung der Version 0.87.2

* Virtueller Joystick beim Spielen zu Fuß verbessert.
* Übermäßige Totzone des Touch-Joysticks reduziert.
* Bewegung zu Fuß komfortabler und präziser gemacht.
* Konfigurationsoption im Pause-Bildschirm während des Spiels hinzugefügt.
* Externe Konfigurationsdatei für Optionen hinzugefügt, die in den originalen PC-Assets nicht verfügbar sind.
* Option zum Aktivieren oder Deaktivieren der Gamepad-Vibration hinzugefügt.
* Option zum Invertieren der Kamera auf der X-Achse hinzugefügt.
* Fehler behoben, der auf einigen Geräten eine unerwünschte Telefonvibration verursachte.
* Fehler behoben, bei dem Touch-Steuerung und externes Gamepad gleichzeitig aktiv bleiben konnten, wenn das Gamepad vor dem Betreten des Spiels verbunden war.



##  Version 0.87

Version **0.87** ist eines der bisher wichtigsten Updates für den Android-Port von *The Simpsons: Hit & Run*.

Diese Version konzentriert sich vor allem darauf, das Spielerlebnis auf mobilen Geräten zu verbessern und den Port komfortabler, zugänglicher und deutlich angenehmer für alle Spieler zu machen, die direkt über den Touchscreen spielen möchten.

---

## 🎮 Vollständig integrierte Touch-Steuerung

Die größte Neuerung dieser Version ist die Einführung einer **vollständig in das originale Spielsystem integrierten Touch-Steuerung**.

Dabei handelt es sich nicht nur um eine einfache Ebene mit Buttons, die über das Spiel gelegt wurde. Die Touch-Steuerung wurde so integriert, dass sie die interne Architektur der ursprünglichen Engine respektiert und das eigene Eingabesystem des Spiels sowie die Werkzeuge nutzt, mit denen die Engine Bildschirmelemente gezeichnet hat.

Dadurch fühlt sich die Erfahrung deutlich sauberer, flüssiger und stimmiger an, während gleichzeitig die bestmögliche Performance erhalten bleibt und die Touch-Steuerung wie ein echter Teil des Spiels wirkt.

Die Touch-Steuerung ist intelligent und kann sich an den aktuellen Spielkontext anpassen, indem sie automatisch zwischen Menü-, Zu-Fuß- und Fahrzeug-Layouts wechselt. Außerdem kann der Port erkennen, ob ein physischer Controller verbunden ist, und die Touch-Steuerung je nach Situation ein- oder ausblenden.

Alle Details zu dieser neuen Architektur, den Symbolen, den Layouts und der Touch-Kompatibilitätsdatei findest du hier:

* 📱 [Touch-Steuerung](TOUCH_CONTROLS.md)

---

## 📱 Verbesserte Vollbild-Unterstützung

Außerdem wurde eine wichtige Verbesserung für den **Vollbildmodus** hinzugefügt.

Auf einigen Android-Geräten wurde der Vollbildmodus nicht korrekt angewendet. Dadurch konnte es passieren, dass das Spiel nicht den gesamten Bildschirm nutzte oder dass Systemleisten unerwünscht sichtbar blieben.

Mit diesem Update wurde die Vollbild-Unterstützung verbessert, um auf betroffenen Geräten ein immersiveres, saubereres und angenehmeres Spielerlebnis zu ermöglichen.

---

## ✨ Zusammenfassung von Version 0.87

* Vollständig integrierte Touch-Steuerung über das originale Spielsystem hinzugefügt.
* Intelligente Layouts für Menü, Spielfigur zu Fuß und Fahrzeuge hinzugefügt.
* Erkennung physischer Controller hinzugefügt, um die Touch-Steuerung automatisch ein- oder auszublenden.
* Eigene Touch-Symbole hinzugefügt.
* Touch-Kompatibilitätsdatei für Geräte hinzugefügt, die Touch-Koordinaten unterschiedlich normalisieren.
* Verbesserte Vollbild-Unterstützung auf Geräten, bei denen der Vollbildmodus nicht korrekt angewendet wurde.

Version **0.87** ist ein großer Schritt, damit sich der Port auf Android deutlich komfortabler und natürlicher anfühlt. Dieses Update wurde entwickelt, um das Spielen auf mobilen Geräten angenehmer, besser integriert und deutlich näher an einer nativen Erfahrung zu machen.
