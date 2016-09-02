#include "Input.h"

CInput::CInput()
{
}

CInput::CInput(const CInput &)
{
}


CInput::~CInput()
{
}

void CInput::Initialise()
{
	// Set all the keys to false.
	for (int i = 0; i < 256; i++)
	{
		mKeys[i] = false;
	}

	return;
}

void CInput::KeyDown(unsigned int input)
{
	// If key is pressed down, save that state in the key array.
	mKeys[input] = true;
	return;
}

void CInput::KeyUp(unsigned int input)
{
	// If a key is let go of then save it's new state in the key array.
	mKeys[input] = false;
	return;
}

bool CInput::IsKeyDown(unsigned int key)
{
	// Return the key state out of the key array.
	return mKeys[key];
}
