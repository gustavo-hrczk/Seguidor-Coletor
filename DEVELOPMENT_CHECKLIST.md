# ✅ Checklist Interativo de Desenvolvimento

## Etapa 0: Preparação (Pré-voo)

### Hardware
- [ ] Arduino UNO conectado via USB
- [ ] Motor esquerdo ligado a pinos 5, 7, 3 (PWM)
- [ ] Motor direito ligado a pinos 2, 4, 6 (PWM)
- [ ] 6 sensores QTR ligados em A0-A5
- [ ] HC-SR04 Trigger em 11, Echo em 12
- [ ] Servo garra ligado ao pino 9
- [ ] Power bank/bateria 5V testado

### Software
- [ ] PlatformIO instalado
- [ ] VS Code com extensão PlatformIO
- [ ] Arquivo config.h revisado
- [ ] Pinos confirmados em config.h

---

## Etapa 1: Sensores + Motores

**Objetivo:** Robô segue linha reta por 10 metros

### Testes Básicos
- [ ] Testar cada motor individualmente (test_components opção 1)
- [ ] Testar cada sensor de linha (test_components opção 2)
- [ ] Sensores reagem ao aproximar da linha
- [ ] Valores mudam quando linha passa por cima

### Calibração Inicial
- [ ] Executar test_components opção 5 (calibração)
- [ ] Anotar THRESHOLD recomendado
- [ ] Atualizar config.h com novo THRESHOLD
- [ ] Re-upload

### Teste de Navegação
- [ ] Upload main.cpp
- [ ] Posicionar robô sobre linha reta
- [ ] Verificar Serial Monitor
- [ ] Robô segue linha por ~2 metros
- [ ] Sem desvio lateral significativo
- [ ] Tempo: ~5-10 segundos para 2 metros

### Ajustes de Velocidade
- [ ] Se muito rápido: reduzir VELOCITY_GLOBAL em 20
- [ ] Se oscila: reduzir VELOCITY_GLOBAL em 30
- [ ] Se muito lento: aumentar VELOCITY_GLOBAL em 10

**✅ Etapa 1 Completa** quando: Robô segue 10m de reta sem parar

---

## Etapa 2: Detecção de Objetos

**Objetivo:** Robô detecta objeto sem parar ou girar

### Teste Ultrassônico
- [ ] Testar sensor isoladamente (test_components opção 3)
- [ ] Aproximar objeto lentamente
- [ ] Leituras mudam suavemente
- [ ] Sem saltos abruptos (outliers)

### Calibração Ultrassônico
- [ ] Ajustar ULTRASONIC_DISTANCE_LONG (início detecção)
- [ ] Ajustar ULTRASONIC_DISTANCE_SHORT (fase 2)
- [ ] Ajustar ULTRASONIC_DISTANCE_CONTACT (coleta)

### Teste de Detecção
- [ ] Posicionar objeto a 40cm do robô
- [ ] Robô em linha reta navegando
- [ ] Objetivo: detecta e reduz velocidade
- [ ] NÃO deve parar ou girar neste ponto
- [ ] Apenas diminui velocidade

### Bloqueio de Detecção em Curva
- [ ] Posicionar objeto durante curva acentuada
- [ ] Robô DEVE IGNORAR objeto (não coletar)
- [ ] Continue navegando na curva
- [ ] Este é o BLOQUEIO 1 (segurança)

**✅ Etapa 2 Completa** quando: Detecta objeto em reta, ignora em curva

---

## Etapa 3: Coleta Completa

**Objetivo:** Coleta → Rotação → Descarte sincronizado

### Teste Servo
- [ ] Testar servo isoladamente (test_components opção 4)
- [ ] Abre completamente
- [ ] Fecha completamente
- [ ] Sem travar
- [ ] Tempo de fechamento < 3 segundos

### Teste de Coleta Simples
- [ ] Objeto a ~5cm do robô
- [ ] Robô aproxima
- [ ] Garra fecha (você pode sentir a força)
- [ ] Garra abre (completo)

### Teste de Rotação 90°
- [ ] Marque um ponto no chão
- [ ] Posicione robô apontando para um lado
- [ ] Execute rotação
- [ ] Meça ângulo rotacionado
- [ ] Ajuste ROTATION_90_DEGREES_TIME se necessário
- [ ] Objetivo: ±5° de 90°

### Teste de Descarte
- [ ] Coleta objeto
- [ ] Rotaciona 90°
- [ ] Abre garra (descarta)
- [ ] Recua um pouco

### Teste de Retorno à Linha
- [ ] Após descartar
- [ ] Robô procura linha
- [ ] Encontra linha
- [ ] Centraliza
- [ ] Continua navegando

**✅ Etapa 3 Completa** quando: Ciclo coleta-descarte-retorno funciona

---

## Etapa 4: Testes Progressivos

**Objetivo:** Aumentar complexidade gradualmente

### Teste 1: Linha Reta 10m
- [ ] Robô segue em reta
- [ ] Sem sair da linha
- [ ] Tempo: 20-30 segundos
- [ ] 0 oscilações

### Teste 2: Linha em "S" (10m)
- [ ] Robô faz 2 curvas grandes
- [ ] Segue curva suave
- [ ] Sem sair da linha
- [ ] Tempo: 30-45 segundos

### Teste 3: Cruzamento (Round Robin)
- [ ] Linha em formato "+"
- [ ] Robô chega na intersecção
- [ ] Escolhe uma direção (ex: direita)
- [ ] Próxima intersecção: escolhe esquerda
- [ ] Sem oscilação

### Teste 4: Objeto em Reta
- [ ] Coloca 1 objeto em linha reta
- [ ] Robô detecta → aproxima → coleta → descarta → continua
- [ ] Sem parar ou perder linha

### Teste 5: Múltiplos Objetos (3-5)
- [ ] Coloca 3-5 objetos ao longo pista
- [ ] Coleta todos sequencialmente
- [ ] Taxa de sucesso: 100%

### Teste 6: Linha Perdida
- [ ] Coloque pista com falha (~30cm)
- [ ] Robô chega ao fim da linha
- [ ] Começa busca (rotação)
- [ ] Encontra linha nova
- [ ] Continua navegando
- [ ] Taxa de sucesso: 8/10 tentativas

**✅ Etapa 4 Completa** quando: Todos os 6 testes passam

---

## Etapa 5: Teste Final

**Objetivo:** Validação completa do sistema

### Teste de Resistência (10 minutos)
- [ ] Pista de 25-30 metros
- [ ] 5+ objetos distribuídos
- [ ] Robô executa por 10 minutos
- [ ] Coleta 5/5 objetos (100%)
- [ ] Sem travamentos
- [ ] Sem perder linha (permanente)

### Monitoramento
- [ ] Serial Monitor ligado durante teste
- [ ] Acompanhar estados sendo impresso
- [ ] Anotar qualquer erro ou comportamento anômalo
- [ ] Verificar estatísticas finais

### Validação de Sucesso
- [ ] Distância total: 30m sem parar ✓
- [ ] Coleta: 5/5 objetos (100%) ✓
- [ ] Recuperação: 8/10 linhas perdidas ✓
- [ ] Servo: nunca travou ✓
- [ ] Rotação: 90° com precisão ✓
- [ ] Cruzamento: sem oscilação ✓
- [ ] Duração: 10 minutos contínuos ✓

**✅ Etapa 5 Completa** quando: Todos os 7 critérios passam

---

## Otimizações Opcionais

### Performance
- [ ] Aumentar VELOCITY_GLOBAL para testar limites
- [ ] Reduzir SENSOR_FILTER_CYCLES (mais responsivo)
- [ ] Reduzir CYCLE_MAIN (menos delay)

### Precisão
- [ ] Aumentar SENSOR_FILTER_CYCLES (mais estável)
- [ ] Ajustar CURVE_COMPENSATION_* (raio constante)
- [ ] Refinar ROTATION_90_DEGREES_TIME (giro exato)

### Robustez
- [ ] Aumentar SERVO_TIMEOUT (tolerar servo lento)
- [ ] Aumentar ULTRASONIC_NOISE_TOLERANCE (menos sensível)
- [ ] Aumentar LINE_SEARCH_TIMEOUT (mais tempo procurando)

---

## Troubleshooting Rápido Durante Testes

### Se Robô Não Se Move
```
[ ] Pinos motores estão corretos?
[ ] Bateria tem carga?
[ ] Motor gira manualmente?
[ ] VELOCITY_GLOBAL > 60?
```

### Se Não Detecta Linha
```
[ ] Threshold calibrado?
[ ] Sensores limpinhos?
[ ] Linha tem contraste com fundo?
[ ] Sensores estão virados para baixo?
```

### Se Oscila Demais
```
[ ] VELOCITY_GLOBAL muito alto?
[ ] CURVE_COMPENSATION muito baixo?
[ ] Linha muito fina?
[ ] Sensores mal posicionados?
```

### Se Não Coleta
```
[ ] HC-SR04 ligado?
[ ] Servo ligado?
[ ] Objeto perto o suficiente?
[ ] Garra está aberta antes de coletar?
```

### Se Perde Linha Frequentemente
```
[ ] VELOCITY_GLOBAL muito alto?
[ ] Threshold ruim?
[ ] Sensores sujos?
[ ] Linha desenhada ruim?
```

---

## Documentação de Referência

Consulte durante desenvolvimento:
- [ ] `README.md` - Visão geral
- [ ] `QUICK_START.md` - Início rápido
- [ ] `CALIBRATION.md` - Calibração detalhada
- [ ] `STATE_MACHINE_DIAGRAMS.md` - Fluxogramas
- [ ] `MENTAL_MAP.md` - Mapa estrutural
- [ ] `config.h` - Parâmetros

---

## Notas Pessoais

```
Data de Início: _______________
Primeira Compilação: _______________
Primeiro Movimento: _______________
Primeira Calibração: _______________
Primeira Coleta: _______________
Data de Conclusão: _______________

Problemas Encontrados:
- _________________________________
- _________________________________
- _________________________________

Soluções Aplicadas:
- _________________________________
- _________________________________
- _________________________________

Observações Finais:
_________________________________
_________________________________
_________________________________
```

---

## 🏁 Resumo Visual de Progresso

```
Etapa 1 (Sensores+Motores)    ████████████░░░░░░░░░░░░░░ XX%
Etapa 2 (Detecção)             ░░░░░░░░░░░░░░░░░░░░░░░░░░ 0%
Etapa 3 (Coleta)               ░░░░░░░░░░░░░░░░░░░░░░░░░░ 0%
Etapa 4 (Testes Prog.)         ░░░░░░░░░░░░░░░░░░░░░░░░░░ 0%
Etapa 5 (Final)                ░░░░░░░░░░░░░░░░░░░░░░░░░░ 0%

PROJETO TOTAL                  ████░░░░░░░░░░░░░░░░░░░░░░ 20%
```

---

**Boa sorte! Você consegue! 🚀** 

Volte a este checklist regularmente para rastrear progresso.
