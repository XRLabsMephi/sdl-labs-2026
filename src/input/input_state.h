#ifndef INPUT_INPUT_STATE_H_
#define INPUT_INPUT_STATE_H_

struct InputState {
  bool move_forward = false;
  bool move_backward = false;
  bool strafe_left = false;
  bool strafe_right = false;
  bool rotate_left = false;
  bool rotate_right = false;
  bool quit = false;
};

#endif  // INPUT_INPUT_STATE_H_
