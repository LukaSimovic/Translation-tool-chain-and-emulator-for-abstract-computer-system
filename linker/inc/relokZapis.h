#pragma once
#include<string>
#include<iostream>
using namespace std;

class RelokZapis {
private:
	int fajl;
	string sekcija;
	string offset;
	string addend;
	string tip;
	string simbol;
	string endian; //1-little (.word), 0-big (jmp,ldr,str...)
public:
	RelokZapis(int f, string se, string o, string t, string si, string a,string e) :
		fajl(f), sekcija(se), offset(o), tip(t), simbol(si), addend(a),endian(e) {}

	friend ostream& operator<<(ostream& it, const RelokZapis& rk) {

		return it<<rk.sekcija << "\t" << rk.offset << "\t" << rk.tip << "\t" << rk.simbol << "\t" << rk.addend << "\t" << rk.endian << "\n";
	}

	string getEndian() { return endian; }
	string getSekcija() { return sekcija; }
	string getSimbol() { return simbol; }
	int getFajl() { return fajl; }
	string getTip() { return tip; }
	string getOffset() { return offset; }
	string getAddend() { return addend; }

	void setFajl(int f) { fajl = f; }
	void setSekcija(string s) { sekcija = s; }
	void setOffset(string o) { offset = o; }
	void setSimbol(string s) { simbol = s; }
	void setAddend(string a) { addend = a; }
};