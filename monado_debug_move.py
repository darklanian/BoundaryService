import os
import sys
import keyboard

x = 0.0
y = 0.0
z = 0.0

def send_move_command(x, y, z):
	#print(f"{x} {y} {z}")
	cmd = f'adb shell am start-service -a org.freedesktop.monado.ipc.DEBUG -e DEBUG_COMMAND "MOVE,{x},{y},{z}" org.freedesktop.monado.openxr_runtime.out_of_process/org.freedesktop.monado.ipc.MonadoService'
	print(cmd)
	os.system(cmd)

def move(dx, dy, dz):
	global x, y, z
	x += dx
	y += dy
	z += dz	
	send_move_command(x, y, z)
	
def reset():
	global x, y, z
	x = 0.0
	y = 0.0
	z = 0.0
	send_move_command(x, y, z)
	
if __name__ == '__main__':

	keyboard.on_press_key("w", lambda _: move(0.0, 0.0, -0.1))
	keyboard.on_press_key("s", lambda _: move(0.0, 0.0, 0.1))
	keyboard.on_press_key("a", lambda _: move(-0.1, 0.0, 0.0))
	keyboard.on_press_key("d", lambda _: move(0.1, 0.0, 0.0))
	keyboard.on_press_key("o", lambda _: reset())
	
	keyboard.wait('esc')
		