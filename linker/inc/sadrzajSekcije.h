#pragma once

#include<string>
#include<iostream>
using namespace std;

class SadrzajSekcije {
private:

	int fajl;
	string sekcija;
	string offset;
	string sadrzaj;

public:
	SadrzajSekcije(int f, string sekc,string o, string s) :
		fajl(f), sekcija(sekc),offset(o), sadrzaj(s) {}

	int getFajl() { return fajl; }
	string getOffset() { return offset; }
	string getSekcija() { return sekcija; }
	string getSadrzaj() { return sadrzaj; }

	void setSadrzaj(string s) { sadrzaj = s; }
	void setOffset(string o) { offset = o; }
	void setFajl(int f) { fajl = f; }

	friend ostream& operator<<(ostream& it, const SadrzajSekcije& s) { return it  <<  s.offset << "\t" << s.sadrzaj << endl; }
};