# EliteArduino

Control panel for Elite Dangerous powered by the Teensy 3.2.

In addition to just buttons and switches, the control panel receives data from [an agent running alongside the game](https://github.com/BenJuan26/edca) to display information and keep the toggle switches in sync.

A more detailed explanation can be found [here](https://benjuan26.com/blog/building-a-smart-elite-dangerous-control-panel/).

## How it works

The Teensy 3.2 has a native Joystick mode out of the box, so that part is easy. Any joystick button can be pressed and released in the Arduino code, based on whether the physical buttons are pressed. The core of this project is to make sure the toggle switches represent the state of what they're controlling in the game in real time. It receives the data about the game, compares it against the positions of the switches, and does what it can to get the game state to match the switch state.

## Example

Let's say, for example, that I'm on a planet and I have Night Vision enabled. I log out of the game and put my controller away, and I reset all the switches to the off position. The next time I go to play, Night Vision is still enabled in-game, but it's disabled on my controller. Of course, I could just not worry about it and accept that "down" on the Night Vision switch now means "on", but then what's the point of using a toggle switch instead of a button? I want the switch to have the final say of the thing that it's controlling, just like it would in a real spaceship, or airplane, or car.

In the case of this controller, after starting up the game, the controller would see that Night Vision is enabled, see that I actually want Night Vision to be disabled, and fire a button press to make it happen. It will try that every so often (five seconds for now) until the game and the button are in sync.

## Fallback to default behaviour

If the controller hasn't received any data from the agent, or if data hasn't been received for some time (currently set to 12 seconds), it will fall back to the default behaviour of simply firing a button press every time a switch changes position. This way, it can conceivably be used without the agent running, though its usefulness will be questionable in that case.

## Other data received from the game

Thanks to the [API used by the agent](https://github.com/BenJuan26/elite), a lot of data could be passed through to the controller if desired. For now, I'm only passing the star system location of the player, since I find it's something that's always buried in a menu or two and it would be nice to have on-hand. It gets displayed on a 16x2 LCD screen on the control panel.

## Potential improvements

Since there is an LCD screen, it could be useful to add a menu that can change what's displayed on the screen. There could also be a menu option for the brightness of the button LEDs, providing they're wired in such a way that PWM could be used to dim them.
