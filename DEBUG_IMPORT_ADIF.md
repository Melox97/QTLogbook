# Debug Importazione ADIF - Analisi Problema

## 🔍 Problema Identificato

**Sintomo:** Solo 2 dei 3 record vengono importati dal file `test_import.adi`

## 📋 Analisi dei Record

### Record 1: IZ0ABC ✅ IMPORTATO
```
<CALL:6>IZ0ABC<QSO_DATE:8>20241201<TIME_ON:6>143000<BAND:3>20M<MODE:3>SSB<RST_SENT:2>59<RST_RCVD:2>59<DXCC:3>248<GRIDSQUARE:6>JN61FW<OPERATOR:6>IZ0XYZ<EOR>
```
- **Modo:** SSB (vocale)
- **RST:** 59/59 (2 cifre) ✅
- **Validazione:** PASSA

### Record 2: DL1DEF ✅ IMPORTATO
```
<CALL:6>DL1DEF<QSO_DATE:8>20241201<TIME_ON:6>144500<BAND:3>40M<MODE:2>CW<RST_SENT:3>599<RST_RCVD:3>579<DXCC:3>230<GRIDSQUARE:6>JO62KM<OPERATOR:6>IZ0XYZ<EOR>
```
- **Modo:** CW (digitale)
- **RST:** 599/579 (3 cifre) ✅
- **Validazione:** PASSA

### Record 3: G0GHI ❌ SCARTATO
```
<CALL:6>G0GHI<QSO_DATE:8>20241201<TIME_ON:6>150000<BAND:3>80M<MODE:4>FT8<RST_SENT:3>-10<RST_RCVD:3>-15<DXCC:3>223<GRIDSQUARE:6>IO91VX<OPERATOR:6>IZ0XYZ<EOR>
```
- **Modo:** FT8 (digitale)
- **RST:** -10/-15 (valori negativi) ❌
- **Validazione:** FALLISCE

## 🐛 Causa del Problema

**File:** `src/contact.cpp` - Metodo `Contact::isValid()`

**Validazione RST problematica:**
```cpp
// Linee 33-44
QRegularExpression rstRegex2Digits("^[1-5][1-9]$");     // 2 cifre per modi vocali
QRegularExpression rstRegex3Digits("^[1-5][1-9][1-9]$"); // 3 cifre per modi digitali/CW

bool isVoiceMode = (m_mode == "SSB" || m_mode == "AM" || m_mode == "FM" || 
                   m_mode == "USB" || m_mode == "LSB");

if (isVoiceMode) {
    // Modi vocali: accetta 2 cifre (es. 59)
    if (!rstRegex2Digits.match(m_rstSent).hasMatch() || !rstRegex2Digits.match(m_rstReceived).hasMatch()) {
        return false;
    }
} else {
    // Modi digitali/CW: richiede 3 cifre (es. 599)
    if (!rstRegex3Digits.match(m_rstSent).hasMatch() || !rstRegex3Digits.match(m_rstReceived).hasMatch()) {
        return false;
    }
}
```

**Problemi:**
1. **FT8 usa RST negativi:** I valori come `-10`, `-15` sono standard per FT8
2. **Regex troppo restrittiva:** `^[1-5][1-9][1-9]$` non accetta valori negativi
3. **Mancanza di supporto FT8:** FT8 non è considerato nei pattern di validazione

## 🔧 Soluzione Proposta

**Modificare la validazione RST per supportare:**
1. **Valori negativi per FT8/FT4:** Pattern come `-XX` o `+XX`
2. **Range esteso:** Da -30 a +30 per modi digitali moderni
3. **Validazione specifica per modo:** FT8/FT4 vs CW/RTTY vs SSB

**Nuovo pattern RST per FT8:**
```cpp
QRegularExpression rstRegexFT("^[+-]?[0-9]{1,2}$"); // Per FT8/FT4: -30 a +30
```

## 📊 Impatto

**Record attualmente scartati:**
- Tutti i QSO FT8/FT4 con RST negativi
- QSO con RST non conformi ai pattern rigidi

**Frequenza del problema:**
- **Alta** per FT8/FT4 (modi molto popolari)
- **Media** per altri modi digitali con RST non standard

---

**Conclusione:** Il problema è nella validazione RST troppo restrittiva che non supporta i valori negativi standard di FT8.