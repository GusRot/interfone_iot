# ESP32 Interfone IoT – README

Este projeto demonstra uma automação de interfone baseada em **ESP32** totalmente integrada ao **Home Assistant**.  A placa detecta um toque (simulado) do interfone, “atende” a ligação via relé e envia o comando DTMF `*1` (simulado por um buzzer) para abrir o portão do condomínio.

---

## 📐 Visão Geral

| Recurso                   | Como é implementado                                                         |
| ------------------------- | --------------------------------------------------------------------------- |
| **Atender o interfone**   | Relé no pino 14 que fecha o gancho do telefone                              |
| **Envio de DTMF**         | Buzzer no pino 25 reproduz os tons correspondentes a `*` e `1`              |
| **Habilitar/Desabilitar** | Switch `interfone_enable` no painel do HA; LED azul (pino 27) indica estado |
| **Simular toque**         | Switch `interfone_ring_sim` aciona todo o fluxo (para testes)               |
| **Logs**                  | Mensagens via `Serial.println()` visíveis no monitor serial                 |

---

## 🛠 Componentes

| Hardware                  | Função                                |
| ------------------------- | ------------------------------------- |
| ESP32 DevKit v4           | Microcontrolador Wi‑Fi                |
| Relé 1 canal (5 V)        | Atender o telefone (pino 14)          |
| Buzzer passivo            | Sons de toque e DTMF (pino 25)        |
| LED azul + resistor 220 Ω | Indica “sistema habilitado” (pino 27) |
| Fonte 5 V USB             | Alimentação do ESP32                  |

> Para a simulação no **Wokwi** usa‑se apenas buzzer, dois LEDs e o relé é representado pelo LED verde.

---

## ⚡ Ligações (resumo)

* **P25 →** Buzzer SIG   (+ GND)
* **P14 →** Relé IN (ou LED verde A)
* **P27 →** LED azul A   (+ resistor → GND)
* **GND →** GND de todos os periféricos
* **TX/RX** ligados ao `$serialMonitor` no Wokwi para exibir logs

---

## 💾 Código–chave (`main.cpp`)

* **`playTone()`** – wrapper sobre LEDC (`ledcWriteTone`) para gerar som.
* **`tocarRing()` / `tocarDTMF()`** – sequências de tons.
* **`simularProcessoInterfone()`** – executa todo o fluxo (ring → relé → DTMF).
* **MQTT/HA** – dois `HASwitch` controlam habilitação e simulação.

---

## 🚀 Como testar

1. **Clonar** o projeto (PlatformIO ou Arduino IDE).
2. Ajustar `WIFI_SSID/WIFI_PASSWORD` e broker (`BROKER_ADDR`).
3. Carregar na placa **ESP32** ou abrir no **Wokwi** com o `diagram.json` fornecido.
4. No Home Assistant:

   * Adicionar a integração **MQTT** (se necessário).
   * Dois botões surgirão automaticamente via auto‑discovery:

     * `Habilitar Interfone`
     * `Simular Toque de Interfone`
5. Clique em **Habilitar Interfone**: LED azul acende.
6. Clique em **Simular Toque**: você ouvirá o toque, o LED verde (relé) acende, depois o tom DTMF é tocado.
7. Observe o **Serial Monitor** para logs detalhados.

---

## 📝 Personalizações

* **Duração dos tons**: ajustar tempos em `tocarRing()` / `tocarDTMF()`.
* **Frequências reais DTMF**: usar duas saídas DAC ou mixer externo para reproduzir pares de tons.
* **Relé físico**: ligar o módulo 5 V ao botão de gancho do telefone.
* **DFPlayer Mini**: substituir o buzzer para reproduzir áudio real `*1.mp3`.

---

## 🧑‍💻 Licença

Projeto de demonstração para fins acadêmicos – livre para modificar e reutilizar.
