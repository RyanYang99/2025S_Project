#pragma once

typedef void (* keyhit_t)(const char character);

void initialize_input_handler(void);
void handle_input_event(void);
void destroy_input_handler(void);
void subscribe_keyhit(const keyhit_t callback);
void unsubscribe_keyhit(const keyhit_t callback);