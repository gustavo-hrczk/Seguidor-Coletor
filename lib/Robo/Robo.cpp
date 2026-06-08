#include "Robo.h"

// ============================================================================
// Construtor
// ============================================================================
Robo::Robo() {}

// ============================================================================
// inicializar()
// ============================================================================
void Robo::inicializar() {
    _motor.initialize();
    _sensor.initialize();
    _ultrasonic.initialize();
    _gripper.initialize();

    _motor.stop();
    _gripper.open();
}

// ============================================================================
// motorEsquerdo() / motorDireito()
// Converte -100..+100 para PWM real e mantém o motor oposto inalterado.
// ============================================================================
void Robo::motorEsquerdo(int velocidade) {
    int pwm = _converterVelocidade(velocidade);
    _motor.setMotorSpeed(pwm, _motor.getRightSpeed());
}

void Robo::motorDireito(int velocidade) {
    int pwm = _converterVelocidade(velocidade);
    _motor.setMotorSpeed(_motor.getLeftSpeed(), pwm);
}

// ============================================================================
// pararMotores()
// ============================================================================
void Robo::pararMotores() {
    _motor.stop();
}

// ============================================================================
// seguirLinha()
// Lê a posição da linha e chama o controlador PID do MotorController.
//
// Por que o PID e não if/else?
//   If/else com velocidades fixas produz correção bang-bang: o robô
//   corrige demais, passa do centro, corrige no outro lado — oscilação
//   contínua. O PID aplica correção proporcional ao desvio: quanto mais
//   longe do centro, mais corrige; quanto mais próximo, menos — convergência
//   suave sem micro-passos.
//
// O aluno chama apenas seguirLinha(NORMAL) — a complexidade do PID
// permanece invisível dentro de followLine().
// ============================================================================
void Robo::seguirLinha(int velocidade) {
    _sensor.readSensors();
    float pos = _sensor.getLinePosition();   // -1.0..+1.0 para o PID interno

    // Converte velocidade do aluno para PWM real
    uint8_t pwm = (uint8_t)constrain(_converterVelocidade(velocidade), 0, 255);

    _motor.followLine(pos, pwm);
}

// ============================================================================
// resetarSeguimento()
// Zera o estado interno do PID (erro anterior, integral, timestamp).
// Sem este reset, o acúmulo da manobra anterior causa uma correção
// brusca imediatamente ao retomar o seguimento.
// ============================================================================
void Robo::resetarSeguimento() {
    _motor.resetPD();
    _sensor.resetLastDirection();
}

// ============================================================================
// lerLinha()
// ============================================================================
int Robo::lerLinha() {
    _sensor.readSensors();
    return (int)(_sensor.getLinePosition() * 100.0f);   // -100..+100
}

// ============================================================================
// temLinha()
// ============================================================================
bool Robo::temLinha() {
    _sensor.readSensors();
    return _sensor.isLineDetected();
}

// ============================================================================
// lerDistancia()
// ============================================================================
int Robo::lerDistancia() {
    return _ultrasonic.readDistance();
}

// ============================================================================
// leituraEstavelSensor()
// ============================================================================
bool Robo::leituraEstavelSensor() {
    return _ultrasonic.isReadingStable();
}

// ============================================================================
// fecharGarra() / abrirGarra() / garrataFechada()
// ============================================================================
void Robo::fecharGarra()      { _gripper.close();    }
void Robo::abrirGarra()       { _gripper.open();     }
bool Robo::garrataFechada()   { return _gripper.isClosed(); }

// ============================================================================
// _converterVelocidade()
// Mapeia -100..+100 para -BASE_SPEED..+BASE_SPEED com deadzone aplicada.
// ============================================================================
int Robo::_converterVelocidade(int velocidade) const {
    velocidade = constrain(velocidade, -100, 100);
    if (velocidade == 0) return 0;

    int pwm = (int)((float)velocidade / 100.0f * (float)BASE_SPEED);

    if      (pwm > 0 && pwm <  PWM_MIN_DEADZONE) pwm =  PWM_MIN_DEADZONE;
    else if (pwm < 0 && pwm > -PWM_MIN_DEADZONE) pwm = -PWM_MIN_DEADZONE;

    return pwm;
}
