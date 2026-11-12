#include "core/CircuitSimulator.h"
#include <iostream>

void CircuitSimulator::registerComponent(HardwareComponent* comp) {
    if (comp) {
        componentMap[comp->code] = comp;
    }
}

HardwareComponent* CircuitSimulator::getComponent(const std::string& code) {
    auto it = componentMap.find(code);
    return (it != componentMap.end()) ? it->second : nullptr;
}

void CircuitSimulator::addWire(const std::string& name,
                              const std::string& fromComp, const std::string& fromPin,
                              const std::string& toComp, const std::string& toPin,
                              const Vec3& color) {
    Wire w;
    w.name = name;
    w.fromCompCode = fromComp;
    w.fromPinName = fromPin;
    w.toCompCode = toComp;
    w.toPinName = toPin;
    w.color = color;
    w.active = false;
    wires.push_back(w);
}

void CircuitSimulator::setupStandardCarCircuit() {
    wires.clear();

    Vec3 red7V4(0.92f, 0.18f, 0.18f); // Đỏ bus 7.4V
    Vec3 orange5V(1.0f, 0.55f, 0.12f); // Cam bus 5V
    Vec3 blackGND(0.15f, 0.15f, 0.15f); // Đen GND
    Vec3 blueSig(0.2f, 0.65f, 0.95f);  // Xanh dương tín hiệu
    Vec3 yellowPWM(0.95f, 0.85f, 0.15f);// Vàng PWM

    // 1. Tuyến nguồn chính 7.4V từ Hộp pin BAT_SW -> DOMINO
    addWire("W_BAT_SW_7V4", "BAT_SW", "SW_OUT", "DOMINO", "IN_7V4", red7V4);
    addWire("W_BAT_SW_GND", "BAT_SW", "BAT-",   "DOMINO", "IN_GND", blackGND);

    // 2. Tuyến phân phối 7.4V từ DOMINO -> ARD, L298N, BUCK
    addWire("W_DOM_TO_ARD_VIN",  "DOMINO", "OUT_ARD",   "ARD",   "VIN", red7V4);
    addWire("W_DOM_TO_ARD_GND",  "DOMINO", "GND_ARD",   "ARD",   "GND", blackGND);

    addWire("W_DOM_TO_L298N_12V","DOMINO", "OUT_L298N", "L298N", "12V", red7V4);
    addWire("W_DOM_TO_L298N_GND","DOMINO", "GND_L298N", "L298N", "GND", blackGND);

    addWire("W_DOM_TO_BUCK_IN",  "DOMINO", "OUT_BUCK",  "BUCK",  "IN+", red7V4);
    addWire("W_DOM_TO_BUCK_GND", "DOMINO", "GND_BUCK",  "BUCK",  "IN-", blackGND);

    // 3. Tuyến hạ áp 5.0V từ BUCK -> DOMINO (Bus 5V)
    addWire("W_BUCK_TO_DOM_5V",  "BUCK",   "OUT+",      "DOMINO", "IN_BUCK_5V", orange5V);

    // 4. Tuyến cấp nguồn 5.0V từ DOMINO -> 3 Servo MG996R
    addWire("W_DOM_TO_SRV_BASE", "DOMINO", "OUT_SRV1",  "SRV_BASE",  "VCC", orange5V);
    addWire("W_DOM_TO_SRV_ELBOW","DOMINO", "OUT_SRV2",  "SRV_ELBOW", "VCC", orange5V);
    addWire("W_DOM_TO_SRV_GRIP", "DOMINO", "OUT_SRV3",  "SRV_GRIP",  "VCC", orange5V);

    // 5. Tuyến nguồn 5V logic từ Arduino LDO -> L298N (Logic 5V)
    addWire("W_ARD_TO_L298N_5V",   "ARD", "5V",  "L298N",  "5V",  orange5V);

    // 6. Tuyến điều khiển cầu H L298N từ Arduino
    addWire("W_ARD_ENA", "ARD", "D5",  "L298N", "ENA", yellowPWM);
    addWire("W_ARD_IN1", "ARD", "D2",  "L298N", "IN1", blueSig);
    addWire("W_ARD_IN2", "ARD", "D4",  "L298N", "IN2", blueSig);
    addWire("W_ARD_IN3", "ARD", "D12", "L298N", "IN3", blueSig);
    addWire("W_ARD_IN4", "ARD", "D13", "L298N", "IN4", blueSig);
    addWire("W_ARD_ENB", "ARD", "D6",  "L298N", "ENB", yellowPWM);

    // 8. Tuyến công suất động cơ L298N -> MOT_L, MOT_R
    addWire("W_L298N_MOTL_A", "L298N", "OUT1", "MOT_L", "A", blueSig);
    addWire("W_L298N_MOTL_B", "L298N", "OUT2", "MOT_L", "B", blueSig);
    addWire("W_L298N_MOTR_A", "L298N", "OUT3", "MOT_R", "A", blueSig);
    addWire("W_L298N_MOTR_B", "L298N", "OUT4", "MOT_R", "B", blueSig);

    // 9. Tuyến tín hiệu PWM điều khiển 3 Servo từ Arduino
    addWire("W_ARD_SRV_BASE",  "ARD", "D9",  "SRV_BASE",  "SIG", yellowPWM);
    addWire("W_ARD_SRV_ELBOW", "ARD", "D10", "SRV_ELBOW", "SIG", yellowPWM);
    addWire("W_ARD_SRV_GRIP",  "ARD", "D11", "SRV_GRIP",  "SIG", yellowPWM);
}

void CircuitSimulator::stepCircuit(float dt) {
    // 1. Lan truyền điện áp tuần tự giữa các linh kiện
    auto* bat   = dynamic_cast<BatterySwitchComp*>(getComponent("BAT_SW"));
    auto* domino= dynamic_cast<DominoBusComp*>(getComponent("DOMINO"));
    auto* buck  = dynamic_cast<BuckLM2596Comp*>(getComponent("BUCK"));
    auto* ard   = dynamic_cast<ArduinoUnoComp*>(getComponent("ARD"));
    auto* l298n = dynamic_cast<L298NDriverComp*>(getComponent("L298N"));
    auto* hcsr04= dynamic_cast<UltrasonicComp*>(getComponent("HCSR04"));
    auto* motL  = dynamic_cast<DCMotorComp*>(getComponent("MOT_L"));
    auto* motR  = dynamic_cast<DCMotorComp*>(getComponent("MOT_R"));
    auto* srvBase = dynamic_cast<ServoMG996RComp*>(getComponent("SRV_BASE"));
    auto* srvElbow= dynamic_cast<ServoMG996RComp*>(getComponent("SRV_ELBOW"));
    auto* srvGrip = dynamic_cast<ServoMG996RComp*>(getComponent("SRV_GRIP"));

    if (bat) bat->onElectricalUpdate();

    // Lan truyền BAT_SW -> DOMINO
    if (bat && domino) {
        Pin* swOut = bat->getPin("SW_OUT");
        Pin* domIn7 = domino->getPin("IN_7V4");
        if (swOut && domIn7) domIn7->voltage = swOut->voltage;
        domino->onElectricalUpdate();
    }

    // Lan truyền DOMINO -> BUCK
    if (domino && buck) {
        Pin* domBuck = domino->getPin("OUT_BUCK");
        Pin* buckIn  = buck->getPin("IN+");
        if (domBuck && buckIn) buckIn->voltage = domBuck->voltage;
        buck->onElectricalUpdate();

        // BUCK -> DOMINO (Bus 5V)
        Pin* buckOut = buck->getPin("OUT+");
        Pin* domIn5  = domino->getPin("IN_BUCK_5V");
        if (buckOut && domIn5) domIn5->voltage = buckOut->voltage;
        domino->onElectricalUpdate();
    }

    // Lan truyền DOMINO -> ARD
    if (domino && ard) {
        Pin* domArd = domino->getPin("OUT_ARD");
        Pin* ardVin = ard->getPin("VIN");
        if (domArd && ardVin) ardVin->voltage = domArd->voltage;
        ard->onElectricalUpdate();
    }

    // Lan truyền ARD -> HCSR04 & L298N (Logic 5V)
    if (ard) {
        Pin* ard5V = ard->getPin("5V");
        float v5 = ard5V ? ard5V->voltage : 0.0f;
        if (hcsr04) {
            Pin* vcc = hcsr04->getPin("VCC");
            if (vcc) vcc->voltage = v5;
            hcsr04->onElectricalUpdate();
        }
        if (l298n) {
            Pin* p5 = l298n->getPin("5V");
            if (p5) p5->voltage = v5;
        }
    }

    // Lan truyền DOMINO -> L298N (Power 12V/7.4V)
    if (domino && l298n) {
        Pin* domL298 = domino->getPin("OUT_L298N");
        Pin* l298_12 = l298n->getPin("12V");
        if (domL298 && l298_12) l298_12->voltage = domL298->voltage;
    }

    // Lan truyền ARD tín hiệu -> L298N
    if (ard && l298n) {
        auto copySignal = [&](const std::string& ardPin, const std::string& l298Pin) {
            Pin* p1 = ard->getPin(ardPin);
            Pin* p2 = l298n->getPin(l298Pin);
            if (p1 && p2) {
                p2->voltage = ard->isPowered ? p1->voltage : 0.0f;
                p2->pwmDuty = ard->isPowered ? p1->pwmDuty : 0.0f;
            }
        };
        copySignal("D5",  "ENA");
        copySignal("D2",  "IN1");
        copySignal("D4",  "IN2");
        copySignal("D6",  "ENB");
        copySignal("D12", "IN3");
        copySignal("D13", "IN4");
        l298n->onElectricalUpdate();
    }

    // L298N -> MOT_L, MOT_R
    if (l298n && motL && motR) {
        // Cập nhật điện áp chân A và B của động cơ
        Pin* o1 = l298n->getPin("OUT1");
        Pin* o2 = l298n->getPin("OUT2");
        Pin* motLA = motL->getPin("A");
        Pin* motLB = motL->getPin("B");
        if (motLA && o1) motLA->voltage = o1->voltage;
        if (motLB && o2) motLB->voltage = o2->voltage;

        Pin* o3 = l298n->getPin("OUT3");
        Pin* o4 = l298n->getPin("OUT4");
        Pin* motRA = motR->getPin("A");
        Pin* motRB = motR->getPin("B");
        if (motRA && o3) motRA->voltage = o3->voltage;
        if (motRB && o4) motRB->voltage = o4->voltage;

        // Cập nhật động học và trạng thái nguồn từ module cầu H L298N
        motL->updatePhysics(dt, l298n->outVoltL, l298n->isPowered);
        motR->updatePhysics(dt, l298n->outVoltR, l298n->isPowered);
    }

    // DOMINO 5V & ARD PWM -> 3 Servo
    auto updateServo = [&](ServoMG996RComp* srv, const std::string& domOutPin, const std::string& ardPin) {
        if (!srv) return;
        if (domino) {
            Pin* pVcc = srv->getPin("VCC");
            Pin* pDom = domino->getPin(domOutPin);
            if (pVcc && pDom) pVcc->voltage = pDom->voltage;
        }
        if (ard) {
            Pin* pSig = srv->getPin("SIG");
            Pin* pArd = ard->getPin(ardPin);
            if (pSig && pArd) {
                pSig->voltage = ard->isPowered ? 5.0f : 0.0f;
                pSig->pwmDuty = pArd->pwmDuty;
            }
        }
        srv->onElectricalUpdate();
    };

    updateServo(srvBase,  "OUT_SRV1", "D9");
    updateServo(srvElbow, "OUT_SRV2", "D10");
    updateServo(srvGrip,  "OUT_SRV3", "D11");

    // 2. Tính toán tổng dòng điện tiêu thụ (mA)
    if (!bat || !bat->switchOn) {
        totalCurrentMa = 0.0f;
    } else {
        float i = 45.0f; // Dòng tĩnh Arduino + LED
        if (buck && buck->isPowered) i += 12.0f;
        if (hcsr04 && hcsr04->isPowered) i += 15.0f;
        if (motL) i += std::fabs(motL->rpm) * 2.8f;
        if (motR) i += std::fabs(motR->rpm) * 2.8f;
        if (srvBase && srvBase->isPowered)  i += 40.0f;
        if (srvElbow && srvElbow->isPowered) i += 50.0f;
        if (srvGrip && srvGrip->isPowered)  i += 35.0f;
        totalCurrentMa = i;
    }

    // Cập nhật trạng thái active cho từng dây
    for (auto& w : wires) {
        HardwareComponent* src = getComponent(w.fromCompCode);
        if (src && src->isPowered) {
            Pin* p = src->getPin(w.fromPinName);
            w.active = (p && (p->voltage > 0.5f || p->pwmDuty > 0.0f));
        } else {
            w.active = false;
        }
    }
}

void CircuitSimulator::render3DWires() const {
    if (!showWires) return;

    for (const auto& w : wires) {
        auto it1 = componentMap.find(w.fromCompCode);
        auto it2 = componentMap.find(w.toCompCode);
        if (it1 == componentMap.end() || it2 == componentMap.end()) continue;

        HardwareComponent* c1 = it1->second;
        HardwareComponent* c2 = it2->second;
        if (!c1 || !c2) continue;

        const Pin* p1 = c1->getPin(w.fromPinName);
        const Pin* p2 = c2->getPin(w.toPinName);
        if (!p1 || !p2) continue;

        Mat4 m1 = c1->getWorldMatrix();
        Mat4 m2 = c2->getWorldMatrix();

        Vec3 pos1 = p1->getWorldPosition(m1);
        Vec3 pos2 = p2->getWorldPosition(m2);

        w.render3D(pos1, pos2, c1->isPowered);
    }
}
