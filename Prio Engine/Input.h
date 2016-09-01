#ifndef INPUT_H
#define INPUT_H

class CInput
{
public:
	CInput();
	CInput(const CInput&);
	~CInput();

	// Initialised the input object class.
	void Initialise();

	// Triggered when key is pressed down. 
	void KeyDown(unsigned int);
	// Triggered when key is let go of.
	void KeyUp(unsigned int);

	// Is the key being held down?
	bool IsKeyDown(unsigned int);

private:
	bool mKeys[256];
};

#endif