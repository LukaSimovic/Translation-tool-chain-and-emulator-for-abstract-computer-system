#pragma once
#include<fstream>
#include"simbol.h"
#include<list>
#include"sadrzajSekcije.h"
#include"relokZapis.h"
using namespace std;

class Asembler {
private:
	ifstream ulazni_fajl;
	ofstream izlazni_fajl;
    string ime_izlaznog_fajla;

	string greske;

	list<Simbol> tabelaSimbola;
	list<SadrzajSekcije> tabelaSekcija;
	list<RelokZapis> tabelaRelokZapisa;

	string trenutnaSekcija;
	int lc;

	int brABSsimbola;
	int linijaKoda;

public:
	Asembler();
	Asembler(string ulaz, string izlaz);
	void postaviUlaz(string u);
	void postaviIzlaz(string i);
	void zatvoriFajlove();

	void pokreni();
	bool prviProlaz();
	bool drugiProlaz();

	void dodajSimbUtabSimb(Simbol simb);
	int simbVecUTab(string ime);
	int dohvMaxRBRTabSimb();
	void ispisiTabSimb();
	
	void dodajRedUtabSek(SadrzajSekcije ss);
	void ispisiTabSek();

	void dodajRelZapUtab(RelokZapis rk);
	string obradiSimbolAbsolute(string simb,string os,int endian);
	string obradiSimbolPcRelative(string simb, string os, int endian);
	void ispisiTabRelZap();


	//pomocne funkcije
	string srediRed(string red);
	int hex2int(string h);
	string lit2sadrzaj(string lit);
	string sadrzaj2lit(string sadrzaj);
	string obrniBajtove(string rec);
	string reg2sadrzaj(string reg);

};
