/*
ifndef means "if not defined"
so:
	if __GAME_TIMER_H__ (representing this header file) is not defined:
	define it
	and then end the definition

this is so that if the file is referenced from multiple other files it does not try to redefine (generating an error)
*/

#ifndef __MOD_TIMER_H__
#define __MOD_TIMER_H__

/*
===============================================================================

Timer base.

===============================================================================
*/

class fsTimer{

private:
	int max;
	int currVal;

public:
	//Constructor
	fsTimer(int inMax)
	{
		max = inMax;
		currVal = inMax;
	}

	//decrements timer value until 0
	bool tick(void) { 
		currVal = max(currVal - 1, 0); 
		//Return true if still ticking, false if not
		return (bool)(currVal > 0);
	}

	//resets timer value to max
	void reset(void) { currVal = max; }

};

#endif /* !__MOD_TIMER_H__ */