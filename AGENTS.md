# Guía para agentes que trabajan con ZEsarUX

Este documento describe cómo inspeccionar, compilar, ejecutar y depurar ZEsarUX de forma segura y reproducible. Se aplica a todo el repositorio. Las instrucciones más específicas que puedan existir en subdirectorios tienen prioridad dentro de su ámbito.

## Principios generales

- Antes de modificar código, inspeccionar el estado con `git status --short`. El árbol puede contener cambios y archivos del usuario que deben conservarse.
- No descartar, sobrescribir, formatear ni incluir en un commit cambios ajenos a la tarea.
- Los comentarios nuevos en el código fuente deben escribirse en español.
- Buscar archivos y símbolos primero con `rg` y `rg --files`.
- Mantener los cambios específicos de una máquina dentro de su implementación. Por ejemplo, una corrección necesaria para TBBlue no debe cambiar el comportamiento de otros Spectrum mediante código común, salvo que esté demostrado que el fallo también es común.
- No crear commits ni hacer `push` a menos que el usuario lo solicite expresamente.

## Ubicación y compilación

El código, el ejecutable y la mayoría de las pruebas están bajo `src/`.

Compilación habitual:

```bash
cd src
./configure
make -j4
```

Ejecutar `./configure` antes de la primera compilación, después de limpiar la configuración o cuando cambien las opciones, dependencias o condiciones del entorno de construcción. Si el árbol ya está configurado y solo se modifica una unidad concreta, puede bastar con ejecutar `make -j4`; el sistema recompilará lo necesario. Revisar siempre los avisos y el código de salida de ambos comandos.

Antes de entregar cambios:

```bash
git diff --check
git diff -- ruta/de/los/archivos/modificados
```

Ejecutar las pruebas relacionadas con el cambio. Un ejemplo para TBBlue es:

```bash
cd src
./tests/tbblue_mmu.sh
```

No debe suponerse que una compilación correcta demuestra el funcionamiento: las correcciones de CPU, memoria, paginado, vídeo o dispositivos requieren una prueba funcional proporcionada al riesgo del cambio.

## Ayuda de línea de comandos

ZEsarUX tiene dos niveles de ayuda y deben consultarse ambos cuando se busque una opción:

```bash
./zesarux --noconfigfile --vo null --ao null --help
./zesarux --noconfigfile --vo null --ao null --experthelp
```

Al hacer pruebas automatizadas, indicar un controlador de vídeo no gráfico. No lanzar el emulador con el controlador Cocoa/X11 predeterminado porque la ventana aparecerá en el escritorio del usuario.

Opciones recomendadas:

```text
--vo null
--ao null
```

`--vo stdout` también puede ser apropiado cuando interesa observar la salida textual. Incluso para consultar opciones durante una sesión de trabajo conviene evitar cualquier ejecución que pueda inicializar una interfaz gráfica.

## Medios e imágenes de disco

- Considerar las imágenes MMC, IDE, disquetes, cintas y snapshots como datos del usuario.
- No modificar el medio original. Usar una copia de prueba cuando sean necesarias escrituras.
- Para MMC, añadir `--mmc-no-persistent-writes` siempre que la prueba no necesite conservar cambios.
- No reemplazar ni regenerar una imagen entregada por el usuario.
- Al comparar versiones de un sistema, comprobar cuidadosamente qué imagen corresponde a cada versión.

Ejemplo seguro para TBBlue:

```bash
./zesarux \
  --noconfigfile \
  --mmc-file tbblue-no-ok.mmc \
  --enable-mmc \
  --machine tbblue \
  --enable-divmmc-ports \
  --mmc-no-persistent-writes \
  --vo null \
  --ao null \
  tbblue-input.zsf
```

## Activación de ZRCP

ZRCP permite controlar y depurar una instancia sin interfaz gráfica. Para activarlo:

```bash
./zesarux \
  --noconfigfile \
  --machine tbblue \
  --vo null \
  --ao null \
  --enable-remoteprotocol \
  --remoteprotocol-port 10003 \
  --exit-after 120
```

Agregar los medios, opciones y snapshot requeridos por la prueba. Elegir un puerto libre y un tiempo de salida suficiente, pero limitado, para no dejar procesos abandonados.

Conexión sencilla:

```bash
nc 127.0.0.1 10003
```

ZRCP dispone de ayuda propia:

```text
help
help nombre-del-comando
```

Consultar la ayuda en vez de adivinar parámetros. En automatizaciones con `nc`, enviar preferentemente un comando cada vez y esperar a que vuelva el prompt. Usar finales de línea CRLF (`\r\n`) si un cliente programático produce respuestas inconsistentes. Varias órdenes enviadas juntas pueden mezclar prompts y resultados.

Al terminar:

```text
exit-emulator
```

`quit` solo cierra la conexión; no debe confundirse con finalizar el emulador.

## Esperar un breakpoint con `run`

No hace falta sondear registros ni implementar una espera externa. ZRCP puede ejecutar la CPU y mantener la conexión bloqueada hasta alcanzar un breakpoint.

Secuencia recomendada:

```text
enter-cpu-step
enable-breakpoints
set-breakpoint 1 PC=0038H
set-breakpointaction 1 break
run verbose no-stop-on-data
```

Comportamiento:

1. `enter-cpu-step` entrega a ZRCP el control de la ejecución de la CPU.
2. `enable-breakpoints` activa globalmente el sistema de breakpoints.
3. `set-breakpoint` define la condición.
4. `set-breakpointaction` hace que la ejecución se detenga al cumplirse.
5. `run` no devuelve el prompt hasta que se dispara un breakpoint, se abre un menú, llega otro evento que detiene la ejecución o se alcanza el límite solicitado.

La respuesta identifica el breakpoint y, con `verbose`, muestra registros, MMU e instrucción actual. Por ejemplo:

```text
Breakpoint fired: PC=38H
PC=0038 ...
0038 PUSH AF
command@cpu-step>
```

Parámetros útiles de `run`:

- `verbose`: muestra el estado al comenzar y terminar.
- Un número: limita la cantidad de instrucciones, por ejemplo `run verbose 100000`.
- `no-stop-on-data`: evita que la llegada de datos por el socket haga retornar `run`. Es importante si otra conexión o herramienta enviará teclas mientras se espera el breakpoint.
- `update-immediately`: actualiza la pantalla después de cada instrucción; requiere las opciones de vídeo real y visualización del electrón y es mucho más lento.

Verificar siempre la sintaxis vigente con:

```text
help run
```

Las condiciones aceptan registros, operadores, memoria, puertos y funciones como `OPMWA`, `OPMRA`, `OPMWV` y `OPMRV`. Ejemplos:

```text
set-breakpoint 1 PC=0C99H
set-breakpoint 2 OPMWA(5CE0H)
set-breakpoint 3 OUTFIRED=1 AND PWA&00FFH=FEH
```

Consultar `help set-breakpoint` para la gramática completa. Los casos simples `PC=XXXX`, `MWA=XXXX` y `MRA=XXXX` tienen optimizaciones internas.

Para continuar después de una parada puede volver a usarse `run`. Para abandonar el modo paso a paso:

```text
exit-cpu-step
```

## Envío controlado de teclado

ZRCP ofrece órdenes de alto nivel como:

```text
send-keys-ascii
send-keys-event
send-keys-string
```

Consultar su ayuda antes de utilizarlas. Para diagnósticos de matriz de teclado puede usarse `set-ui-io-ports`, que establece directamente las ocho filas y el joystick. En ese caso siempre debe enviarse también el estado de liberación; de lo contrario la tecla queda pulsada y genera repeticiones.

Ejemplo usado para pulsar y liberar `A` en Spectrum:

```text
set-ui-io-ports FFFEFFFFFFFFFFFF00
set-ui-io-ports FFFFFFFFFFFFFFFF00
```

La duración entre ambas órdenes importa. Una conexión lenta puede mantener la tecla durante varios frames. Al investigar traducción de teclado, comprobar separadamente:

- el estado de la matriz;
- el carácter traducido por la ROM;
- la escritura en variables como `LAST_K`;
- el búfer donde la aplicación inserta el carácter;
- el código que finalmente lo representa en pantalla.

No concluir que el puerto de teclado está mal solo porque el carácter visible sea incorrecto.

## Memoria y paginado

En máquinas paginadas, una dirección de CPU no identifica por sí sola la memoria física. Antes y después de una operación relevante consultar:

```text
get-memory-pages
get-current-memory-zone
get-memory-zones
```

Lectura visible por la CPU:

```text
read-memory E140H 80H
hexdump E140H 80H
```

Para acceder a otra zona:

```text
set-memory-zone numero
save-binary /tmp/volcado.bin 0 longitud
```

Recordar que el programa puede cambiar la MMU entre dos comandos. Registrar el mapa junto con los datos para que un volcado sea interpretable.

En TBBlue, los registros MMU son NextReg `0x50` a `0x57`. NextReg `0x8E` controla el paginado compatible con Spectrum 128K y su bit 3 decide si una escritura cambia o conserva MMU6/MMU7. Las correcciones en esta lógica deben limitarse a TBBlue.

## Registro de transacciones

El registro de transacciones puede crecer muy rápidamente, sobre todo durante `HALT`, bucles de vídeo o esperas de teclado. Configurarlo antes de activarlo:

```text
cpu-transaction-log logfile /tmp/zesarux-transaction.log
cpu-transaction-log truncate yes
cpu-transaction-log registers yes
cpu-transaction-log ignrephalt yes
cpu-transaction-log enabled yes
```

El nombre `ignrephalt` es el parámetro expuesto actualmente por ZRCP. Confirmarlo con:

```text
help cpu-transaction-log
```

Desactivarlo en cuanto se obtenga el fragmento necesario:

```text
cpu-transaction-log enabled no
```

También puede detenerse desde una acción de breakpoint:

```text
set-breakpointaction 1 stop-transaction-log
```

Siempre que sea posible, preferir `enter-cpu-step` más `run` y un breakpoint preciso. Una traza completa desde el arranque puede ocupar cientos de megabytes y ocultar el código relevante.

## Método recomendado de diagnóstico

1. Reproducir el fallo sin GUI y sin escrituras persistentes.
2. Confirmar la versión, máquina, medios y parámetros exactos.
3. Reducir el problema por capas: entrada, traducción, memoria, CPU, salida y representación.
4. Usar breakpoints para observar el primer punto donde el estado esperado diverge.
5. Anotar el mapa de memoria junto con registros y direcciones.
6. Comparar una versión funcional y otra defectuosa cuando existan ambas.
7. Modificar la implementación más específica que explique la divergencia.
8. Compilar y repetir exactamente la reproducción original.
9. Ejecutar las pruebas de regresión relacionadas.
10. Revisar el diff y confirmar que no se incluyeron cambios del usuario.

## Entrega

El resumen final debe indicar:

- la causa concreta encontrada;
- los archivos modificados;
- cómo se validó;
- cualquier limitación o prueba pendiente;
- si se tocaron medios o datos (normalmente no deberían tocarse).

No afirmar que un problema está corregido basándose únicamente en una hipótesis o en que el proyecto compile.
