from pynput import keyboard
from pynput.keyboard import Key, Controller

k = Controller()
last_char = ''


def on_press(key):
    global last_char
    try:
        # print('alphanumeric key {0} pressed'.format(key.char))
        if last_char == ':' and key.char == ')':
            for i in range(0, 2):
                k.press(Key.backspace)
                k.release(Key.backspace)
            k.type(':D')
        last_char = key.char
    except AttributeError:
        last_char = ' '
        # special key like shift, space, alt, MENU, CMD, CAPSLOCK etc
        # print('special key {0} pressed'.format(key))


def on_release(key):
    # print('{0} released'.format(key))
    if key == keyboard.Key.esc:
        # Stop listener
        return False


# Collect events until released
with keyboard.Listener(
        on_press=on_press,
        on_release=on_release) as listener:
    listener.join()
