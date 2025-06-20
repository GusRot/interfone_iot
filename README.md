### README – Firmware “ESP32 Interfone IoT”

*(versão de testes com LEDs de feedback visual)*

---

## 1 . Visão geral

Este firmware transforma um ESP32 em um controlador do interfone:

1. **Toca o interfone** (simulado).
2. **Atende** a chamada por meio de um relé.
3. **Envia o tom DTMF “\*1”** para abrir o portão.
4. Tudo é **controlado via Home Assistant** usando MQTT.

Para facilitar testes sem hardware de áudio, dois LEDs substituem temporariamente o buzzer/DFPlayer:

| Ação real                 | Substituto visual | Pino    |
| ------------------------- | ----------------- | ------- |
| Toque do interfone (ring) | LED laranja       | GPIO 32 |
| Tom DTMF \*1              | LED rosa          | GPIO 33 |

---

## 2 . Por que LEDs em vez de som?

* **Limitação do simulador Wokwi:** o periférico LEDC/Tone do ESP32 para de funcionar depois que o Wi-Fi é inicializado, impedindo que o ícone de buzzer pisque.
* **Feedback rápido:** piscar LEDs garante que o fluxo possa ser validado por qualquer pessoa, mesmo sem áudio ou transformador acoplado.
* **Compatível com a placa real:** basta trocar as chamadas `blinkLED()` por `tone()`/DFPlayer quando o hardware definitivo estiver disponível.

---

## 3 . Pinagem utilizada

| Função                            | GPIO    | Observação                  |
| --------------------------------- | ------- | --------------------------- |
| Relé (gancho)                     | **14**  | Pulso HIGH/LOW para atender |
| LED “Sistema habilitado”          | **27**  | Azul                        |
| LED “Ring” (simula bip)           | **32**  | Laranja                     |
| LED “DTMF” (simula tom)           | **33**  | Rosa                        |
| — Reservados para etapa final —   |         |                             |
| Sensor KY-037 (detecção de toque) | 35      | Ainda não conectado         |
| DFPlayer Mini (TX/RX)             | 16 / 17 | Ainda não conectado         |

---

## 4 . Tópicos MQTT

| Tópico                   | Payload    | Função                                                                |
| ------------------------ | ---------- | --------------------------------------------------------------------- |
| `interfone_enable/set`   | `ON`/`OFF` | Liga ou desliga o sistema                                             |
| `interfone_ring_sim/set` | `ON`       | Dispara o fluxo de toque (apenas `ON`; volta a `OFF` automaticamente) |

*Discovery* do Home Assistant cria duas entidades “switch” correspondentes.

---

## 5 . Fluxo de estados

1. **Sistema desabilitado (`OFF`)**
   *LED azul apagado*
   • O LED Ring pisca, mas relé + DTMF **não** executam.
2. **Sistema habilitado (`ON`)**
   *LED azul aceso*
   • Ao receber comando de toque:

   1. LED Ring pisca duas vezes.
   2. Relé aciona (LED verde, se conectado).
   3. LED DTMF pisca duas vezes.
   4. Relé desliga.

---

## 6 . Compilação e simulação
### 6.1  Ambiente

* **PlatformIO** + framework *arduino-esp32*
* **Wokwi** para simulação rápida
*Para simular no Wokwi* não é preciso nenhuma flag extra, pois o firmware já usa apenas LEDs.


## 7 . Próximos passos

1. **Integrar KY-037**: disparo automático do fluxo quando o interfone real tocar.
2. **Substituir LEDs por DFPlayer Mini** + transformador 600 : 600 para injetar tom DTMF “\*1”.
3. **Ajustar pinagem** (relé → GPIO 25, DFPlayer em 16/17) conforme documento do hardware.

---

## 8 . Licença

Código liberado sob MIT. Use e modifique à vontade.