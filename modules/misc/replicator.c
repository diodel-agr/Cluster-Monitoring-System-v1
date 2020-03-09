#include <linux/input.h> // for struct input_event and other vars and defs.
#include <unistd.h> // for write() function.
#include <fcntl.h> // for open() and defines.
#include <string.h> // for strlen().
#include <stdio.h>
#include <sys/time.h> // for struct timeval and gettimeofday fct.
#include <stdlib.h> // for malloc and free.

#define KEY_EVENT 1
#define EV_PRESSED 1
#define EV_RELEASED 0
#define EV_REPEAT 2

void fct(FILE* fp) {
    fprintf(fp, "KEY_A: %d.\n", KEY_A);
    fprintf(fp, "KEY_B: %d.\n", KEY_B);
    fprintf(fp, "KEY_C: %d.\n", KEY_C);
    fprintf(fp, "KEY_D: %d.\n", KEY_D);
    fprintf(fp, "KEY_E: %d.\n", KEY_E);
    fprintf(fp, "KEY_F: %d.\n", KEY_F);
    fprintf(fp, "KEY_G: %d.\n", KEY_G);
    fprintf(fp, "KEY_H: %d.\n", KEY_H);
    fprintf(fp, "KEY_I: %d.\n", KEY_I);
    fprintf(fp, "KEY_J: %d.\n", KEY_J);
    fprintf(fp, "KEY_K: %d.\n", KEY_K);
    fprintf(fp, "KEY_L: %d.\n", KEY_L);
    fprintf(fp, "KEY_M: %d.\n", KEY_M);
    fprintf(fp, "KEY_N: %d.\n", KEY_N);
    fprintf(fp, "KEY_O: %d.\n", KEY_O);
    fprintf(fp, "KEY_P: %d.\n", KEY_P);
    fprintf(fp, "KEY_Q: %d.\n", KEY_Q);
    fprintf(fp, "KEY_R: %d.\n", KEY_R);
    fprintf(fp, "KEY_S: %d.\n", KEY_S);
    fprintf(fp, "KEY_T: %d.\n", KEY_T);
    fprintf(fp, "KEY_U: %d.\n", KEY_U);
    fprintf(fp, "KEY_V: %d.\n", KEY_V);
    fprintf(fp, "KEY_W: %d.\n", KEY_W);
    fprintf(fp, "KEY_X: %d.\n", KEY_X);
    fprintf(fp, "KEY_Y: %d.\n", KEY_Y);
    fprintf(fp, "KEY_Z: %d.\n", KEY_Z);
    
    fprintf(fp, "KEY_0: %d.\n", KEY_0);
    fprintf(fp, "KEY_1: %d.\n", KEY_1);
    fprintf(fp, "KEY_2: %d.\n", KEY_2);
    fprintf(fp, "KEY_3: %d.\n", KEY_3);
    fprintf(fp, "KEY_4: %d.\n", KEY_4);
    fprintf(fp, "KEY_5: %d.\n", KEY_5);
    fprintf(fp, "KEY_6: %d.\n", KEY_6);
    fprintf(fp, "KEY_7: %d.\n", KEY_7);
    fprintf(fp, "KEY_8: %d.\n", KEY_8);
    fprintf(fp, "KEY_9: %d.\n", KEY_9);

    fprintf(fp, "KEY_F1: %d.\n", KEY_F1);
    fprintf(fp, "KEY_F2: %d.\n", KEY_F2);
    fprintf(fp, "KEY_F3: %d.\n", KEY_F3);
    fprintf(fp, "KEY_F4: %d.\n", KEY_F4);
    fprintf(fp, "KEY_F5: %d.\n", KEY_F5);
    fprintf(fp, "KEY_F6: %d.\n", KEY_F6);
    fprintf(fp, "KEY_F7: %d.\n", KEY_F7);
    fprintf(fp, "KEY_F8: %d.\n", KEY_F8);
    fprintf(fp, "KEY_F9: %d.\n", KEY_F9);
    fprintf(fp, "KEY_F10: %d.\n", KEY_F10);
    fprintf(fp, "KEY_F11: %d.\n", KEY_F11);
    fprintf(fp, "KEY_F12: %d.\n", KEY_F12);
    
    fprintf(fp, "KEY_ESC: %d.\n", KEY_ESC);
    fprintf(fp, "KEY_MINUS: %d.\n", KEY_MINUS);
    fprintf(fp, "KEY_BACKSPACE: %d.\n", KEY_BACKSPACE);
    fprintf(fp, "KEY_TAB: %d.\n", KEY_TAB);
    fprintf(fp, "KEY_ENTER: %d.\n", KEY_ENTER);
    fprintf(fp, "KEY_BACKSLASH: %d.\n", KEY_BACKSLASH);
    fprintf(fp, "KEY_COMMA: %d.\n", KEY_COMMA);
    fprintf(fp, "KEY_STOP: %d.\n", KEY_STOP);
    fprintf(fp, "KEY_SLASH: %d.\n", KEY_SLASH);
    fprintf(fp, "KEY_SPACE: %d.\n", KEY_SPACE);
    
    fprintf(fp, "KEY_INSERT: %d.\n", KEY_INSERT);
    fprintf(fp, "KEY_HOME: %d.\n", KEY_HOME);
    fprintf(fp, "KEY_END: %d.\n", KEY_END);
    fprintf(fp, "KEY_LEFT: %d.\n", KEY_LEFT);
    fprintf(fp, "KEY_RIGHT: %d.\n", KEY_RIGHT);
    fprintf(fp, "KEY_UP: %d.\n", KEY_UP);
    fprintf(fp, "KEY_DOWN: %d.\n", KEY_DOWN);
    fprintf(fp, "KEY_PAUSE: %d.\n", KEY_PAUSE);
    
    fprintf(fp, "KEY_YEN: %d.\n", KEY_YEN);
    fprintf(fp, "KEY_MENU: %d.\n", KEY_MENU);
    fprintf(fp, "KEY_NUMLOCK: %d.\n", KEY_NUMLOCK);
    fprintf(fp, "KEY_CAPSLOCK: %d.\n", KEY_CAPSLOCK);
    fprintf(fp, "KEY_SEMICOLON: %d.\n", KEY_SEMICOLON);
}

unsigned short* getKeycodes(char* srt);

int main(int argc, char* argv[]) {
    // argv[0] process name.
    // argv[1] path to event file.
    // argv[2] string to write.
    
    // debug file.
    FILE* fp;
    fp = fopen("/home/razvan/Documents/*licenta/keylogger/klm/test/test.txt", "w");
    
    fprintf(fp, "argv[1] = %s.\n", argv[1]);
    fprintf(fp, "argv[2] = %s.\n", argv[2]);
    // open /dev/input/eventX file.
    int fd = open(argv[1], O_WRONLY);
    fprintf(fp, "fd = %d.\n", fd);
	if (fd < 0) {
        fclose(fp);
		return fd;
	}
	
	char* string = argv[2]; // pointer la tastele apasate.
    fprintf(fp, "Trying to write: %s.\n", string);
    
    struct input_event event;
    struct timezone tz;
    int res = gettimeofday(&event.time, &tz);
    
    if (res == -1) {
        fprintf(fp, "Could not get the time of day.");
        fclose(fp);
        close(fd);
        return res;
    }
    
    event.type = EV_KEY; // key event.
    unsigned short* keycodes = getKeycodes(string); // get corresponding key codes.
    unsigned short* code = keycodes;
    // do the good stuff.
    while (*code != 0) {
        // simulate key press.
        event.value = EV_PRESSED;
        event.code = *code;
        write(fd, &event, sizeof(struct input_event));
        fprintf(fp, "write %d\n", *code);
        event.time.tv_usec++;
        // simulate key release.
        event.value = EV_RELEASED;
        write(fd, &event, sizeof(struct input_event));
        event.time.tv_usec++;
        code++;
    }

    // release mem.
    fprintf(fp, "Closing file.\n");
    fclose(fp);
    close(fd);
    free(keycodes);
    return 0;
}

unsigned short* getKeycodes(char* str) {
    int len = strlen(str);
    unsigned short* keycodes = (unsigned short*)malloc(sizeof(unsigned short) * (len + 1));
    int count = 0;
    int i = 0;
    for (; i < len; ++i) {
        switch (str[i]) {
            case 'a':
            case 'A':
                keycodes[count++] = KEY_A;
                break;
            case 'b':
            case 'B':
                keycodes[count++] = KEY_B;
                break;
            case 'c':
            case 'C':
                keycodes[count++] = KEY_C;
                break;
            case 'd':
            case 'D':
                keycodes[count++] = KEY_D;
                break;
            case 'e':
            case 'E':
                keycodes[count++] = KEY_E;
                break;
            case 'f':
            case 'F':
                keycodes[count++] = KEY_F;
                break;
            case 'g':
            case 'G':
                keycodes[count++] = KEY_G;
                break;
            case 'h':
            case 'H':
                keycodes[count++] = KEY_H;
                break;
            case 'i':
            case 'I':
                keycodes[count++] = KEY_I;
                break;
            case 'j':
            case 'J':
                keycodes[count++] = KEY_J;
                break;
            case 'k':
            case 'K':
                keycodes[count++] = KEY_K;
                break;
            case 'l':
            case 'L':
                keycodes[count++] = KEY_L;
                break;
            case 'm':
            case 'M':
                keycodes[count++] = KEY_M;
                break;
            case 'n':
            case 'N':
                keycodes[count++] = KEY_N;
                break;
            case 'o':
            case 'O':
                keycodes[count++] = KEY_O;
                break;
            case 'p':
            case 'P':
                keycodes[count++] = KEY_P;
                break;
            case 'q':
            case 'Q':
                keycodes[count++] = KEY_Q;
                break;
            case 'r':
            case 'R':
                keycodes[count++] = KEY_R;
                break;
            case 's':
            case 'S':
                keycodes[count++] = KEY_S;
                break;
            case 't':
            case 'T':
                keycodes[count++] = KEY_T;
                break;
            case 'u':
            case 'U':
                keycodes[count++] = KEY_U;
                break;
            case 'v':
            case 'V':
                keycodes[count++] = KEY_V;
                break;
            case 'w':
            case 'W':
                keycodes[count++] = KEY_W;
                break;
            case 'x':
            case 'X':
                keycodes[count++] = KEY_X;
                break;
            case 'y':
            case 'Y':
                keycodes[count++] = KEY_Y;
                break;
            case 'z':
            case 'Z':
                keycodes[count++] = KEY_Z;
                break;
            default:
                break;
        }
    }
    keycodes[count] = 0;
    return keycodes;
}
