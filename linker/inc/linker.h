#pragma once

#include<list>
#include<string>
#include<fstream>
#include"simbol.h"
#include"relokZapis.h"
#include"sadrzajSekcije.h"
#include"sekcija.h"
using namespace std;

class Linker {
private:
	int mod; //0-hex, 1-linkable

	list <string> nazivi_ulaznih_fajlova;
	ofstream izlazni_fajl;

	list<string> adreseSekcija;

	list <Simbol> tabelaSimbola;
	list <SadrzajSekcije> tabelaSadrzajaSekcija;
	list <RelokZapis> tabelaRelZap;
	list <Sekcija> tabelaSekcija;

	string greska;

	int brSekcija;

public:
	Linker(int m, list<string> as);
	Linker(int m, list<string> ulazi, string izlaz, list<string> as);
	
	void postaviUlaze(list<string> ulazi);
	void postaviIzlaz(string izlaz);
	void zatvoriIzlazniFajl();

	void pokreni();
	bool obradiUlazneFajlove();
	bool rasporediSekcijeHex();
	bool azurirajTabSimbolaHex();
	bool obradiRelokZapise();

	void rasporediSekcijeLinkable();
	void azurirajSadrzajeSekcija();
	void azurirajTabSimbolaIRelZapiseLinkable();
	void azurirajRelokZapise();

	void generisiZapisHEX();

	//pomocne fje
	void ispisiTabele();
	void ispisiPomocneTabele();
	int hex2int(string h);
	string int2sadrzaj(int br);
	int postojiSekcijaSaPlace(string sekc);
	int string2int(string s);
	string int2string(int i);
	void nadjiIUbaciOstale(string simb, int poc, bool p,int cnt);
	bool sekcijaVecUbacena(string sekc);
	int maxPrvaSlobodnaAdresa();
	void razresiRelZapis(int fajl, string sekcija, int offset, int vrednost, string tip, string addend, string endian);
	int sledeciOffset(int trenutniOffset, string trenutniSadrzaj);
};
