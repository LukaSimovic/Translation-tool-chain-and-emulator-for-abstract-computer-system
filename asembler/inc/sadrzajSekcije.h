#pragma once

#include<string>
using namespace std;

class SadrzajSekcije {
private:

	string sekcija;
	string offset;
	string sadrzaj;

public:

	SadrzajSekcije(string sek, string o, string sad) : sekcija(sek), offset(o), sadrzaj(sad) {}

	string dohvatiSekciju() { return sekcija; }
	string dohvatiOffset() { return offset; }
	string dohvatiSadrzaj() { return sadrzaj; }

	friend ostream& operator<<(ostream& it, const SadrzajSekcije& s) { return it << s.offset << "\t" << s.sadrzaj << endl; }
};