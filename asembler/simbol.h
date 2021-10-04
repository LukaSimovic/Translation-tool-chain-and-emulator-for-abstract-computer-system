#pragma once

#include<string>
using namespace std;

class Simbol {
private:
	string ime;
	string sekcija;
	int offset;
	string lge; //local=100, global=010, extern=001
	int size; //samo za sekcije
	static int noviRBR;
	int rbr = noviRBR++;
public:
	Simbol(string i, string sek, int o, string l, int siz) :
		ime(i), sekcija(sek), offset(o), lge(l), size(siz) {}


	string dohvatiIme() { return ime;  }
	int dohvatiRBR() { return rbr; }
	string dohvatiLGE() { return lge; }
	void postaviLGE(string l) { lge = l; }
	string dohvatiSekciju() { return sekcija; }
	void postaviSekciju(string sek) { sekcija = sek; }
	void postaviOfset(int o) { offset = o; }
	void postaviSize(int s) { size = s; }
	int dohvatiSize() { return size; }
	int dohvatiOffset() { return offset; }
	
	friend ostream& operator<<(ostream& it, const Simbol& s) {
		string ii, ss;
		if(s.ime.length() > 7) {
			ii = s.ime + "\t";
		}
		else {
			ii = s.ime + "\t\t";
		}
		if (s.sekcija.length() > 7) {
			ss = s.sekcija + "\t";
		}
		else {
			ss = s.sekcija + "\t\t";
		}

		return it << ii << ss << s.offset << '\t' << s.lge << '\t' << s.size << '\t' << s.rbr << '\n';
	}
};

