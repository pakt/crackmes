#include <iostream>
#include "ecnr.h"


//using namespace std;

// y^2 == x^3 + a*x^2 + b mod p
int ECNR::set_curve(char *a_, char *b_, char *p_){
	a = a_;
	b = b_;
	p = p_;
	ecurve(a,b,p,MR_PROJECTIVE);

	// don't forget to set_point
	state = GOT_CURVE;

	return TRUE;
}

// Q = k*P
bool ECNR::set_point(char *k_, char *ord_, char *Px, char *Py){
	Big x,y;

	if(state == NO_CURVE)
		return FALSE;

	k = k_;
	ord = ord_;
	x = Px;
	y = Py;

	if (!P.set(x,y))
	{
		cout << "Problem - point (x,y) is not on the curve" << endl;
		return FALSE;
	}

	W = P;
    W *= ord;

    if (!W.iszero())
    {
        cout << "Problem - point (x,y) is not of order ord" << endl;
        return FALSE;
    }

	Q = k*P;

	state = OK;

	return TRUE;
}

void ECNR::set_seed(unsigned int s){
	rnd_seed = s;
	irand(rnd_seed);	//weak, 32 bit seed :P
}

bool ECNR::sign(Big msg, Big& sig1, Big& sig2)
{
	Big t,x,y;
	ECn R;

	if(state != OK)
		return FALSE;

	//cout << ord << k << " " << p << endl;

	if(rnd_seed)
		t = rand(ord);
	else
		t = 4;	//chosen by fair dice roll
				//guaranteed to be random

	R = t*P;
	R.get(x,y);

	sig1 = (x + msg)%ord;
	sig2 = (t-k*sig1)%ord;

	if(sig1<0) sig1 += ord;
	if(sig2<0) sig2 += ord;


	return TRUE;
}

bool ECNR::verify(Big msg, Big sig1, Big sig2)
{
	if(state != OK)
		return FALSE;

	Big x,y,o;
	ECn R,S;

	R = mul(sig2, P, sig1, k*P);
	R.get(x,y);

	o = (sig1 - x)%ord;

	return (msg == o);
}


