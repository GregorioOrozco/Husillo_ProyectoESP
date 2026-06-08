import serial
import time

class HusilloController:
    def __init__(self, port, baudrate=115200, mm_per_step=0.01):
        """
        mm_per_step: Resolución calculada (Avance_husillo / pasos_totales).
        """
        self.ser = serial.Serial(port, baudrate, timeout=0.1)
        self.mm_per_step = mm_per_step
        self.target_pos_mm = 0.0
        self.current_vel_mm_s = 0.0
        self.last_cmd = "N/A"
        self.start_time = 0

    def move(self, target_mm, velocity_mm_s):
        """
        REQUERIMIENTO: Manda comandos que reescriben el anterior.
        """
        steps = int(target_mm / self.mm_per_step)
        vel_steps = int(velocity_mm_s / self.mm_per_step)
        
        cmd = f"P:{steps},V:{vel_steps}\n"
        self.ser.write(cmd.encode())
        
        # Actualización de estado para reporte
        self.target_pos_mm = target_mm
        self.current_vel_mm_s = velocity_mm_s
        self.last_cmd = cmd.strip()
        self.start_time = time.time()

    def get_status(self, current_pos_estimate_mm):
        """
        REQUERIMIENTO: Describe estado actual, último comando y tiempo esperado (ETA).
        """
        dist_restante = abs(self.target_pos_mm - current_pos_estimate_mm)
        eta = dist_restante / self.current_vel_mm_s if self.current_vel_mm_s > 0 else 0
        
        return {
            "estado": "MOVIMIENTO" if dist_restante > 0.1 else "IDLE",
            "objetivo_mm": self.target_pos_mm,
            "ultimo_comando": self.last_cmd,
            "eta_segundos": round(eta, 2)
        }