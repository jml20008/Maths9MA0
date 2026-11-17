#ifndef MOCK_KEYBOARD_H
#define MOCK_KEYBOARD_H
typedef struct {int key;} key_event_t;
key_event_t getkey(void);
#endif
