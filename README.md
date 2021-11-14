# 2021_ASO_P1_F1_Linux_Kernel_Module

---

## Index
1. [Introducció](#Introducció)
    1. [Objectius de la pràctica](#Objectius-de-la-pràctica)
    2. [Resum de l'enunciat](#Resum-de-l'enunciat) 
    3. [Requeriments](#Requeriments)
2. [Explicacio per a poder executar](#Explicacio-per-a-poder-executar) 
    1. [Software](#Software)
    2. [Hardware](#Hardware)
3. [Procediment](#Procediment)
4. [Problemes observats](#Problemes-observats)
5. [Conclusió](#Conclusió)

---

## Introducció

### Objectius de la pràctica
* Aplicar en un escenari real els coneixements adquirits en l’assignatura.
* Modificar el codi font i recompilar paquets de software.
* Aprendre a compilar mòduls del nucli (kernel) del sistema i adaptar-lo a les necessitats funcionals i de hardware en cada cas.
* Modificar mòduls del nucli (kernel) per adaptar-los a l’entorn utilitzat.

### Resum de l'enunciat 
Crear un LKM per a la raspberry pi. Aquest ens ha de permetre interactuar i controlar una petita placa. Aquesta placa disposa de 4 botons i 2 leds. 

Hi ha un boto per engegar i un altre per parar cadascun dels leds. Adicionalment aquests botons executaran un script.sh a la raspberry.

### Requeriments
* El LKM (Linux Kernel Module) s’ha de poder instal·lar en una Raspberry Pi amb Raspbian.
* El LKM ha de poder executar una acció cada vegada que es premi un dels botons.
* El LKM ha de ser capaç d’encendre/apagar els 2 leds. Hi haurà 4 botons.
* Cada un dels botons ha d’executar un script diferent.
* S’ha de mantenir un recompte de quantes vegades s’ha pres un botó.
* En tot moment s’han de generar logs per cada acció que es faci en el LKM.

## Explicacio per a poder executar

### Software

Per a poder compilar i instalar el modul amb facilitat hem creat un makefile. Tenint en compte que ja es te el codi en el dispositiu.

Per compilar el modul executar:
> _make_ 

Per instalar el modul executar:
> _make install_

Per desinstalar el modul cal executar:
> _make delete_

Per borrar els fitxers de compilacio, un cop ja s'ha instal·lat:
> _make clean_

#### <u>Prerequisits</u>

Per a que la compilacio funcioni adequadament cal:
* Tenir els headers per controlar els pins instal·lats. (No venen per defecte)
* Haver reiniciat el sistema despres de l'ultima actialització de paquets.
* Haver modificat el fitxer buttons.c per especificar-li la ruta on se situen els escripts a executar. Linea 194 a 197.

### Hardware

Per a la pràctica s'ha fet us de l'esquema electric proporcionat a l'enunciat.

![General EXT2](img/documentacio_ext2_general.png)

## Procediment

A continuació s'expliquen els passos que s'han seguit per a assolir la practica.

<u>Comprar el hardware</u>

En el meu cas com que no crec donar-li molta més utilitat a la raspberry en un futur próxim m'he decantat per a comprar la més económica. La `raspberry pi 0 Hardware`. Pel que fa a la unitat de memória segueix el mateix patró, és una micro sd de 4gb.

<u>Instal·lar el sistema operatiu</u>

Per a aquest procés he fet us de l'instalador que et proporciona raspberry a la seva pagina web. El `Raspberry Pi Imager`. 
Donat que la unitat de memória només disposa de 4gb (unes 3,9gb útils), no es possible instal·lar el sistema operatiu amb interfície gràfica, per falta d'espai. Per he instal·lat el `Raspberry Pi OS Lite(32-bit)`. 

<u>Connexió a la xarxa</u>

Per a connectar-me via el servei ssh que ve instal·lat he de connectar la raspberry i l'ordinador a la mateixa xarxa. Connectar l'ordinador és tribial. Pero per a connectar la raspberry s'ha de connectar la sd a l'ordinador, obrir la carpeta arrel de la sd i crear 2 arxius. El primer, que es deixa buit s'ha d'anomenar `ssh.txt`. El segon s'ha d'anomenar `wpa_supplicant.conf` i ha de contenir la configuració seguent:

```conf
ctrl_interface=DIR=/var/run/wpa_supplicant GROUP=netdev
update_config=1
country=US

network={
    ssid="YOUR_SSID"
    psk="YOUR_PASS"
    key_mgmt=WPA-PSK
}
```
>* YOUR_SSID - ssid de la xarxa a la que vols connectar la raspberry
>* YOUR_PASS - contrasenya de la xarxa

Finalment queda connectar la sd a la raspberry i connectar-li la font d'alimentació.

Mes endevant vaig trobar que es podia configurar desde el `Raspberry Pi Imager`, pero com ja ho tenia tot configurat no ho vaig haver de fer servir.

<u>Connexió a la raspberry - ssh</u>

Un cop tenim els dos dispositius a la mateixa xarxa podem connectar-nos via ssh. El nom d'usuari es pi i el nom de host és raspberry(nomes si és la unica connectada en aquesta xarxa). En cas contrari s'haurà de buscar la ip d'aquesta, amb altres métodes. 

Un cop conectats per primer cop, li fem una petita configuració. Primerament canviar la contrasenya que ve per defecte `raspberry`. I canviar-li el hostname per a poder connectar-nos amb més facilitat. 

Un cop tenim aixó actualitzarem tot el software de la manera més habitual `apt update` i `apt upgrade`. Seguidament reiniciarem la raspberry per assegurar que tot s'inicia amb la ultima versió.

<u>Instal·lacio de software</u>

Caldrà instal·lar els headers per a poder interectuar amb els pins. 

> sudo apt install raspberrypi-kernel-headers

En aquest pas també instal·larem git i la resta de eines que considerem necessaries. Així com els directoris necessaris per a poder treballar d'una manera comode i ordenada.

<u>Creació del projecte</u>

Un cop tenim tot els passos anteriors crearem el makefile:

```Makefile
EXECUTABLE= buttons

obj-m := $(EXECUTABLE).o

all:
	sudo make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) modules

install:
	sudo insmod $(EXECUTABLE).ko

delete:
	sudo rmmod $(EXECUTABLE).ko

clean: 
	sudo make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) clean
```

Aques ens permet compilar amb `make`, instal·lar amb `make install`, desinstal·lar amb `make delete` i netejar els fitxers de compilacioó del modul amb `make clean`.

<u>Codi</u>

Per a especificar que volem que faci el modul farem un programa en c. Aquest es troba a [`buttons.c`](/Button/buttons.c). 

A continuació es fara una petita explicació de l'estructura i les funcions d'aquest. 

<u>Estroctura del codi</u>

Primerament especificarem dades generals sobre el modul en questió.

```c
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Bernat Segura");
MODULE_DESCRIPTION("KLM to manage 4 buttons to open and close 2 leds and execute a script for each button");
MODULE_VERSION("1.0");
```

Tots els moduls de kernel tenen les seguents funcions. Aquestes s'executaràn quan es creï i quan s'elimini el modul respectivament. El paràmetre que reben és una funció que nosaltres definirem i que s'executarà quan es crei o s'elimini.

```c
module_init(gpio_init);
module_exit(gpio_exit);
```

Per a aconseguir el menor temps possible de resposta treballarem amb imterrupcions. Quan s'apreti un botó saltarà una interrupció, que executarà una funció on estarà implementat el que ha de fer. Aquesta funció és el handler i s'encarregarà de totes les interrupcions.

```c
static irq_handler_t gpio_irq_handler(unsigned int irq, void *dev_id, struct pt_regs *regs);
```

Ara que ja sabem que en instal·lar el modul s'iniciarà tot el necessari. Quan salti una interrupció s'executarà el handler i quan eliminem el modul s'alliberaran tots els recursos. Anem a veure les funcions de gpio i interrupcions que fan possible tot aixó.

<u>Funcions externes</u>

En la funció de init ens trobem amb:

* gpio_is_valid: funció que reb un port de la gpio i ens diu si està en ús.
```c
gpio_is_valid(gpioLED1);
```

* gpio_request: Demana al sistema el pin que previament hem comprovat que no es trobava en ús.
```c
gpio_request(gpioLED1, "sysfs");
```

* gpio_direction_output: Indica que el port de la gpio és de sortida. I seteja el valor inicial. 
```c
gpio_direction_output(gpioLED1, led1On);
```

* gpio_direction_input: Indica que el port de la gpio és d'entrada. 
```c
gpio_direction_input(gpioButton1);
```

* gpio_export: Fa accessible el port via sysfd. I decideix si pot canviar de input a output i viceversa.
```c
gpio_export(gpioLED1, false);
```

* gpio_set_debounce: Crea un control de rebots del temps indicat en ms.
```c
gpio_set_debounce(gpioButton1, DEBOUNCE_TIME);
```

* gpio_to_irq: Associa un port a una interrupcio.
```c
irqNumber1 = gpio_to_irq(gpioButton1);
```

* request_irq: Es demana la interrupció al sistema. I es configura com treballarà aqeusta. En el nostre cas com que tenim els botons per pull up treballa amb el flanc de baixada `IRQF_TRIGGER_FALLING` i com a funció de handler tenim `gpio_irq_handler`. 
```c
result1 = request_irq(irqNumber1, (irq_handler_t) gpio_irq_handler, IRQF_TRIGGER_FALLING, "gpio_handler_button_1", NULL);
```

En la funció de exit ens trobem amb:

* gpio_set_value: Seteja el valor de una sortida, el fem servir per parar els leds.
```c
gpio_set_value(gpioLED1, led1On);
```

* gpio_unexport: Elimina els ports del sysfd.
```c
gpio_unexport(gpioLED1);
```

* gpio_free: Allibera els ports del sistema.
```c
gpio_free(gpioLED1);
```

* free_irq: Allibera la interrupció del sistema.
```c
free_irq(irqNumber1, NULL);
```

Finalment en el handler.

* call_usermodehelper: Funció que ens permet llançar una comanda desde fora l'espai de kernel peró estan dins de l'espai de kernel. Amb envp li especifiquem les variable d'entorn per executar la comanda. Amb argv li indiquem la comanda que volem executar, en aquest cas el nostre script. Amb UMH_NO_WAIT indiquem que no cal esperar a que s'acabi d'executar la comanda per a seguir l'execució del codi. Finalment rebem si hi ha hagut algun error. 
```c
static char *envp[] = {"HOME=/", "TERM=linux", "PATH=/sbin:/bin:/usr/sbin:/usr/bin", NULL };
char *argv1[] = {"/home/pi/F1/Scripts/button1.sh", NULL};
int err = 0;

err = call_usermodehelper(argv1[0], argv1, envp, UMH_NO_WAIT);

if (err != 0) {
    printk(KERN_INFO "ASO: ERROR on exec: %d", err);
}
```

## Problemes observats

<u>Connexio a la xarxa</u>

Un cop va estar tot configurat ja no va donar problemes. Pero per a connectar inicialment la raspberry a la red wifi del móbil vaig probar múltiples opcions. Aixi com multiples variacions de l'escript final.

<u>Versio de kernel</u>

En el makefile s'executa la comanda `uname -r` per a trobar la versió de kernel que hi ha instal·lada.

```
Man uname:
...
-r, --kernel-release
    print the kernel release
...
```

El problema el vaig tenir quan just connectar-me per primer cop vaig actualitzar tot el sistema amb `apt update` i `apt upgrade`. Seguidament vaig començar a fer el makefile, el codi, etc. I el makefile em donava un error dient que la carpeta ` /lib/modules/$(shell uname -r)` no existia. Finalment vaig trobar que `uname -r` no em retornava la versió mes actual. I a la carpeta `/lib/modules` si que trobavem la carpeta amb el nom de la versió més recent. 

Per sort ho vaig solucionar facilment reiniciant la raspberry.

<u>Executar l'escript</u>

En aquest cas vaig estar provant diverses funcions de la familia `usermodehelper`. Amb totes elles compilava be el codi, i s'instal·lava be. Peró en prémer qualsevol botó el kernel petava i la raspberry deixava de funcionar. El problema va resultar estar en un paràmetre que indica si vols esperar a que la commanda que estas executant acabi o no abans de seguir amb la execució del codi. Jo tenia posat que esperés. Peró es veu que posar aixó en el handler d'una interrupció fa petar el sistema. Per tant la solució va ser indicar a la funció que no esperes a que la comanda s'acabes d'executar. 

Finalment un últim error va ser no tenir el .sh amb permisos d'execució. 

## Conclusió

Un cop acabada la pràctica veig molt més clar qué és un modul de sistema aixi com les seves funcionalitats i utilitats. 

Al llarg de la practica he assolit molts coneixement, aixó ho atribueixo al gran nombre d'aspectes i àmbits en els que t'has de desenvolupar per a completar tots els petits reptes que et planteja aquest projecte.

---

2020-2021

Sistemes operatius avançats - SOA
   
[Bernat Segura]() - <bernat.segura@students.salle.url.edu>

---
