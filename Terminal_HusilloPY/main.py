import serial
import time

# --------------------------------
# CONFIGURACIÓN
# --------------------------------
PORT = "COM6"      # Cambia según tu ESP32
BAUDRATE = 115200

# --------------------------------
# CONEXIÓN
# --------------------------------
try:
    esp32 = serial.Serial(PORT, BAUDRATE, timeout=1)
    time.sleep(2)
except Exception as e:
    print(f"Error conectando a la ESP32: {e}")
    exit()


# --------------------------------
# ENVÍO DE COMANDOS
# --------------------------------
def send_command(cmd):
    esp32.write((cmd + "\n").encode())
    time.sleep(0.05)

    while esp32.in_waiting:
        print(esp32.readline().decode(errors="ignore").strip())


# --------------------------------
# MENÚ DE AYUDA
# --------------------------------
def show_instructions():

    while True:

        print("\n==============================")
        print("INSTRUCCIONES DISPONIBLES")
        print("==============================")

        print("STATUS")
        print("  Consulta posición, estado, cola y límites")

        print("\nMOVE:xxxxx")
        print("  Mueve a una posición absoluta")
        print("  Rango permitido: 1500-159000")

        print("\nHOME")
        print("  Regresa al centro")

        print("\nREHOME")
        print("  Ejecuta rutina completa de homing")

        print("\nSTOP")
        print("  Detiene movimiento actual")

        print("\nRUN")
        print("  Continúa movimiento detenido")

        print("\nBACK")
        print("  Regresar al menú principal")

        print("==============================")

        cmd = input("Comando a enviar o BACK: ").strip()

        if cmd.upper() == "BACK":
            return

        send_command(cmd)


# --------------------------------
# CARGA DE INSTRUCCIONES
# --------------------------------
def load_instructions():

    while True:

        print("\n==============================")
        print("CARGA DE INSTRUCCIONES")
        print("==============================")
        print("MOVE:1500-159000")
        print("Hasta 100 instrucciones")
        print("Separadas por coma")
        print("")
        print("Ejemplo:")
        print("MOVE:20000,MOVE:40000,MOVE:80000")
        print("")
        print("Escriba BACK para regresar")
        print("==============================")

        data = input("> ").strip()

        if data.upper() == "BACK":
            return

        commands = data.split(",")

        for cmd in commands:

            cmd = cmd.strip()

            if cmd == "":
                continue

            send_command(cmd)

        print("\nInstrucciones enviadas.\n")


# --------------------------------
# MENÚ PRINCIPAL
# --------------------------------
def main():

    while True:

        print("\n==============================")
        print("CONTROL DE HUSILLO")
        print("==============================")
        print("1. Consultar instrucciones")
        print("2. Cargar instrucciones")
        print("3. Exit")
        print("==============================")

        option = input("Seleccione una opción: ")

        if option == "1":
            show_instructions()

        elif option == "2":
            load_instructions()

        elif option == "3":

            print("Cerrando conexión...")

            esp32.close()

            break

        else:
            print("Opción inválida")


if __name__ == "__main__":
    main()
    