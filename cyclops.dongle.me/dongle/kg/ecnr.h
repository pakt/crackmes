#ifndef __ECNR_H__
#define __ECNR_H__

#include "big.h"
#include "ecn.h"

enum STATE {NO_CURVE, GOT_CURVE, OK};

class ECNR
{
	private:
		STATE state;
		unsigned int rnd_seed;
		Big a,b,p;
		Big k,ord;
		ECn P,Q,W;

	public:
		ECNR()
		{
			rnd_seed = 0;
			state = NO_CURVE;
		}
		~ECNR()
		{ }

		int set_curve(char *a, char *b, char *p);
		bool set_point(char *k, char *ord, char *Px, char *Py);

		void set_seed(unsigned int s);

		bool sign(Big msg, Big& sig1, Big& sig2);
		bool verify(Big msg, Big sig1, Big sig2);
};



#endif