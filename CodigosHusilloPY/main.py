from husillo_control import HusilloController
import time

# 1. Configuración del objeto: Ejemplo de integración standalone
# Asegúrate de que 'COM3' coincida con el puerto de tu ESP32
robot = HusilloController(port='COM6', mm_per_step=0.005)

# 2. Movimiento inicial: REQUERIMIENTO de control en posición y velocidad
print("Iniciando movimiento...")
robot.move(target_mm=20.0, velocity_mm_s=2.0)

# 3. Ciclo de monitoreo: REQUERIMIENTO de describir estado y ETA
# range(5) hará que el programa pregunte por el estado 5 veces (una cada segundo)
for _ in range(5):
    # Simulamos que el robot estima estar en 50.0mm para ver el cálculo del ETA
    estado = robot.get_status(current_pos_estimate_mm=50.0)
    print(f"Status: {estado}")
    time.sleep(1)

# 4. REQUERIMIENTO: Sobrescritura de comando en ejecución
# No necesitamos esperar a que termine el anterior, el módulo manda el nuevo de inmediato
print("\nCAMBIO DE PLAN: Regresando a 0mm inmediatamente.")
robot.move(target_mm=0.0, velocity_mm_s=40.0)

# Opcional: Mostrar el estado tras la sobrescritura
print(f"Nuevo Status: {robot.get_status(current_pos_estimate_mm=50.0)}")