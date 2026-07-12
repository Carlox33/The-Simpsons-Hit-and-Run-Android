# 🎮 Simpsons Hit & Run Port para Android

🌍 **Idiomas disponibles**

[🇪🇸 Español](#-español) | [🇬🇧 English](#-english) | [🇫🇷 Français](#-français) | [🇩🇪 Deutsch](#-deutsch)


[Discord](https://discord.gg/6Gw6jTJv8w)
# INSTALLATION.md

## 🇪🇸 Español

Hola, esto es un **port del videojuego *The Simpsons: Hit & Run* para Android**.

Por favor, lee este pequeño documento en tu idioma correspondiente **de principio a fin**, ya que es de **vital importancia para una correcta instalación**.


En el futuro se incorporarán **controles táctiles con sus respectivos iconos**.

### Instalación

1. Elige correctamente la versión **APK** que deseas instalar:
   * **NTSC**
   * **PAL (multi-lenguaje)**

2. Una vez instalada la aplicación, **debes abrirla al menos una vez**. Esto permitirá que la app **genere su carpeta privada automáticamente** en:

   `Android/data/com.c4rlox.simpsons/files`

3. Cuando abras la app te indicará que **faltan archivos**. Estos archivos son los **assets del juego de PC (versión release oficial)**, es decir, **la versión original que venía en el CD**.

   ⚠️ **No utilices los assets de la versión filtrada del código**, ya que **no corresponde a la versión release oficial y no es compatible**.

4. Una vez tengas tus assets, deberás **copiarlos o moverlos a la siguiente carpeta**:

   `Android/data/com.c4rlox.simpsons/files`

   ⚠️ **MUY IMPORTANTE:** La carpeta **debe ser generada por la aplicación al abrirla por primera vez**. Si la creas manualmente, **la app puede no tener permisos para leer los archivos dentro de ella**.

### Extra – Cómo funcionan los assets PAL y NTSC

Los **assets base son los mismos tanto para PAL como para NTSC**. La diferencia importante está en el archivo de diálogos que acompaña a cada versión/idioma:

* **NTSC inglés** → `dialog.rcf`
* **PAL inglés** → `dialog.rcf`
* **PAL español** → `dialogs.rcf`
* **PAL francés** → `dialogf.rcf`
* **PAL alemán** → `dialogg.rcf`

### Extra – Cómo saber qué versión de assets tienes

Si no sabes qué versión de assets tienes, puedes diferenciarlos fácilmente por el archivo de diálogos:

* Si tus assets contienen un archivo llamado **dialog.rcf** → tienes **assets en inglés** (válidos para **NTSC inglés** y también para **PAL inglés**).
* Si tus assets contienen un archivo llamado **dialogs.rcf** → tienes **assets PAL en español**.
* Si tus assets contienen un archivo llamado **dialogf.rcf** → tienes **assets PAL en francés**.
* Si tus assets contienen un archivo llamado **dialogg.rcf** → tienes **assets PAL en alemán**.


## ⚙️ Configuración extra para el usuario

El port de Android incluye varias opciones de configuración extra que pueden ser modificadas por el usuario final mediante archivos externos.

---

## 🖥️ Resolución de renderizado adaptativa

El port incluye un sistema de **resolución de renderizado adaptativa** para Android.

Una vez instalada la aplicación y después de abrirla al menos una vez, se generará automáticamente un archivo llamado:

```txt
Simpsons_resolution.txt
```

Este archivo se generará en la siguiente ruta:

```txt
Android/data/com.c4rlox.simpsons/files
```

Desde este archivo, puedes cambiar la resolución interna de renderizado del juego **antes de iniciarlo**.

Solo necesitas modificar el siguiente valor:

```txt
target_height
```

La anchura de renderizado se calcula automáticamente para respetar la relación de aspecto del dispositivo.

Valores recomendados:

* `720`
* `800`
* `850`
* `900`
* `1080`

---

## 📱 Configuración de los controles táctiles

En la version **0.92** se añadió la posibilidad de poder personalizar los controles táctiles dentro del juego de forma sencilla e intuitiva para el usuario final, pudiendo modificar tanto el tamaño, como la posición de los botones dentro del juego, por otro lado la opacidad (transparencia) de los botones táctiles puede ser modificada a través del archivo:

```txt
Android/data/com.c4rlox.simpsons/touch_controls/Simpsons_touch_controls_configuration.txt
```
Para más información, consulta el archivo **[TOUCH_CONTROLS.md](TOUCH_CONTROLS.md)**.

La versión **0.87** introdujo controles táctiles integrados, permitiendo jugar directamente desde la pantalla del dispositivo Android.

Por defecto, el juego utiliza el **Modo 1** para las coordenadas táctiles. Este modo debería funcionar correctamente en la mayoría de dispositivos.

Si, después de iniciar el juego, notas que los botones táctiles no responden correctamente, que los toques no coinciden con la posición de los iconos o que la zona táctil parece desplazada, cambia al **Modo 2** modificando este archivo:

```txt
Android/data/com.c4rlox.simpsons/touch_controls/Simpsons_touch_mode.txt
```

Resumen:

* **Modo 1:** modo por defecto recomendado.
* **Modo 2:** modo alternativo si los controles táctiles no funcionan correctamente en tu dispositivo.

---

## ⚙️ Configuración adicional 

El port también incluye un archivo de configuración adicional llamado:

```txt
Simpsons_configuration.txt
```

Este archivo se encuentra en la siguiente ruta:

```txt
Android/data/com.c4rlox.simpsons/files/Simpsons_configuration/Simpsons_configuration.txt
```

Cualquier cambio en este archivo debe realizarse **antes de iniciar el juego**.

A través de este archivo, puedes configurar las siguientes opciones:

* Activar o desactivar la vibración del gamepad.
* Elegir si quieres invertir la cámara en el eje X.

Estas opciones no estaban disponibles originalmente en la versión de PC del juego. Como no formaban parte de los assets originales de PC, se ofrecen mediante este archivo de configuración externo en lugar de estar integradas directamente en el menú de configuración del juego.

---


## ⚙️ Configuración extra dentro del juego 

A partir de la versión **0.87.4**, algunos ajustes extra pueden modificarse directamente dentro del juego:

* Vibración del mando.
* Inversión de cámara.

Para usar estas opciones desde el menú de ajustes del juego, es necesario tener aplicado el archivo `ingame.p3d` modificado, debido a que el juego original de 2003 en PC no tenía estas opciones:

```txt
Android/data/com.c4rlox.simpsons/files/art/frontend/scrooby/ingame.p3d
```

Este archivo modificado puede obtenerse uniéndote al servidor de Discord:

[Unirse al servidor de Discord](https://discord.gg/M4FeZpgBxX)

Si no tienes el archivo `ingame.p3d` modificado, estos ajustes todavía pueden cambiarse antes de iniciar el juego editando el archivo de configuración externo:

```txt
Android/data/com.c4rlox.simpsons/files/Simpsons_configuration/Simpsons_configuration.txt
```
---

Espero que disfrutes el port.


---

## 🇬🇧 English

Hello, this is an **Android port of the game *The Simpsons: Hit & Run***.

Please read this short document **from start to finish in your corresponding language**, as it is **very important for a proper installation**.

### Installation

1. Choose carefully which **APK version** you want to install:
   * **NTSC**
   * **PAL (multi-language)**

2. After installing the app, **you must open it at least once**. This allows the app to **generate its private folder automatically** in:

   `Android/data/com.c4rlox.simpsons/files`

3. When you open the app it will tell you that **files are missing**. These files are the **PC game assets (official release version)** — the **original version that came on the CD**.

   ⚠️ **Do not use assets from the leaked source code version**, because **they are not from the official release and are not compatible**.

4. Once you have your assets, **copy or move them into the following folder**:

   `Android/data/com.c4rlox.simpsons/files`

   ⚠️ **VERY IMPORTANT:** The folder **must be generated by the app when opening it for the first time**. If you create it manually, **the app may not have permission to read the files inside it**.

### Extra – How PAL and NTSC assets work

The **base assets are the same for both PAL and NTSC**. The important difference is the dialogue file used by each version/language:

* **NTSC English** → `dialog.rcf`
* **PAL English** → `dialog.rcf`
* **PAL Spanish** → `dialogs.rcf`
* **PAL French** → `dialogf.rcf`
* **PAL German** → `dialogg.rcf`

### Extra – How to know which assets you have

If you are not sure which version of assets you have, you can identify them through the dialogue file:

* If your assets contain a file named **dialog.rcf** → you have **English assets** (valid for **NTSC English** and also **PAL English**).
* If your assets contain a file named **dialogs.rcf** → you have **PAL Spanish assets**.
* If your assets contain a file named **dialogf.rcf** → you have **PAL French assets**.
* If your assets contain a file named **dialogg.rcf** → you have **PAL German assets**.


## ⚙️ Extra User Configuration

The Android port includes several extra configuration options that can be modified by the final user through external files.

---

## 🖥️ Adaptive Rendering Resolution

The port includes an **adaptive rendering resolution** system for Android.

Once the application has been installed and opened at least once, it will automatically generate a file called:

```txt
Simpsons_resolution.txt
```

This file will be generated in the following path:

```txt
Android/data/com.c4rlox.simpsons/files
```

From this file, you can change the internal rendering resolution of the game **before launching it**.

You only need to modify the following value:

```txt
target_height
```

The rendering width is calculated automatically in order to preserve the aspect ratio of the device.

Recommended values:

* `720`
* `800`
* `850`
* `900`
* `1080`

---

## 📱 Touch Controls Configuration

In version **0.92**, the ability to customize the touch controls within the game in a simple and intuitive way for the end user was added, allowing both the size and the position of the in-game buttons to be modified. Additionally, the opacity (transparency) of the touch buttons can be modified through the following file:

```txt
Android/data/com.c4rlox.simpsons/touch_controls/Simpsons_touch_controls_configuration.txt
````

For more information, see the **[TOUCH_CONTROLS.md](TOUCH_CONTROLS.md)** file.


Version **0.87** introduced integrated touch controls, allowing the game to be played directly from the Android device screen.

By default, the game uses **Mode 1** for touch coordinates. This mode should work correctly on most devices.

If, after launching the game, you notice that the touch buttons do not respond correctly, the touch input does not match the icon positions, or the touch area seems displaced, change to **Mode 2** by modifying this file:

```txt
Android/data/com.c4rlox.simpsons/touch_controls/Simpsons_touch_mode.txt
```

Summary:

* **Mode 1:** recommended default mode.
* **Mode 2:** alternative mode if the touch controls do not work correctly on your device.

For more information, check the **[TOUCH_CONTROLS.md](TOUCH_CONTROLS.md)** file.

---

## ⚙️ Additional Gameplay Configuration

The port also includes an additional configuration file called:

```txt
Simpsons_configuration.txt
```

This file is located in the following path:

```txt
Android/data/com.c4rlox.simpsons/files/Simpsons_configuration/Simpsons_configuration.txt
```

Any changes to this file must be made **before launching the game**.

Through this file, you can configure the following options:

* Enable or disable gamepad vibration.
* Choose whether to invert the camera on the X axis.

These options were not originally available in the PC version of the game. Since they were not part of the original PC assets, they are provided through this external configuration file instead of being directly integrated into the in-game settings menu.

---

## ⚙️ Extra in-game configuration

Starting from version **0.87.4**, some extra settings can be changed directly in-game:

* Gamepad vibration.
* Camera inversion.

To use these options from the in-game settings menu, you need to have the modified `ingame.p3d` file applied, because the original 2003 PC game did not include these options:

```txt
Android/data/com.c4rlox.simpsons/files/art/frontend/scrooby/ingame.p3d
```

This modified file can be obtained by joining the Discord server:

[Join the Discord server](https://discord.gg/M4FeZpgBxX)

If you do not have the modified `ingame.p3d` file, these settings can still be changed before launching the game by editing the external configuration file:

```txt
Android/data/com.c4rlox.simpsons/files/Simpsons_configuration/Simpsons_configuration.txt
```

---
I hope you enjoy the port.


---

## 🇫🇷 Français

Bonjour, ceci est **un port Android du jeu *The Simpsons: Hit & Run***.

Veuillez lire ce document **du début à la fin dans votre langue**, car il est **très important pour une installation correcte**.

### Installation

1. Choisissez correctement la **version APK** que vous souhaitez installer :
   * **NTSC**
   * **PAL (multilingue)**

2. Une fois l'application installée, **ouvrez-la au moins une fois**. Cela permettra à l'application **de générer automatiquement son dossier privé** dans :

   `Android/data/com.c4rlox.simpsons/files`

3. Lorsque vous ouvrirez l'application, elle indiquera que **des fichiers sont manquants**. Ces fichiers sont les **assets du jeu PC (version officielle release)**, c'est-à-dire **la version originale du CD**.

   ⚠️ **N'utilisez pas les assets provenant de la version du code source leakée**, car **ils ne correspondent pas à la version officielle et ne sont pas compatibles**.

4. Une fois que vous avez vos assets, **copiez-les dans le dossier suivant** :

  `Android/data/com.c4rlox.simpsons/files`

   ⚠️ **TRÈS IMPORTANT :** Le dossier **doit être créé par l'application lors de son premier lancement**. Si vous le créez manuellement, **l'application peut ne pas avoir les permissions pour lire les fichiers**.

### Extra – Fonctionnement des assets PAL et NTSC

Les **assets de base sont les mêmes pour PAL et NTSC**. La différence importante concerne le fichier de dialogues utilisé selon la version/la langue :

* **NTSC anglais** → `dialog.rcf`
* **PAL anglais** → `dialog.rcf`
* **PAL espagnol** → `dialogs.rcf`
* **PAL français** → `dialogf.rcf`
* **PAL allemand** → `dialogg.rcf`

### Extra – Comment identifier vos assets

Si vous ne savez pas quelle version d'assets vous avez, vous pouvez l'identifier grâce au fichier de dialogues :

* Si vos assets contiennent un fichier nommé **dialog.rcf** → vous avez des **assets anglais** (valables pour **NTSC anglais** et aussi **PAL anglais**).
* Si vos assets contiennent un fichier nommé **dialogs.rcf** → vous avez des **assets PAL espagnols**.
* Si vos assets contiennent un fichier nommé **dialogf.rcf** → vous avez des **assets PAL français**.
* Si vos assets contiennent un fichier nommé **dialogg.rcf** → vous avez des **assets PAL allemands**.


## ⚙️ Configuration supplémentaire pour l’utilisateur

Le port Android inclut plusieurs options de configuration supplémentaires qui peuvent être modifiées par l’utilisateur final à l’aide de fichiers externes.

---

## 🖥️ Résolution de rendu adaptative

Le port inclut un système de **résolution de rendu adaptative** pour Android.

Une fois l’application installée et après l’avoir ouverte au moins une fois, un fichier appelé sera automatiquement généré :

```txt
Simpsons_resolution.txt
```

Ce fichier sera généré dans le chemin suivant :

```txt
Android/data/com.c4rlox.simpsons/files
```

Depuis ce fichier, vous pouvez modifier la résolution interne de rendu du jeu **avant de le lancer**.

Il vous suffit de modifier la valeur suivante :

```txt
target_height
```

La largeur de rendu est calculée automatiquement afin de respecter le format d’image de l’appareil.

Valeurs recommandées :

* `720`
* `800`
* `850`
* `900`
* `1080`

---

## 📱 Configuration des contrôles tactiles

Dans la version **0.92**, la possibilité de personnaliser les commandes tactiles directement dans le jeu de manière simple et intuitive pour l’utilisateur final a été ajoutée, permettant de modifier aussi bien la taille que la position des boutons dans le jeu. Par ailleurs, l’opacité (transparence) des boutons tactiles peut être modifiée à l’aide du fichier suivant :

```txt
Android/data/com.c4rlox.simpsons/touch_controls/Simpsons_touch_controls_configuration.txt
````

Pour plus d’informations, consultez le fichier **[TOUCH_CONTROLS.md](TOUCH_CONTROLS.md)**.

La version **0.87** a introduit des contrôles tactiles intégrés, permettant de jouer directement depuis l’écran de l’appareil Android.

Par défaut, le jeu utilise le **Mode 1** pour les coordonnées tactiles. Ce mode devrait fonctionner correctement sur la plupart des appareils.

Si, après avoir lancé le jeu, vous remarquez que les boutons tactiles ne répondent pas correctement, que les touches ne correspondent pas à la position des icônes ou que la zone tactile semble décalée, passez au **Mode 2** en modifiant ce fichier :

```txt
Android/data/com.c4rlox.simpsons/touch_controls/Simpsons_touch_mode.txt
```

Résumé :

* **Mode 1 :** mode par défaut recommandé.
* **Mode 2 :** mode alternatif si les contrôles tactiles ne fonctionnent pas correctement sur votre appareil.

Pour plus d’informations, consultez le fichier **[TOUCH_CONTROLS.md](TOUCH_CONTROLS.md)**.

---

## ⚙️ Configuration supplémentaire du gameplay

Le port inclut également un fichier de configuration supplémentaire appelé :

```txt
Simpsons_configuration.txt
```

Ce fichier se trouve dans le chemin suivant :

```txt
Android/data/com.c4rlox.simpsons/files/Simpsons_configuration/Simpsons_configuration.txt
```

Toute modification de ce fichier doit être effectuée **avant de lancer le jeu**.

Grâce à ce fichier, vous pouvez configurer les options suivantes :

* Activer ou désactiver la vibration de la manette.
* Choisir si vous souhaitez inverser la caméra sur l’axe X.

Ces options n’étaient pas disponibles à l’origine dans la version PC du jeu. Comme elles ne faisaient pas partie des assets originaux de la version PC, elles sont proposées via ce fichier de configuration externe au lieu d’être directement intégrées dans le menu de configuration du jeu.

---


## ⚙️ Configuration supplémentaire en jeu

À partir de la version **0.87.4**, certains réglages supplémentaires peuvent être modifiés directement en jeu:

* Vibration de la manette.
* Inversion de la caméra.

Pour utiliser ces options depuis le menu des réglages du jeu, il est nécessaire d’avoir appliqué le fichier `ingame.p3d` modifié, car le jeu PC original de 2003 n’incluait pas ces options:

```txt
Android/data/com.c4rlox.simpsons/files/art/frontend/scrooby/ingame.p3d
```

Ce fichier modifié peut être obtenu en rejoignant le serveur Discord:

[Rejoindre le serveur Discord](https://discord.gg/M4FeZpgBxX)

Si vous n’avez pas le fichier `ingame.p3d` modifié, ces réglages peuvent toujours être changés avant de lancer le jeu en modifiant le fichier de configuration externe:

```txt
Android/data/com.c4rlox.simpsons/files/Simpsons_configuration/Simpsons_configuration.txt
```

---

J’espère que vous apprécierez le port.



---

## 🇩🇪 Deutsch

Hallo, dies ist **ein Android-Port des Spiels *The Simpsons: Hit & Run***.

Bitte lies dieses Dokument **vollständig in deiner Sprache**, da es **sehr wichtig für eine korrekte Installation** ist.

### Installation

1. Wähle die gewünschte **APK-Version**:
   * **NTSC**
   * **PAL (Mehrsprachig)**

2. Nach der Installation **öffne die App mindestens einmal**, damit sie **automatisch ihren privaten Ordner erstellt**:

   `Android/data/com.c4rlox.simpsons/files`

3. Die App wird anzeigen, dass **Dateien fehlen**. Diese Dateien sind die **PC-Spielassets der offiziellen Release-Version (CD-Version)**.

   ⚠️ **Verwende keine Assets aus der geleakten Source-Code-Version**, da diese **nicht kompatibel sind**.

4. Kopiere deine Assets in:

   `Android/data/com.c4rlox.simpsons/files`

   ⚠️ **SEHR WICHTIG:** Der Ordner **muss von der App beim ersten Start erstellt werden**.

### Extra – Wie PAL- und NTSC-Assets funktionieren

Die **Basis-Assets sind bei PAL und NTSC gleich**. Der wichtige Unterschied liegt in der verwendeten Dialogdatei je nach Version/Sprache:

* **NTSC Englisch** → `dialog.rcf`
* **PAL Englisch** → `dialog.rcf`
* **PAL Spanisch** → `dialogs.rcf`
* **PAL Französisch** → `dialogf.rcf`
* **PAL Deutsch** → `dialogg.rcf`

### Extra – So erkennst du, welche Assets du hast

Wenn du nicht sicher bist, welche Asset-Version du hast, kannst du sie über die Dialogdatei erkennen:

* Wenn deine Assets eine Datei namens **dialog.rcf** enthalten → hast du **englische Assets** (gültig für **NTSC Englisch** und auch **PAL Englisch**).
* Wenn deine Assets eine Datei namens **dialogs.rcf** enthalten → hast du **PAL-Assets auf Spanisch**.
* Wenn deine Assets eine Datei namens **dialogf.rcf** enthalten → hast du **PAL-Assets auf Französisch**.
* Wenn deine Assets eine Datei namens **dialogg.rcf** enthalten → hast du **PAL-Assets auf Deutsch**.


## ⚙️ Zusätzliche Benutzerkonfiguration

Der Android-Port enthält mehrere zusätzliche Konfigurationsoptionen, die vom Endbenutzer über externe Dateien geändert werden können.

---

## 🖥️ Adaptive Renderauflösung

Der Port enthält ein System für **adaptive Renderauflösung** auf Android.

Nachdem die Anwendung installiert und mindestens einmal geöffnet wurde, wird automatisch eine Datei mit folgendem Namen erstellt:

```txt
Simpsons_resolution.txt
```

Diese Datei wird im folgenden Pfad erstellt:

```txt
Android/data/com.c4rlox.simpsons/files
```

Über diese Datei können Sie die interne Renderauflösung des Spiels **vor dem Starten des Spiels** ändern.

Sie müssen nur den folgenden Wert ändern:

```txt
target_height
```

Die Renderbreite wird automatisch berechnet, um das Seitenverhältnis des Geräts beizubehalten.

Empfohlene Werte:

* `720`
* `800`
* `850`
* `900`
* `1080`

---

## 📱 Konfiguration der Touch-Steuerung

In Version **0.92** wurde die Möglichkeit hinzugefügt, die Touch-Steuerung im Spiel auf einfache und intuitive Weise für den Endbenutzer anzupassen. Dabei können sowohl die Größe als auch die Position der Schaltflächen im Spiel geändert werden. Darüber hinaus kann die Deckkraft (Transparenz) der Touch-Schaltflächen über die folgende Datei angepasst werden:

```txt
Android/data/com.c4rlox.simpsons/touch_controls/Simpsons_touch_controls_configuration.txt
````

Weitere Informationen findest du in der Datei **[TOUCH_CONTROLS.md](TOUCH_CONTROLS.md)**.

Version **0.87** führte integrierte Touch-Steuerungen ein, mit denen das Spiel direkt über den Bildschirm des Android-Geräts gespielt werden kann.

Standardmäßig verwendet das Spiel **Modus 1** für die Touch-Koordinaten. Dieser Modus sollte auf den meisten Geräten korrekt funktionieren.

Wenn Sie nach dem Starten des Spiels feststellen, dass die Touch-Buttons nicht richtig reagieren, die Berührungen nicht mit der Position der Symbole übereinstimmen oder der Touch-Bereich verschoben wirkt, wechseln Sie zu **Modus 2**, indem Sie diese Datei ändern:

```txt
Android/data/com.c4rlox.simpsons/touch_controls/Simpsons_touch_mode.txt
```

Zusammenfassung:

* **Modus 1:** empfohlener Standardmodus.
* **Modus 2:** alternativer Modus, falls die Touch-Steuerung auf Ihrem Gerät nicht korrekt funktioniert.

Weitere Informationen finden Sie in der Datei **[TOUCH_CONTROLS.md](TOUCH_CONTROLS.md)**.

---

## ⚙️ Zusätzliche Gameplay-Konfiguration

Der Port enthält außerdem eine zusätzliche Konfigurationsdatei mit dem Namen:

```txt
Simpsons_configuration.txt
```

Diese Datei befindet sich im folgenden Pfad:

```txt
Android/data/com.c4rlox.simpsons/files/Simpsons_configuration/Simpsons_configuration.txt
```

Alle Änderungen an dieser Datei müssen **vor dem Starten des Spiels** vorgenommen werden.

Über diese Datei können Sie die folgenden Optionen konfigurieren:

* Gamepad-Vibration aktivieren oder deaktivieren.
* Auswählen, ob die Kamera auf der X-Achse invertiert werden soll.

Diese Optionen waren ursprünglich in der PC-Version des Spiels nicht verfügbar. Da sie nicht Teil der originalen PC-Assets waren, werden sie über diese externe Konfigurationsdatei bereitgestellt, anstatt direkt in das Konfigurationsmenü des Spiels integriert zu sein.

---


## ⚙️ Zusätzliche Einstellungen im Spiel

Ab Version **0.87.4** können einige zusätzliche Einstellungen direkt im Spiel geändert werden:

* Gamepad-Vibration.
* Kamerainvertierung.

Um diese Optionen über das Einstellungsmenü im Spiel zu verwenden, muss die modifizierte Datei `ingame.p3d` angewendet werden, da das originale PC-Spiel von 2003 diese Optionen nicht enthielt:

```txt
Android/data/com.c4rlox.simpsons/files/art/frontend/scrooby/ingame.p3d
```

Diese modifizierte Datei kann über den Discord-Server bezogen werden:

[Dem Discord-Server beitreten](https://discord.gg/M4FeZpgBxX)

Wenn du die modifizierte Datei `ingame.p3d` nicht hast, können diese Einstellungen trotzdem vor dem Starten des Spiels geändert werden, indem die externe Konfigurationsdatei bearbeitet wird:

```txt
Android/data/com.c4rlox.simpsons/files/Simpsons_configuration/Simpsons_configuration.txt
```

---

Ich hoffe, Sie genießen den Port.

