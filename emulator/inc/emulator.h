#pragma once

#include<fstream>
#include<string>
#include<math.h>
using namespace std;

class Emulator {
private:

	ifstream ulazni_fajl;

	char memorija[1 << 16] = { 0 }; //velicina mem adr prostora je 2^16B

	unsigned short registri[6] = { 0 }; //r0 - r5, reg bez spec namene
	unsigned short PC, SP, PSW; //r6,r7,r8

	string greska;

	bool prekidTajmer, prekidTerminal;
	bool radiTajmer;
	long long int trenutnoVreme;

	//pomocne fje
	int hex2int(string h);

	char citajBajt();
	short citajDvaBajta(int endian);
	void upisiBajt(char vrednost,unsigned short adresa);
	void upisiDvaBajta(short vrednost, unsigned short adresa, int endian);

	void postaviFlegove(int br, short reg1, short reg2); //2 - ZN, 31 - ZCN (<<),32 - ZCN (>>), 4 - ZOCN
	bool proveriFlegove(int ins); //1 - JEQ, 2 - JNE, 3- JGT
	void postaviFleg(int poz, bool sr);
	bool proveriFleg(int poz);

	void obradiZahteveZaPrekidom();
	void skociNaPrekidnuRutinu(int ulaz);

	void pokreniTajmer();
	void tick();
	long long int izracunajPerioduGenZP();

	bool podesiTerminal();
	void vratiTerminal();
    void procitajKarakter();

public:
	
	Emulator(string ulaz);
	void pokreni();
	bool popuniMemoriju();
	void emuliraj();
};
