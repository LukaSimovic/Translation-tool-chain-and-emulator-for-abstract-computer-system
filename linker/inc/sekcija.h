#pragma once

#include<string>
#include<iostream>
using namespace std;

class Sekcija {
private:
	int fajl;
	string ime;
	int size;
	int pocAdr;
	bool place;
	bool ispisana;
public:
	Sekcija(int f, string i, int s, int p, bool pl) :
		fajl(f), ime(i), size(s), pocAdr(p), place(pl) {
		ispisana = false;
	}

	string getIme() { return ime; }
	bool getPlace() { return place; }
	int getPocAdr() { return pocAdr; }
	int getFajl() { return fajl; }
	int getSize() { return size; }
	bool getIspisana() { return ispisana; }

	void setIspisana() { ispisana = true; }
	
	void setSize(int s) { size = s; }
	void setPocAdr(int p) { pocAdr = p; }


	friend ostream& operator<<(ostream& it, const Sekcija& s)
		{ return it << s.fajl << "\t" << s.ime << "\t" << s.size << "\t" << s.pocAdr << "\t" << s.place << endl; }

};