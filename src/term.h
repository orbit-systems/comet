#pragma once
#define TERM_H

// Makes the terminal send STDIN output immediately
// instead of line buffering and turns off echoing
void term_setup(void);
// Restores the terminal if `term_setup` had been run before
// This does nothing if `term_setup` hasn't been called, so it's safe to call
void term_reset(void);
// Exits cleanly, making sure to unfuck the terminal
void Exit(void);
