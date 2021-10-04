#pragma once

#include<string>
#include<iostream>
using namespace std;

class Simbol {
private:
	int fajl;
	string simbol;
	string sekcija;
	string offset;
	string size;
	string local;
	string rbr;

public:
	Simbol(int f, string simb, string sekc, string o, string siz, string l, string r) :
		fajl(f), simbol(simb), offset(o), sekcija(sekc), size(siz), local(l), rbr(r) {}

	friend ostream& operator<<(ostream& it, const Simbol& s) {

		string ii, ss;
		if (s.simbol.length() >= 7) {
			ii = s.simbol + "\t";
		}
		else {
			ii = s.simbol + "\t\t";
		}
		if (s.sekcija.length() >= 7) {
			ss = s.sekcija + "\t";
		}
		else {
			ss = s.sekcija + "\t\t";
		}

		return it << ii << ss << s.offset << '\t' << s.local << '\t' << s.size << '\t' << s.rbr << '\n';
	}

	int getFajl() { return fajl; }
	string getSimbol() { return simbol; }
	string getSekcija() { return sekcija; }
	string getOffset() { return offset; }
	string getSize() { return size; }
	string getLocal() { return local; }
	string getRbr() { return rbr; }

	void setOffset(string o) { offset = o; }
	void setSize(string s) { size = s; }
	void setFajl(int f) { fajl = f; }
	void setRBR(string r) { rbr = r; }
};