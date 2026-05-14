#include "MotorController.h"

// ============================================================================
// Construtor
// Inicializa velocidades e estado PD como zero.
// Os pinos são configurados em initialize() — não aqui.
// ============================================================================
MotorController::MotorController()
    : _currentLeft(0), _currentRight(0),
      _prevError(0.0f), _lastPDTime(0) {}

// ============================================================================
// initialize()
// Configura todos os pinos do L298N como OUTPUT e para os motores.
// Deve ser chamado uma vez no setup() antes de qualquer movimento.
// ============================================================================
void MotorController::initialize() {
    pinMode(PIN_IN1, OUTPUT);
    pinMode(PIN_IN2, OUTPUT);
    pinMode(PIN_IN3, OUTPUT);
    pinMode(PIN_IN4, OUTPUT);
    pinMode(PIN_ENA, OUTPUT);
    pinMode(PIN_ENB, OUTPUT);
    stop();   // garante que os motores iniciam parados
}

// ============================================================================
// setMotorSpeed()
// Ponto central de controle — todos os métodos de movimento passam por aqui.
// Armazena velocidades e repassa para setPinDirection() de cada motor.
//
// Convenção de sinal (motor esquerdo invertido no chassi):
//   Esquerdo: negativo = frente | positivo = ré
//   Direito:  positivo = frente | negativo = ré
// ============================================================================
void MotorController::setMotorSpeed(int speedLeft, int speedRight) {
    _currentLeft  = speedLeft;
    _currentRight = speedRight;
    setPinDirection(PIN_IN3, PIN_IN4, PIN_ENA, speedLeft);   // motor esquerdo
    setPinDirection(PIN_IN1, PIN_IN2, PIN_ENB, speedRight);  // motor direito
}

// ============================================================================
// move()
// Aplica velocidade uniforme nos dois motores para movimentos predefinidos.
// Os sinais negativos/positivos compensam a inversão do motor esquerdo.
// ============================================================================
void MotorController::move(Direction direction, uint8_t speed) {
    speed = constrain(speed, 0, 255);
    switch (direction) {
        case FORWARD:    setMotorSpeed(-speed,  speed); break;  // esq ré → frente real
        case BACKWARD:   setMotorSpeed( speed, -speed); break;
        case TURN_LEFT:  setMotorSpeed(-speed, -speed); break;  // ambos no mesmo sentido
        case TURN_RIGHT: setMotorSpeed( speed,  speed); break;
        default:         stop(); break;
    }
}

// ============================================================================
// stop()
// Para ambos os motores imediatamente.
// ============================================================================
void MotorController::stop() { setMotorSpeed(0, 0); }

// ============================================================================
// curveCompensated()
// Realiza curva com diferença de velocidade entre motores:
//   Motor externo = speed (velocidade plena)
//   Motor interno = speed * compensationFactor (reduzido)
//
// Usado nos testes manuais. O followLine() tem sua própria compensação via PD.
// ============================================================================
void MotorController::curveCompensated(Direction direction, uint8_t speed,
                                        float compensationFactor) {
    int outer = constrain((int)speed, 0, 255);
    int inner = constrain((int)(speed * compensationFactor), 0, 255);
    applyDeadzoneCorrection(outer);
    applyDeadzoneCorrection(inner);

    // Motor interno está no lado da curva — recebe velocidade reduzida
    if      (direction == TURN_LEFT)  setMotorSpeed(-inner,  outer);
    else if (direction == TURN_RIGHT) setMotorSpeed(-outer,  inner);
}

// ============================================================================
// followLine()
// Controlador PD de seguimento de linha.
//
// Como funciona:
//   error      = posição atual da linha (0 = centrado, meta sempre = 0)
//   derivative = variação do erro entre ciclos (detecta velocidade de mudança)
//   correction = Kp * error + Kd * derivative
//
//   Motor externo à curva: mantém baseSpeed (não reduz)
//   Motor interno à curva: reduzido por (1 - |correção normalizada|)
//     → mínimo PD_MIN_INNER_SPEED para garantir torque em curvas fechadas
//
// Por que não reduzir o externo?
//   Reduzir o motor externo diminuiria a velocidade média do robô,
//   além de poder causar parada em curvas onde a correção é alta.
//   Manter o externo constante preserva a velocidade de avanço.
//
// Temporização:
//   Executa apenas se PD_SAMPLE_MS ms passaram desde o último cálculo.
//   Isso desacopla a frequência do loop da frequência do controlador.
// ============================================================================
void MotorController::followLine(float linePosition, uint8_t baseSpeed) {
    unsigned long now = millis();

    // Limita a frequência de atualização — evita derivativo instável em loops rápidos
    if (now - _lastPDTime < PD_SAMPLE_MS) return;
    _lastPDTime = now;

    float error      = linePosition;
    float derivative = error - _prevError;                          // variação do erro
    float correction = (PD_KP * error) + (PD_KD * derivative);     // saída do PD
    _prevError       = error;                                       // armazena para próximo ciclo

    // Normaliza correção para 0.0..1.0 — representa o quanto reduzir o motor interno
    float corrNorm = constrain(fabs(correction), 0.0f, 1.0f);

    int outerSpeed = (int)baseSpeed;
    int innerSpeed = (int)(baseSpeed * (1.0f - corrNorm));
    innerSpeed     = constrain(innerSpeed, PD_MIN_INNER_SPEED, (int)baseSpeed);

    // Sinal positivo = linha à direita → curva à direita → esquerdo é externo
    // Sinal negativo = linha à esquerda → curva à esquerda → direito é externo
    int leftSpeed, rightSpeed;
    if (correction >= 0.0f) {
        leftSpeed  = outerSpeed;   // esquerdo externo
        rightSpeed = innerSpeed;
    } else {
        leftSpeed  = innerSpeed;
        rightSpeed = outerSpeed;   // direito externo
    }

    setMotorSpeed(-leftSpeed, rightSpeed);  // aplica convenção de inversão do motor esq

    if (DEBUG_MODE) {
        static unsigned long lastDbg = 0;
        if (now - lastDbg >= 200) {   // limita impressão a 5Hz para não saturar serial
            lastDbg = now;
            Serial.print(F("[PD] err="));  Serial.print(error, 3);
            Serial.print(F(" cor="));      Serial.print(correction, 3);
            Serial.print(F(" L="));        Serial.print(leftSpeed);
            Serial.print(F(" R="));        Serial.println(rightSpeed);
        }
    }
}

// ============================================================================
// resetPD()
// Zera o erro anterior e o timestamp.
// Deve ser chamado sempre que o seguimento for interrompido (parada,
// recuperação, coleta) para evitar spike no termo derivativo na retomada.
// ============================================================================
void MotorController::resetPD() {
    _prevError  = 0.0f;
    _lastPDTime = 0;
}

// ============================================================================
// applyDeadzoneCorrection()
// Eleva o valor ao mínimo PWM_MIN_DEADZONE se estiver abaixo do limiar.
// Abaixo desse valor, os motores TT recebem sinal mas não vencem o atrito,
// gerando calor sem movimento — a correção elimina essa zona morta.
// ============================================================================
void MotorController::applyDeadzoneCorrection(int& v) const {
    if      (v > 0 && v <  PWM_MIN_DEADZONE) v =  PWM_MIN_DEADZONE;
    else if (v < 0 && v > -PWM_MIN_DEADZONE) v = -PWM_MIN_DEADZONE;
}

// ============================================================================
// setPinDirection()
// Traduz velocidade com sinal para sinais digitais do L298N e PWM.
//
// L298N usa dois pinos de direção por motor:
//   speed > 0: IN1=HIGH IN2=LOW  → frente
//   speed < 0: IN1=LOW  IN2=HIGH → ré
//   speed = 0: IN1=LOW  IN2=LOW  → freio (ambos LOW = curto-circuito no motor)
//
// O valor PWM é sempre o absoluto da velocidade — direção é definida pelos INs.
// ============================================================================
void MotorController::setPinDirection(int in1, int in2, int pwmPin, int speed) {
    int pwm = constrain(abs(speed), 0, 255);
    if (speed == 0) {
        digitalWrite(in1, LOW);  digitalWrite(in2, LOW);  analogWrite(pwmPin, 0);
    } else if (speed > 0) {
        digitalWrite(in1, HIGH); digitalWrite(in2, LOW);  analogWrite(pwmPin, pwm);
    } else {
        digitalWrite(in1, LOW);  digitalWrite(in2, HIGH); analogWrite(pwmPin, pwm);
    }
}
