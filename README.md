
# 💣 Airsoft Bomb Simulator (Wokwi)

## 🎮 Comandi Veloci

### 🔐 Codici Default
| Azione         | Codice    |
|----------------|-----------|
| **Arma Bomba** | `000000`  |
| **Disarma**    | `123456`  |
| **Login DEV**  | `123456`  |

### 🕹️ Controlli
| Tasto   | Funzione          | Note       |
|---------|-------------------|------------|
| **0-9** | Inserisci numeri  |            |
| **\***  | **Conferma**      | Tasto invio/OK |
| **#**   | **Cancella**      | Resetta input corrente |
| **D**   | **Disarma**       | Solo quando armata |
| **A** | **Dev Mode** | Premi per accedere |

### 🛠️ Modalità Dev (Sviluppatore)
1. Nella schermata iniziale, **premi 'A'** per accedere al login
2. Inserisci la password DEV: `123456` + `*`
3. Usa il menu:
   - **B**: Imposta **Timer** (in secondi) → `B` per salvare
   - **C**: Imposta **Codice Disarmo** → `C` per salvare
   - **A**: Esci e torna al menu principale

### 📝 Variabili di Configurazione (Sketch)
Ecco le variabili principali che puoi modificare all'inizio del file `sketch.ino` per personalizzare il comportamento:

```cpp
// Tempo iniziale della bomba (in secondi)
// 300 = 5 minuti. Modifica questo valore per partite più lunghe o corte.
unsigned long bombTimer = 300;

// Password per ARMARE la bomba (Default: 000000)
// Questa è la password che i terroristi usano per attivare la bomba.
const char armPassword[] = "000000";

// Password per accedere al menu SVILUPPATORE (Default: 123456)
// Serve per cambiare timer e codice disarmo senza riprogrammare.
const char devPassword[] = "123456";

// Password INIZIALE per DISARMARE (Default: 123456)
// Nota: Questa variabile può essere cambiata dal Menu Dev durante il gioco.
char disarmPassword[10] = "123456";

// Altre variabili di sistema (Non modificare)
// inputPassword[]: Buffer che memorizza i numeri digitati
// lastDisplayUpdate: Usa millis() per aggiornare lo schermo ogni secondo
// lastLedBlinkTime: Gestisce il lampeggio dei LED senza bloccare il loop
```

---

## ⚙️ Come Funziona il Codice

Il firmware è una **Macchina a Stati Finiti (FSM)** che gestisce la logica della bomba.

### 📊 Stati Principali
1. **S_IDLE**: In attesa del codice di armaggio (`000000`).
2. **S_ARMED**: Bomba attiva! Il timer scorre alla rovescia.
   - **> 90s**: LED rosso fisso.
   - **30-90s**: LED rosso lampeggia 1 volta/sec + Beep.
   - **< 30s**: LED e Buzzer veloci (2 volte/sec).
3. **S_DEFUSING**: Inserimento codice di disarmo.
4. **S_EXPLODED**: Timer scaduto. Suono continuo e LED lampeggianti per 10s.
5. **S_DEFUSED**: Bomba disattivata con successo. LED Verde attivo.

### 🔧 Gestione Memoria
Il codice è ottimizzato per **Arduino Uno** (2KB RAM):
- Stringhe salvate in Flash con macro `F()`
- Uso di `char array` invece di `String`
- Gestione timer non bloccante con `millis()`

## 🔌 Collegamenti
- **LCD**: I2C (SDA=A4, SCL=A5)
- **Tastiera**: Pin 2-9
- **LED**: R=10, Y=11, G=12
- **Buzzer**: Pin 13
