# 🎮 Touch Controls - Version 0.87

🌍 **Available languages**

[🇪🇸 Español](#-español) | [🇬🇧 English](#-english) | [🇫🇷 Français](#-français) | [🇩🇪 Deutsch](#-deutsch)

---

# 🇪🇸 Español

## 🎮 Controles táctiles totalmente integrados

Con la versión **0.87**, el port de *The Simpsons: Hit & Run* para Android recibe una de sus mejoras más importantes hasta la fecha: **controles táctiles completamente integrados con el sistema original del juego**.

Esta implementación no ha sido sencilla, pero ha merecido totalmente la pena. Han sido más de **100 horas de trabajo**, investigación, pruebas y ajustes para conseguir que el resultado no se sienta como un simple añadido externo, sino como una parte real del propio juego.

El objetivo desde el principio era claro: no colocar una simple capa visual encima del juego, sino construir un sistema táctil que respetara al máximo el motor original, su arquitectura interna y la forma en la que el propio juego gestionaba la entrada y el renderizado en 2003.

Por eso, los controles táctiles han sido integrados usando el propio sistema del juego y las herramientas originales que el motor utilizaba para dibujar elementos en pantalla. Esto permite conseguir una experiencia mucho más sólida, limpia y coherente, manteniendo el mejor rendimiento posible y una integración mucho más fiel con el funcionamiento interno del juego.

En la actualizacion  **0.87.4** se ha desplazado el minimapa del juego para ofrecer una mejor experiencia al usuario que juega con controles táctiles, no interfiriendo ahora la posición de los iconos con el minimapa como ocurría en versiones anteriores

## ✨ Iconos diseñados manualmente

Todos los iconos táctiles han sido diseñados por mí usando **GIMP**, cuidando que encajen visualmente con la estética del juego y que sean claros durante la partida.

La idea no era simplemente colocar botones en pantalla, sino crear una interfaz táctil cómoda, reconocible y agradable de usar, especialmente pensada para dispositivos Android.

Cada icono ha sido preparado intentando mantener un equilibrio entre visibilidad, estilo y comodidad, para que el jugador pueda entender rápidamente qué acción realiza cada botón sin romper la personalidad visual del juego.

## 🧠 Controles inteligentes según el contexto

Los nuevos controles táctiles son inteligentes y saben reconocer en qué situación se encuentra el jugador.

El sistema cambia automáticamente entre diferentes layouts según el contexto:

* **Menú**
* **A pie**
* **En vehículo**

Esto significa que los controles no son estáticos ni aparecen siempre de la misma forma. El juego adapta el layout táctil dependiendo de lo que esté ocurriendo en pantalla.

Además, cuando el jugador va a pie, el sistema puede mostrar iconos personalizados para las diferentes acciones que puede realizar el personaje, haciendo que la experiencia sea más clara, natural y cómoda.

Esto permite que la pantalla no esté llena de botones innecesarios y que solo aparezcan los controles realmente útiles en cada momento.


## 🛠️ Personalización completa de los controles táctiles

Con la versión **0.92**, el sistema táctil da un paso más al incorporar un nuevo menú desde el que cada usuario puede personalizar los controles directamente dentro del juego de una forma sencilla e intuitiva.

Este nuevo sistema permite modificar tanto la **posición** como el **tamaño** de los botones táctiles, haciendo posible adaptar su distribución a diferentes tamaños de pantalla, formas de jugar y preferencias personales.

Además, la **opacidad o transparencia** de los botones puede configurarse manualmente mediante el siguiente archivo:

```txt
Android/data/com.c4rlox.simpsons/touch_controls/Simpsons_touch_controls_configuration.txt
```

De esta forma, cada jugador puede crear una distribución más cómoda y adaptada a su dispositivo sin perder la integración del sistema táctil con el motor original del juego.

### 📍 Acceso al menú de personalización

El botón para acceder al nuevo menú de personalización se encuentra disponible en dos lugares:

* En el menú principal de opciones, dentro de la **casa de Homer**.
* En el menú de pausa del **mundo 3D**, dentro del apartado **Opciones**.

> Este menú ha sido diseñado para utilizarse completamente mediante la pantalla táctil, por lo que solamente estará disponible cuando **NO haya ningún mando externo conectado**.

Cuando un mando físico está conectado, el sistema mantiene el comportamiento habitual y oculta el acceso al editor táctil.

### 💾 Guardar cambios y cambiar de layout

Al acceder al menú de personalización, aparecen dos botones en la parte superior central de la pantalla.

El botón situado a la izquierda permite realizar dos acciones al mismo tiempo:

1. Guardar los cambios realizados en el layout actual.
2. Avanzar al siguiente grupo de controles táctiles.

Los layouts se muestran en el siguiente orden:

1. **Controles a pie**
2. **Controles del vehículo**
3. **Controles del menú**

De esta forma, el usuario puede revisar y personalizar de manera independiente los tres grupos principales de controles del juego.

Después de guardar el tercer layout, todos los grupos de controles táctiles habrán quedado configurados.

### 🔄 Restablecer los controles

El botón **RESET**, situado también en la parte superior central de la pantalla, permite restaurar la configuración predeterminada del layout que se esté editando en ese momento.

El restablecimiento afecta únicamente al grupo de controles actual.

Por ejemplo, si el usuario está modificando los **controles del vehículo** y pulsa **RESET**, solamente se restaurarán los botones correspondientes al vehículo. Los controles a pie y los controles del menú conservarán sus posiciones y tamaños personalizados.

Esto permite experimentar con diferentes configuraciones sin perder los cambios realizados en los demás layouts.

### 👆 Mover los botones

Los iconos táctiles pueden desplazarse libremente por la pantalla mediante una pulsación sostenida y un movimiento de arrastre.

El procedimiento es el siguiente:

1. Mantén pulsado el icono que quieras mover.
2. Arrástralo hasta la posición deseada.
3. Suelta el icono para colocarlo en su nueva ubicación.

Este sistema permite adaptar la distribución de los controles a las preferencias del usuario y colocarlos en las zonas de la pantalla que le resulten más cómodas.

### 📐 Modificar el tamaño de los botones

El tamaño de cada botón puede modificarse realizando pulsaciones rápidas sobre su icono dentro del menú de personalización.

Cada botón dispone de **11 niveles de tamaño**, numerados desde el nivel `0` hasta el nivel `10`.

El nivel utilizado por defecto es:

```txt
Nivel 5
```

Con cada pulsación rápida, el tamaño del botón aumenta un **10 %**, siempre que el nivel actual sea inferior a `10`.

El cambio de tamaño sigue un comportamiento cíclico:

```txt
0 → 1 → 2 → 3 → 4 → 5 → 6 → 7 → 8 → 9 → 10 → 0 → 1...
```

Por tanto:

* Si el botón se encuentra en un nivel inferior a `10`, avanzará al siguiente nivel.
* Si el botón se encuentra en el nivel `10`, la siguiente pulsación lo devolverá al nivel `0`.

Esto permite recorrer rápidamente todos los tamaños disponibles sin necesidad de utilizar barras, menús adicionales o controles externos.

> Una pulsación rápida modifica el tamaño del botón, mientras que una pulsación sostenida acompañada de un desplazamiento permite moverlo.

### ✥ Comportamiento especial de la cruceta del menú

Las cuatro flechas que forman la cruceta del layout correspondiente a los controles del menú disponen de un comportamiento adicional.

Estas flechas pueden moverse de dos maneras diferentes:

* **Mover toda la cruceta:** arrastrando el espacio central formado por las cuatro flechas.
* **Mover una flecha individualmente:** arrastrando directamente la flecha que se quiera modificar, como ocurre con cualquier otro botón.

Gracias a este comportamiento, es posible desplazar las cuatro flechas como un único conjunto para mantener su distribución original o ajustar individualmente la posición de cada una.

## ⚙️ Archivo de configuración de los controles táctiles

Los cambios de posición y tamaño realizados desde el menú se almacenan en el archivo:

```txt
Android/data/com.c4rlox.simpsons/touch_controls/Simpsons_touch_controls_configuration.txt
```

Este archivo también permite modificar manualmente la opacidad de los botones táctiles, tanto en su estado normal como cuando están siendo pulsados.



## 🎮 Integración como un mando real

Uno de los puntos más importantes de esta versión es que los controles táctiles no funcionan como un añadido externo improvisado. Han sido integrados como si fueran un **mando disponible dentro del propio sistema del juego**.

Gracias a esto, el juego puede trabajar con los controles táctiles de forma mucho más natural, respetando mejor su sistema interno de input.

Además, el port es capaz de detectar si hay un mando físico conectado. Si el jugador conecta un mando, el layout táctil puede ocultarse automáticamente. Si no hay mando disponible, los iconos táctiles aparecen para permitir jugar cómodamente desde la pantalla.

Esto hace que la experiencia sea mucho más flexible: puedes jugar con controles táctiles cuando quieras, pero también puedes usar un mando físico sin que la interfaz táctil moleste en pantalla.

## ⚙️ Archivo de compatibilidad táctil: Simpsons_touch_mode.txt

Debido a diferencias técnicas entre dispositivos Android, algunos móviles pueden normalizar las coordenadas táctiles de forma diferente.

Para ofrecer la mayor compatibilidad posible, he añadido un archivo llamado:

```txt
Simpsons_touch_mode.txt
```

Este archivo se encuentra en la siguiente ruta:

```txt
Android/data/com.c4rlox.simpsons/touch_controls/Simpsons_touch_mode.txt
```

Por defecto, el juego utiliza el **modo 1**, que debería funcionar correctamente en la mayoría de dispositivos.

Sin embargo, si en tu móvil notas que los controles táctiles no responden bien, que los toques no coinciden correctamente con la posición de los botones o que la zona táctil parece desplazada, puedes cambiar al **modo 2** desde este archivo.

Este sistema se ha añadido para que más dispositivos puedan disfrutar de los controles táctiles correctamente, incluso cuando Android o el propio dispositivo gestionan las coordenadas táctiles de una forma diferente.

En resumen:

* **Modo 1**: modo por defecto y recomendado para la mayoría de dispositivos.
* **Modo 2**: modo alternativo de compatibilidad si el modo 1 no funciona correctamente en tu móvil.

## 🚀 Una mejora clave para Android

Esta actualización representa un paso enorme para hacer que el port sea mucho más accesible, cómodo y disfrutable en Android.

La versión **0.87** no solo añade botones en pantalla: introduce una arquitectura táctil mucho más seria, pensada para integrarse con el juego de la forma más limpia posible y ofrecer una experiencia realmente cómoda.

De hecho, el resultado se siente tan natural que, en algunos momentos, puede llegar a ser igual de cómodo o incluso más cómodo que un control tradicional. Lo digo totalmente en serio: merece mucho la pena probarlo.

---

# 🇬🇧 English

## 🎮 Fully integrated touch controls

With version **0.87**, the Android port of *The Simpsons: Hit & Run* receives one of its most important improvements so far: **touch controls fully integrated with the original game system**.

This implementation was not easy, but it was absolutely worth it. It represents more than **100 hours of work**, research, testing and fine-tuning to make the result feel not like a simple external addition, but like a real part of the game itself.

From the beginning, the goal was clear: not to place a simple visual layer on top of the game, but to build a touch control system that respected the original engine, its internal architecture and the way the game handled input and rendering back in 2003.

That is why the touch controls have been integrated using the game’s own systems and the original tools the engine used to draw elements on screen. This helps provide a much more solid, clean and coherent experience, while keeping the best possible performance and a much more faithful integration with the game’s internal behavior.

In update **0.87.4**, the in-game minimap has been moved to provide a better experience for users playing with touch controls, preventing the position of the icons from interfering with the minimap as happened in previous versions.

## ✨ Manually designed icons

All touch icons have been designed by me using **GIMP**, carefully making sure they fit the visual style of the game and remain clear during gameplay.

The idea was not simply to place buttons on the screen, but to create a comfortable, recognizable and pleasant touch interface, especially designed for Android devices.

Each icon has been prepared with a balance between visibility, style and comfort, so the player can quickly understand each action without breaking the visual personality of the game.

## 🧠 Smart context-aware controls

The new touch controls are intelligent and can recognize the situation the player is currently in.

The system automatically switches between different layouts depending on the context:

* **Menu**
* **On foot**
* **In vehicle**

This means the controls are not static and do not always appear in the same way. The game adapts the touch layout depending on what is happening on screen.

When the player is on foot, the system can also display custom icons for the different actions the character can perform, making the experience clearer, more natural and more comfortable.

This prevents the screen from being filled with unnecessary buttons and ensures that only the controls that are actually useful appear at the right moment.

## 🛠️ Complete touch control customization

With version **0.92**, the touch control system takes another step forward by introducing a new menu that allows each user to customize the controls directly within the game in a simple and intuitive way.

This new system allows both the **position** and **size** of the touch buttons to be modified, making it possible to adapt their layout to different screen sizes, play styles, and personal preferences.

In addition, the **opacity or transparency** of the buttons can be configured manually through the following file:

```txt
Android/data/com.c4rlox.simpsons/touch_controls/Simpsons_touch_controls_configuration.txt
```

This allows each player to create a more comfortable layout adapted to their device without losing the integration of the touch control system with the game’s original engine.

### 📍 Accessing the customization menu

The button for accessing the new customization menu is available in two locations:

* In the main options menu, inside **Homer’s house**.
* In the **3D world** pause menu, under **Options**.

> This menu has been designed to be operated entirely through the touchscreen, so it will only be available when **NO external controller is connected**.

When a physical controller is connected, the system maintains its usual behavior and hides access to the touch control editor.

### 💾 Saving changes and switching layouts

When entering the customization menu, two buttons appear at the top center of the screen.

The button on the left performs two actions at the same time:

1. Saves the changes made to the current layout.
2. Moves on to the next group of touch controls.

The layouts are displayed in the following order:

1. **On-foot controls**
2. **Vehicle controls**
3. **Menu controls**

This allows users to review and customize the game’s three main control groups independently.

After saving the third layout, all touch control groups will have been configured.

### 🔄 Resetting the controls

The **RESET** button, also located at the top center of the screen, restores the default configuration of the layout currently being edited.

Resetting only affects the current control group.

For example, if the user is editing the **vehicle controls** and presses **RESET**, only the vehicle-related buttons will be restored. The on-foot controls and menu controls will retain their customized positions and sizes.

This allows users to experiment with different configurations without losing the changes made to the other layouts.

### 👆 Moving the buttons

The touch icons can be moved freely around the screen by pressing and holding them while dragging.

The procedure is as follows:

1. Press and hold the icon you want to move.
2. Drag it to the desired position.
3. Release the icon to place it in its new location.

This system allows users to adapt the control layout to their preferences and place the buttons in the areas of the screen they find most comfortable.

### 📐 Changing the size of the buttons

The size of each button can be changed by quickly tapping its icon within the customization menu.

Each button has **11 size levels**, numbered from level `0` to level `10`.

The default level is:

```txt
Level 5
```

With each quick tap, the button size increases by **10%**, provided that the current level is lower than `10`.

The size adjustment follows a cyclical pattern:

```txt
0 → 1 → 2 → 3 → 4 → 5 → 6 → 7 → 8 → 9 → 10 → 0 → 1...
```

Therefore:

* If the button is at a level lower than `10`, it will move to the next level.
* If the button is at level `10`, the next tap will return it to level `0`.

This makes it possible to quickly cycle through all available sizes without using sliders, additional menus, or external controls.

> A quick tap changes the size of the button, while pressing, holding, and dragging allows it to be moved.

### ✥ Special behavior of the menu directional pad

The four arrows that form the directional pad in the menu control layout have an additional feature.

These arrows can be moved in two different ways:

* **Move the entire directional pad:** drag the central space formed by the four arrows.
* **Move an individual arrow:** drag the specific arrow you want to reposition, just like any other button.

Thanks to this behavior, the four arrows can either be moved together as a single group to preserve their original arrangement or positioned individually.

## ⚙️ Touch control configuration file

The position and size changes made through the menu are stored in the following file:

```txt
Android/data/com.c4rlox.simpsons/touch_controls/Simpsons_touch_controls_configuration.txt
```

This file also allows the opacity of the touch buttons to be adjusted manually, both in their normal state and while they are being pressed.


## 🎮 Integrated as a real controller

One of the most important aspects of this version is that the touch controls do not work like a quick external workaround. They have been integrated as if they were a **real controller available inside the game’s own input system**.

Because of this, the game can work with the touch controls in a much more natural way, while better respecting its internal input system.

The port is also able to detect whether a physical controller is connected. If the player connects a controller, the touch layout can be hidden automatically. If no controller is available, the touch icons appear so the game can be played comfortably from the screen.

This makes the experience much more flexible: you can play with touch controls whenever you want, but you can also use a physical controller without the touch interface getting in the way.

## ⚙️ Touch compatibility file: Simpsons_touch_mode.txt

Due to technical differences between Android devices, some phones may normalize touch coordinates in a different way.

To provide the best compatibility possible, I have added a file called:

```txt
Simpsons_touch_mode.txt
```

This file is located in the following path:

```txt
Android/data/com.c4rlox.simpsons/touch_controls/Simpsons_touch_mode.txt
```

By default, the game uses **mode 1**, which should work correctly on most devices.

However, if you notice that the touch controls do not respond properly on your phone, that touches do not correctly match the button positions, or that the touch area feels offset, you can switch to **mode 2** using this file.

This system has been added so that more devices can enjoy the touch controls correctly, even when Android or the device itself handles touch coordinates differently.

In summary:

* **Mode 1**: default mode, recommended for most devices.
* **Mode 2**: alternative compatibility mode if mode 1 does not work correctly on your phone.

## 🚀 A key improvement for Android

This update represents a huge step toward making the port much more accessible, comfortable and enjoyable on Android.

Version **0.87** does not simply add buttons to the screen: it introduces a much more serious touch architecture, designed to integrate with the game as cleanly as possible and offer a truly comfortable experience.

In fact, the result feels so natural that, in some moments, it can be just as comfortable as a traditional controller, or maybe even more comfortable. I am completely serious: it is really worth trying.

---

# 🇫🇷 Français

## 🎮 Commandes tactiles entièrement intégrées

Avec la version **0.87**, le port Android de *The Simpsons: Hit & Run* reçoit l’une de ses améliorations les plus importantes à ce jour : **des commandes tactiles entièrement intégrées au système original du jeu**.

Cette implémentation n’a pas été simple, mais elle en valait vraiment la peine. Elle représente plus de **100 heures de travail**, de recherche, de tests et d’ajustements afin que le résultat ne ressemble pas à un simple ajout externe, mais à une véritable partie du jeu lui-même.

Dès le départ, l’objectif était clair : ne pas placer une simple couche visuelle par-dessus le jeu, mais construire un système de commandes tactiles qui respecte au maximum le moteur original, son architecture interne et la manière dont le jeu gérait les entrées et l’affichage en 2003.

C’est pourquoi les commandes tactiles ont été intégrées en utilisant les propres systèmes du jeu ainsi que les outils originaux que le moteur utilisait pour afficher les éléments à l’écran. Cela permet d’obtenir une expérience beaucoup plus solide, propre et cohérente, tout en conservant les meilleures performances possibles et une intégration beaucoup plus fidèle au fonctionnement interne du jeu.

Dans la mise à jour **0.87.4**, la mini-carte du jeu a été déplacée afin d’offrir une meilleure expérience aux utilisateurs jouant avec les commandes tactiles, en évitant désormais que la position des icônes n’interfère avec la mini-carte comme c’était le cas dans les versions précédentes.

## ✨ Icônes conçues manuellement

Toutes les icônes tactiles ont été conçues par moi avec **GIMP**, en veillant à ce qu’elles s’intègrent visuellement à l’esthétique du jeu tout en restant claires pendant la partie.

L’idée n’était pas simplement de placer des boutons à l’écran, mais de créer une interface tactile confortable, reconnaissable et agréable à utiliser, spécialement pensée pour les appareils Android.

Chaque icône a été préparée en recherchant un équilibre entre lisibilité, style et confort, afin que le joueur puisse comprendre rapidement chaque action sans casser la personnalité visuelle du jeu.

## 🧠 Commandes intelligentes selon le contexte

Les nouvelles commandes tactiles sont intelligentes et savent reconnaître la situation dans laquelle se trouve le joueur.

Le système change automatiquement de disposition selon le contexte :

* **Menu**
* **À pied**
* **En véhicule**

Cela signifie que les commandes ne sont pas statiques et n’apparaissent pas toujours de la même manière. Le jeu adapte la disposition tactile en fonction de ce qui se passe à l’écran.

Lorsque le joueur est à pied, le système peut également afficher des icônes personnalisées pour les différentes actions que le personnage peut effectuer, ce qui rend l’expérience plus claire, plus naturelle et plus confortable.

Cela évite de surcharger l’écran avec des boutons inutiles et permet d’afficher uniquement les commandes réellement utiles au bon moment.

## 🛠️ Personnalisation complète des commandes tactiles

Avec la version **0.92**, le système de commandes tactiles franchit une nouvelle étape grâce à l’ajout d’un nouveau menu permettant à chaque utilisateur de personnaliser directement les commandes dans le jeu, de manière simple et intuitive.

Ce nouveau système permet de modifier à la fois la **position** et la **taille** des boutons tactiles, afin d’adapter leur disposition aux différentes tailles d’écran, aux différentes façons de jouer et aux préférences personnelles.

De plus, l’**opacité ou la transparence** des boutons peut être configurée manuellement à l’aide du fichier suivant :

```txt
Android/data/com.c4rlox.simpsons/touch_controls/Simpsons_touch_controls_configuration.txt
```

Chaque joueur peut ainsi créer une disposition plus confortable et mieux adaptée à son appareil, sans perdre l’intégration du système tactile avec le moteur d’origine du jeu.

### 📍 Accès au menu de personnalisation

Le bouton permettant d’accéder au nouveau menu de personnalisation est disponible à deux endroits :

* Dans le menu principal des options, à l’intérieur de la **maison d’Homer**.
* Dans le menu de pause du **monde 3D**, dans la section **Options**.

> Ce menu a été conçu pour être entièrement utilisé à l’aide de l’écran tactile. Il sera donc uniquement disponible lorsqu’**AUCUNE manette externe n’est connectée**.

Lorsqu’une manette physique est connectée, le système conserve son comportement habituel et masque l’accès à l’éditeur des commandes tactiles.

### 💾 Enregistrer les modifications et changer de disposition

Lorsque vous accédez au menu de personnalisation, deux boutons apparaissent dans la partie supérieure centrale de l’écran.

Le bouton situé à gauche permet d’effectuer deux actions simultanément :

1. Enregistrer les modifications apportées à la disposition actuelle.
2. Passer au groupe de commandes tactiles suivant.

Les dispositions sont affichées dans l’ordre suivant :

1. **Commandes à pied**
2. **Commandes du véhicule**
3. **Commandes des menus**

L’utilisateur peut ainsi consulter et personnaliser indépendamment les trois principaux groupes de commandes du jeu.

Après l’enregistrement de la troisième disposition, tous les groupes de commandes tactiles auront été configurés.

### 🔄 Réinitialiser les commandes

Le bouton **RESET**, également situé dans la partie supérieure centrale de l’écran, permet de restaurer la configuration par défaut de la disposition actuellement en cours de modification.

La réinitialisation affecte uniquement le groupe de commandes actuel.

Par exemple, si l’utilisateur modifie les **commandes du véhicule** et appuie sur **RESET**, seuls les boutons correspondant au véhicule seront restaurés. Les commandes à pied et les commandes des menus conserveront leurs positions et leurs tailles personnalisées.

Cela permet d’essayer différentes configurations sans perdre les modifications apportées aux autres dispositions.

### 👆 Déplacer les boutons

Les icônes tactiles peuvent être déplacées librement sur l’écran à l’aide d’une pression prolongée suivie d’un mouvement de glissement.

La procédure est la suivante :

1. Maintenez enfoncée l’icône que vous souhaitez déplacer.
2. Faites-la glisser jusqu’à la position souhaitée.
3. Relâchez l’icône pour la placer à son nouvel emplacement.

Ce système permet d’adapter la disposition des commandes aux préférences de l’utilisateur et de placer les boutons dans les zones de l’écran qui lui semblent les plus confortables.

### 📐 Modifier la taille des boutons

La taille de chaque bouton peut être modifiée en appuyant rapidement sur son icône dans le menu de personnalisation.

Chaque bouton possède **11 niveaux de taille**, numérotés du niveau `0` au niveau `10`.

Le niveau utilisé par défaut est :

```txt
Niveau 5
```

À chaque pression rapide, la taille du bouton augmente de **10 %**, à condition que le niveau actuel soit inférieur à `10`.

Le changement de taille suit un fonctionnement cyclique :

```txt
0 → 1 → 2 → 3 → 4 → 5 → 6 → 7 → 8 → 9 → 10 → 0 → 1...
```

Par conséquent :

* Si le bouton se trouve à un niveau inférieur à `10`, il passera au niveau suivant.
* Si le bouton se trouve au niveau `10`, la pression suivante le ramènera au niveau `0`.

Cela permet de parcourir rapidement toutes les tailles disponibles sans utiliser de curseurs, de menus supplémentaires ou de commandes externes.

> Une pression rapide modifie la taille du bouton, tandis qu’une pression prolongée accompagnée d’un glissement permet de le déplacer.

### ✥ Fonctionnement particulier de la croix directionnelle des menus

Les quatre flèches formant la croix directionnelle de la disposition des commandes des menus disposent d’une fonctionnalité supplémentaire.

Ces flèches peuvent être déplacées de deux manières différentes :

* **Déplacer toute la croix directionnelle :** faites glisser l’espace central formé par les quatre flèches.
* **Déplacer une flèche individuellement :** faites directement glisser la flèche que vous souhaitez repositionner, comme pour n’importe quel autre bouton.

Grâce à ce fonctionnement, les quatre flèches peuvent être déplacées ensemble comme un seul groupe afin de conserver leur disposition d’origine, ou être repositionnées individuellement.

## ⚙️ Fichier de configuration des commandes tactiles

Les modifications de position et de taille effectuées depuis le menu sont enregistrées dans le fichier suivant :

```txt
Android/data/com.c4rlox.simpsons/touch_controls/Simpsons_touch_controls_configuration.txt
```

Ce fichier permet également de modifier manuellement l’opacité des boutons tactiles, aussi bien dans leur état normal que lorsqu’ils sont enfoncés.

## 🎮 Intégration comme une vraie manette

L’un des points les plus importants de cette version est que les commandes tactiles ne fonctionnent pas comme une solution externe improvisée. Elles ont été intégrées comme s’il s’agissait d’une **véritable manette disponible dans le système d’entrée du jeu**.

Grâce à cela, le jeu peut travailler avec les commandes tactiles de manière beaucoup plus naturelle, tout en respectant mieux son système interne d’input.

Le port est également capable de détecter si une manette physique est connectée. Si le joueur connecte une manette, la disposition tactile peut se masquer automatiquement. Si aucune manette n’est disponible, les icônes tactiles apparaissent afin de permettre de jouer confortablement depuis l’écran.

Cela rend l’expérience beaucoup plus flexible : vous pouvez jouer avec les commandes tactiles quand vous le souhaitez, mais vous pouvez aussi utiliser une manette physique sans que l’interface tactile gêne l’écran.

## ⚙️ Fichier de compatibilité tactile : Simpsons_touch_mode.txt

En raison de différences techniques entre les appareils Android, certains téléphones peuvent normaliser les coordonnées tactiles d’une manière différente.

Pour offrir la meilleure compatibilité possible, j’ai ajouté un fichier appelé :

```txt
Simpsons_touch_mode.txt
```

Ce fichier se trouve dans le chemin suivant :

```txt
Android/data/com.c4rlox.simpsons/touch_controls/Simpsons_touch_mode.txt
```

Par défaut, le jeu utilise le **mode 1**, qui devrait fonctionner correctement sur la plupart des appareils.

Cependant, si vous remarquez que les commandes tactiles ne répondent pas correctement sur votre téléphone, que les appuis ne correspondent pas bien à la position des boutons ou que la zone tactile semble décalée, vous pouvez passer au **mode 2** grâce à ce fichier.

Ce système a été ajouté afin que davantage d’appareils puissent profiter correctement des commandes tactiles, même lorsque Android ou l’appareil lui-même gère les coordonnées tactiles d’une manière différente.

En résumé :

* **Mode 1** : mode par défaut, recommandé pour la plupart des appareils.
* **Mode 2** : mode de compatibilité alternatif si le mode 1 ne fonctionne pas correctement sur votre téléphone.

## 🚀 Une amélioration essentielle pour Android

Cette mise à jour représente une énorme avancée pour rendre le port beaucoup plus accessible, confortable et agréable sur Android.

La version **0.87** n’ajoute pas simplement des boutons à l’écran : elle introduit une architecture tactile beaucoup plus sérieuse, pensée pour s’intégrer au jeu de la manière la plus propre possible et offrir une expérience réellement confortable.

En réalité, le résultat paraît tellement naturel que, dans certains moments, il peut être aussi confortable qu’une manette traditionnelle, voire même plus confortable. Je le dis très sérieusement : cela vaut vraiment la peine de l’essayer.

---

# 🇩🇪 Deutsch

## 🎮 Vollständig integrierte Touch-Steuerung

Mit Version **0.87** erhält der Android-Port von *The Simpsons: Hit & Run* eine der bisher wichtigsten Verbesserungen: **eine vollständig in das originale Spielsystem integrierte Touch-Steuerung**.

Diese Umsetzung war nicht einfach, aber sie hat sich absolut gelohnt. Mehr als **100 Stunden Arbeit**, Recherche, Tests und Feinanpassungen waren nötig, damit sich das Ergebnis nicht wie eine einfache externe Ergänzung anfühlt, sondern wie ein echter Teil des Spiels selbst.

Von Anfang an war das Ziel klar: Es sollte nicht einfach nur eine visuelle Ebene über das Spiel gelegt werden. Stattdessen sollte ein Touch-Steuerungssystem entstehen, das die ursprüngliche Engine, ihre interne Architektur und die Art, wie das Spiel im Jahr 2003 Eingaben und Darstellung verwaltet hat, so weit wie möglich respektiert.

Deshalb wurden die Touch-Steuerelemente mit den eigenen Systemen des Spiels und den originalen Werkzeugen integriert, die die Engine zum Zeichnen von Bildschirmelementen verwendet hat. Dadurch entsteht eine deutlich stabilere, sauberere und stimmigere Spielerfahrung, mit der bestmöglichen Performance und einer viel engeren Verbindung zur internen Logik des Spiels.

Im Update **0.87.4** wurde die Minikarte des Spiels verschoben, um Nutzern, die mit Touch-Steuerung spielen, ein besseres Spielerlebnis zu bieten. Die Position der Symbole überlappt nun nicht mehr mit der Minikarte, wie es in früheren Versionen der Fall war.

## ✨ Manuell gestaltete Symbole

Alle Touch-Symbole wurden von mir mit **GIMP** gestaltet. Dabei wurde darauf geachtet, dass sie optisch zum Stil des Spiels passen und während des Spielens klar erkennbar bleiben.

Die Idee war nicht einfach, ein paar Buttons auf den Bildschirm zu setzen, sondern eine angenehme, verständliche und komfortable Touch-Oberfläche zu schaffen, die speziell für Android-Geräte entwickelt wurde.

Jedes Symbol wurde mit einem Gleichgewicht aus Sichtbarkeit, Stil und Bedienkomfort vorbereitet, damit der Spieler jede Aktion schnell versteht, ohne dass die visuelle Persönlichkeit des Spiels verloren geht.

## 🧠 Intelligente Steuerung je nach Kontext

Die neuen Touch-Steuerelemente sind intelligent und erkennen, in welcher Spielsituation sich der Spieler gerade befindet.

Das System wechselt automatisch zwischen verschiedenen Layouts:

* **Menü**
* **Zu Fuß**
* **Im Fahrzeug**

Das bedeutet, dass die Steuerung nicht statisch ist und nicht immer auf die gleiche Weise angezeigt wird. Das Spiel passt das Touch-Layout daran an, was gerade auf dem Bildschirm passiert.

Wenn der Spieler zu Fuß unterwegs ist, kann das System außerdem spezielle Symbole für die verschiedenen Aktionen anzeigen, die die Figur ausführen kann. Dadurch wirkt die Steuerung klarer, natürlicher und angenehmer.

So wird verhindert, dass der Bildschirm mit unnötigen Buttons überladen wird. Stattdessen erscheinen nur die Steuerelemente, die im jeweiligen Moment wirklich sinnvoll sind.


## 🛠️ Vollständige Anpassung der Touch-Steuerung

Mit Version **0.92** wird das Touch-Steuerungssystem um ein neues Menü erweitert, über das jeder Benutzer die Steuerung direkt im Spiel auf einfache und intuitive Weise anpassen kann.

Mit diesem neuen System lassen sich sowohl die **Position** als auch die **Größe** der Touch-Schaltflächen verändern. Dadurch kann ihre Anordnung an unterschiedliche Bildschirmgrößen, Spielweisen und persönliche Vorlieben angepasst werden.

Darüber hinaus kann die **Deckkraft beziehungsweise Transparenz** der Schaltflächen manuell über die folgende Datei konfiguriert werden:

```txt
Android/data/com.c4rlox.simpsons/touch_controls/Simpsons_touch_controls_configuration.txt
```

Auf diese Weise kann jeder Spieler eine bequemere, an das eigene Gerät angepasste Anordnung erstellen, ohne dass die Integration des Touch-Steuerungssystems in die ursprüngliche Spiel-Engine verloren geht.

### 📍 Zugriff auf das Anpassungsmenü

Die Schaltfläche zum Öffnen des neuen Anpassungsmenüs ist an zwei Stellen verfügbar:

* Im Hauptmenü unter den Optionen innerhalb von **Homers Haus**.
* Im Pausenmenü der **3D-Welt** im Bereich **Optionen**.

> Dieses Menü wurde vollständig für die Bedienung über den Touchscreen entwickelt und ist daher nur verfügbar, wenn **KEIN externer Controller verbunden ist**.

Wenn ein physischer Controller verbunden ist, behält das System sein übliches Verhalten bei und blendet den Zugriff auf den Touch-Steuerungseditor aus.

### 💾 Änderungen speichern und das Layout wechseln

Nach dem Öffnen des Anpassungsmenüs werden oben in der Mitte des Bildschirms zwei Schaltflächen angezeigt.

Die Schaltfläche auf der linken Seite führt zwei Aktionen gleichzeitig aus:

1. Sie speichert die am aktuellen Layout vorgenommenen Änderungen.
2. Sie wechselt zur nächsten Gruppe von Touch-Steuerelementen.

Die Layouts werden in der folgenden Reihenfolge angezeigt:

1. **Steuerung zu Fuß**
2. **Fahrzeugsteuerung**
3. **Menüsteuerung**

Dadurch kann der Benutzer die drei wichtigsten Steuerungsgruppen des Spiels unabhängig voneinander überprüfen und anpassen.

Nach dem Speichern des dritten Layouts wurden alle Gruppen der Touch-Steuerung konfiguriert.

### 🔄 Steuerung zurücksetzen

Mit der ebenfalls oben in der Mitte des Bildschirms angezeigten Schaltfläche **RESET** kann die Standardkonfiguration des aktuell bearbeiteten Layouts wiederhergestellt werden.

Das Zurücksetzen betrifft ausschließlich die aktuelle Steuerungsgruppe.

Wenn der Benutzer beispielsweise die **Fahrzeugsteuerung** bearbeitet und **RESET** drückt, werden nur die zum Fahrzeug gehörenden Schaltflächen zurückgesetzt. Die Steuerung zu Fuß und die Menüsteuerung behalten ihre angepassten Positionen und Größen bei.

Dadurch können verschiedene Konfigurationen ausprobiert werden, ohne dass die an den anderen Layouts vorgenommenen Änderungen verloren gehen.

### 👆 Schaltflächen verschieben

Die Touch-Symbole können durch Gedrückthalten und Ziehen frei auf dem Bildschirm verschoben werden.

Die Vorgehensweise ist wie folgt:

1. Halte das Symbol gedrückt, das du verschieben möchtest.
2. Ziehe es an die gewünschte Position.
3. Lasse das Symbol los, um es an seiner neuen Position zu platzieren.

Mit diesem System kann die Anordnung der Steuerungselemente an die Vorlieben des Benutzers angepasst werden. Die Schaltflächen lassen sich dadurch in den Bereichen des Bildschirms platzieren, die als besonders angenehm empfunden werden.

### 📐 Größe der Schaltflächen ändern

Die Größe jeder Schaltfläche kann durch kurzes Antippen des entsprechenden Symbols im Anpassungsmenü geändert werden.

Jede Schaltfläche verfügt über **11 Größenstufen**, die von Stufe `0` bis Stufe `10` nummeriert sind.

Die standardmäßig verwendete Stufe ist:

```txt
Stufe 5
```

Mit jedem kurzen Antippen wird die Schaltfläche um **10 %** vergrößert, sofern die aktuelle Stufe kleiner als `10` ist.

Die Größenänderung folgt einem zyklischen Verhalten:

```txt
0 → 1 → 2 → 3 → 4 → 5 → 6 → 7 → 8 → 9 → 10 → 0 → 1...
```

Das bedeutet:

* Befindet sich die Schaltfläche auf einer Stufe unter `10`, wechselt sie zur nächsten Stufe.
* Befindet sich die Schaltfläche auf Stufe `10`, wird sie beim nächsten Antippen auf Stufe `0` zurückgesetzt.

Dadurch können alle verfügbaren Größen schnell durchlaufen werden, ohne Schieberegler, zusätzliche Menüs oder externe Steuergeräte verwenden zu müssen.

> Durch kurzes Antippen wird die Größe der Schaltfläche geändert. Durch Gedrückthalten und gleichzeitiges Ziehen kann sie verschoben werden.

### ✥ Besonderes Verhalten des Menüsteuerkreuzes

Die vier Pfeile, die das Steuerkreuz im Layout der Menüsteuerung bilden, verfügen über eine zusätzliche Funktion.

Diese Pfeile können auf zwei verschiedene Arten verschoben werden:

* **Das gesamte Steuerkreuz verschieben:** den von den vier Pfeilen gebildeten freien Bereich in der Mitte ziehen.
* **Einen einzelnen Pfeil verschieben:** den gewünschten Pfeil direkt ziehen, genauso wie jede andere Schaltfläche.

Dank dieses Verhaltens können die vier Pfeile entweder gemeinsam als eine Einheit verschoben werden, um ihre ursprüngliche Anordnung beizubehalten, oder einzeln positioniert werden.

## ⚙️ Konfigurationsdatei der Touch-Steuerung

Die über das Menü vorgenommenen Änderungen an Position und Größe werden in der folgenden Datei gespeichert:

```txt
Android/data/com.c4rlox.simpsons/touch_controls/Simpsons_touch_controls_configuration.txt
```

Über diese Datei kann außerdem die Deckkraft der Touch-Schaltflächen manuell angepasst werden, sowohl im normalen Zustand als auch während sie gedrückt werden.

## 🎮 Integration wie ein echter Controller

Einer der wichtigsten Punkte dieser Version ist, dass die Touch-Steuerung nicht wie eine schnelle externe Notlösung funktioniert. Sie wurde so integriert, als wäre sie ein **echter Controller innerhalb des eigenen Eingabesystems des Spiels**.

Dadurch kann das Spiel viel natürlicher mit den Touch-Eingaben arbeiten und gleichzeitig sein ursprüngliches Input-System besser respektieren.

Der Port kann außerdem erkennen, ob ein physischer Controller verbunden ist. Wenn der Spieler einen Controller verbindet, kann das Touch-Layout automatisch ausgeblendet werden. Wenn kein Controller verfügbar ist, erscheinen die Touch-Symbole, damit das Spiel bequem über den Bildschirm gespielt werden kann.

Das macht die Erfahrung deutlich flexibler: Man kann jederzeit mit Touch-Steuerung spielen, aber auch einen physischen Controller verwenden, ohne dass die Touch-Oberfläche auf dem Bildschirm stört.

## ⚙️ Touch-Kompatibilitätsdatei: Simpsons_touch_mode.txt

Aufgrund technischer Unterschiede zwischen Android-Geräten können manche Smartphones Touch-Koordinaten unterschiedlich normalisieren.

Um die bestmögliche Kompatibilität zu bieten, habe ich eine Datei hinzugefügt:

```txt
Simpsons_touch_mode.txt
```

Diese Datei befindet sich im folgenden Pfad:

```txt
Android/data/com.c4rlox.simpsons/touch_controls/Simpsons_touch_mode.txt
```

Standardmäßig verwendet das Spiel **Modus 1**, der auf den meisten Geräten korrekt funktionieren sollte.

Falls du jedoch bemerkst, dass die Touch-Steuerung auf deinem Gerät nicht richtig reagiert, dass Berührungen nicht exakt mit den Button-Positionen übereinstimmen oder dass sich der Touch-Bereich verschoben anfühlt, kannst du über diese Datei auf **Modus 2** wechseln.

Dieses System wurde hinzugefügt, damit mehr Geräte die Touch-Steuerung korrekt nutzen können, selbst wenn Android oder das jeweilige Gerät Touch-Koordinaten auf eine andere Weise verarbeitet.

Zusammengefasst:

* **Modus 1**: Standardmodus, empfohlen für die meisten Geräte.
* **Modus 2**: alternativer Kompatibilitätsmodus, falls Modus 1 auf deinem Gerät nicht korrekt funktioniert.

## 🚀 Eine wichtige Verbesserung für Android

Dieses Update ist ein großer Schritt, um den Port auf Android zugänglicher, komfortabler und angenehmer spielbar zu machen.

Version **0.87** fügt nicht einfach nur Buttons auf dem Bildschirm hinzu: Sie führt eine deutlich durchdachtere Touch-Architektur ein, die so sauber wie möglich mit dem Spiel integriert wurde und eine wirklich komfortable Erfahrung bietet.

Tatsächlich fühlt sich das Ergebnis so natürlich an, dass es in manchen Momenten genauso angenehm wie ein klassischer Controller sein kann, vielleicht sogar noch angenehmer. Ich meine das völlig ernst: Es lohnt sich wirklich, es auszuprobieren.
