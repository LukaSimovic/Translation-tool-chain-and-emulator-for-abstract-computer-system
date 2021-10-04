#include "../inc/emulator.h"
#include<iostream>
#include <chrono>
#include<termios.h>
#include<unistd.h>
#include<stdlib.h>

Emulator::Emulator(string ulaz)
{
	ulazni_fajl.open(ulaz);
	if (ulazni_fajl.is_open() == false) {
		cout << "Neuspesno otvoren ulazni fajl!" << endl;
		exit(1);
	}

	SP = hex2int("0xFF00");
	PC = 0;
	PSW = 0;

	prekidTerminal=false;
	prekidTajmer=false;
	radiTajmer=false;
}

void Emulator::pokreni()
{
	if (!popuniMemoriju()) {
		cout << greska << endl;
		exit(1);
	}
	emuliraj();
}

bool Emulator::popuniMemoriju()
{
	string red;
	while (getline(ulazni_fajl, red)) {
		string adrstr = red.substr(0, 4);
		int adr = hex2int("0x" + adrstr);
		int pom = 0;
		red = red.substr(6);
		while (true) {

			if (adr + pom >= hex2int("0xFF00")) {
				greska = "Adresa je veca od maksimalne dozvoljene adrese kojoj korisnik moze da pristupi!\n";
				return false;
			}

			string bajt = red.substr(0, 2);
			memorija[adr + pom] = hex2int("0x" + bajt);
			//cout << "memorija[" << (adr + pom) << "] = " << hex2int("0x" + bajt) << endl;
			pom++;
			if (red.length() == 2) { //to je bio poslednji bajt u redu
				break;
			}
			else {
				red = red.substr(3);
			}
		}
	}

	ulazni_fajl.close();

	return true;
}

void Emulator::emuliraj()
{
	//ivt[0]
	PC = citajDvaBajta(1);

	pokreniTajmer();

	if(!podesiTerminal()){
        greska = "Greska pri podesavanju terminala!";
        cout<<greska<<endl;
        exit(1);
    }
	

	while (true) {

		unsigned short prethodniPC = PC;

		//DOHVATANJE INSTRUKCIJE
		unsigned char ins = citajBajt();

		if (ins == 0) { //halt
			break;
		}
		else if (ins == 16) { //int

			unsigned char regs = citajBajt();
			unsigned char reg = (reg >> 4) & 0xf;
			unsigned short pr;
			if (reg > 8) {
				greska = "Pogresno prosledjen registar instrukciji INT!\n";
				PC = prethodniPC;
				skociNaPrekidnuRutinu(1);
				continue;
			}
			else if (reg == 8) {
				pr = PSW;
			}
			else if (reg == 7) {
				pr = PC;
			}
			else if (reg == 6) {
				pr = SP;
			}
			else {
				pr = registri[reg];
			}

			unsigned short adresaUlaza = (pr % 8) * 2;
			unsigned char nizi = memorija[adresaUlaza++];
			unsigned char visi = memorija[adresaUlaza];
			unsigned short adresaPrekRutine = visi * 256 + nizi;

			//stavi PC na stek
			SP = SP - 2;
			upisiDvaBajta(PC, SP, 1);
			
			//stavi PSW na stek
			SP = SP - 2;
			upisiDvaBajta(PSW, SP, 1);

			//novi PC
			PC = adresaPrekRutine;

			postaviFleg(13, 1);
			postaviFleg(14, 1);
			postaviFleg(15, 1);
		}
		else if (ins == 32) { //iret

			//skini psw sa steka
			unsigned char nizi = memorija[SP];
			unsigned char visi = memorija[SP + 1];
			PSW = visi * 256 + nizi;
			SP = SP + 2;

			//skini pc sa steka
			nizi = memorija[SP];
			visi = memorija[SP + 1];
			PC = visi * 256 + nizi;
			SP = SP + 2;

		}
		else if (ins == 48) { //call
			unsigned char regs = citajBajt();

			unsigned char adrMod = citajBajt();
			unsigned char nacinAdr = adrMod & 0x0f;

			unsigned short operand;

			if (nacinAdr == 0) { //immed
				unsigned short payload = citajDvaBajta(0);
				operand = payload;
			}
			else if (nacinAdr == 1) { //regdir
				unsigned char reg = regs & 0x0f;
				if (reg > 8) {
					greska = "Pogresno prosledjen registar insturkciji CALL!\n";
					PC = prethodniPC;
					skociNaPrekidnuRutinu(1);
					continue;
				}

				if (reg == 8) {
					operand = PSW;
				}
				else if (reg == 7) {
					operand = PC;
				}
				else if (reg == 6) {
					operand = SP;
				}
				else {
					operand = registri[reg];
				}
			}
			else if (nacinAdr == 2) { //regind
				unsigned char reg = regs & 0x0f;
				if (reg > 8) {
					greska = "Pogresno prosledjen registar insturkciji CALL!\n";
					PC = prethodniPC;
					skociNaPrekidnuRutinu(1);
					continue;
				}

				if (reg == 8) {
					operand = PSW;
				}
				else if (reg == 7) {
					operand = PC;
				}
				else if (reg == 6) {
					operand = SP;
				}
				else {
					operand = registri[reg];
				}

				unsigned short nizi = memorija[operand];
				unsigned short visi = memorija[operand + 1];
				operand = visi * 256 + nizi;
			}
			else if (nacinAdr == 3) { //reginddisp

				unsigned short payload = citajDvaBajta(0);
				unsigned char reg = regs & 0x0f;
				if (reg > 8) {
					greska = "Pogresno prosledjen registar insturkciji CALL!\n";
					PC = prethodniPC;
					skociNaPrekidnuRutinu(1);
					continue;
				}

				if (reg == 8) {
					operand = PSW;
				}
				else if (reg == 7) {
					operand = PC;
				}
				else if (reg == 6) {
					operand = SP;
				}
				else {
					operand = registri[reg];
				}

				unsigned short nizi = memorija[operand+payload];
				unsigned short visi = memorija[operand+ payload+ 1];
				operand = visi * 256 + nizi;

			}
			else if (nacinAdr == 4) { //memdir

				unsigned short payload = citajDvaBajta(0);
				unsigned short nizi = memorija[payload];
				unsigned short visi = memorija[payload + 1];
				operand = visi * 256 + nizi;
			}
			else if (nacinAdr == 5) { //regdisp

				unsigned short payload = citajDvaBajta(0);
				unsigned char reg = regs & 0x0f;
				if (reg > 8) {
					greska = "Pogresno prosledjen registar insturkciji CALL!\n";
					PC = prethodniPC;
					skociNaPrekidnuRutinu(1);
					continue;
				}

				if (reg == 8) {
					operand = PSW;
				}
				else if (reg == 7) {
					operand = PC;
				}
				else if (reg == 6) {
					operand = SP;
				}
				else {
					operand = registri[reg];
				}

				operand = operand + payload;
			}
			else {
				greska = "Pogresno prosledjen nacin adresiranja instrukciji CALL!\n";
				PC = prethodniPC;
				skociNaPrekidnuRutinu(1);
				continue;
			}

			//stavi pc na stek
			SP = SP - 2;
			upisiDvaBajta(PC, SP, 1);

			//skoci na potprogram
			PC = operand;

		}
		else if (ins == 64) { //ret
			
			//skini pc sa steka
			unsigned char nizi = memorija[SP];
			unsigned char visi = memorija[SP + 1];
			PC = visi * 256 + nizi;
			SP = SP + 2;

		}
		else if (ins == 80) { //jmp
			unsigned char regs = citajBajt();

			unsigned char adrMod = citajBajt();
			unsigned char nacinAdr = adrMod & 0x0f;

			unsigned short operand;

			if (nacinAdr == 0) { //immed
				unsigned short payload = citajDvaBajta(0);
				operand = payload;
			}
			else if (nacinAdr == 1) { //regdir
				unsigned char reg = regs & 0x0f;
				if (reg > 8 ) {
					greska = "Pogresno prosledjen registar insturkciji JMP!\n";
					PC = prethodniPC;
					skociNaPrekidnuRutinu(1);
					continue;
				}
				
				if (reg == 8) {
					operand = PSW;
				}
				else if (reg == 7) {
					operand = PC;
				}
				else if (reg == 6) {
					operand = SP;
				}
				else {
					operand = registri[reg];
				}
			}
			else if (nacinAdr == 2) { //regind
				unsigned char reg = regs & 0x0f;
				if (reg > 8) {
					greska = "Pogresno prosledjen registar insturkciji JMP!\n";
					PC = prethodniPC;
					skociNaPrekidnuRutinu(1);
					continue;
				}

				if (reg == 8) {
					operand = PSW;
				}
				else if (reg == 7) {
					operand = PC;
				}
				else if (reg == 6) {
					operand = SP;
				}
				else {
					operand = registri[reg];
				}

				unsigned short nizi = memorija[operand ];
				unsigned short visi = memorija[operand + 1];
				operand = visi * 256 + nizi;
			}
			else if (nacinAdr == 3) { //reginddisp

				short payload = citajDvaBajta(0);
				unsigned char reg = regs & 0x0f;
				if (reg > 8) {
					greska = "Pogresno prosledjen registar insturkciji JMP!\n";
					PC = prethodniPC;
					skociNaPrekidnuRutinu(1);
					continue;
				}

				if (reg == 8) {
					operand = PSW;
				}
				else if (reg == 7) {
					operand = PC;
				}
				else if (reg == 6) {
					operand = SP;
				}
				else {
					operand = registri[reg];
				}

				unsigned short nizi = memorija[operand + payload];
				unsigned short visi = memorija[operand + payload + 1];
				operand = visi * 256 + nizi;

			}
			else if (nacinAdr == 4) { //memdir

				unsigned short payload = citajDvaBajta(0);
				unsigned short nizi = memorija[payload];
				unsigned short visi = memorija[payload + 1];
				operand = visi * 256 + nizi;
			}
			else if (nacinAdr == 5) { //regdisp

				short payload = citajDvaBajta(0);
				unsigned char reg = regs & 0x0f;
				if (reg > 8) {
					greska = "Pogresno prosledjen registar insturkciji JMP!\n";
					PC = prethodniPC;
					skociNaPrekidnuRutinu(1);
					continue;
				}

				if (reg == 8) {
					operand = PSW;
				}
				else if (reg == 7) {
					operand = PC;
				}
				else if (reg == 6) {
					operand = SP;
				}
				else {
					operand = registri[reg];
				}

				operand = operand + payload;
			}
			else {
				greska = "Pogresno prosledjen nacin adresiranja instrukciji JMP!\n";
				PC = prethodniPC;
				skociNaPrekidnuRutinu(1);
				continue;
			}

			PC = operand;
		}
		else if (ins == 81) { //jeq
			unsigned char regs = citajBajt();

			unsigned char adrMod = citajBajt();
			unsigned char nacinAdr = adrMod & 0x0f;

			unsigned short operand;

			if (nacinAdr == 0) { //immed
				unsigned short payload = citajDvaBajta(0);
				operand = payload;
			}
			else if (nacinAdr == 1) { //regdir
				unsigned char reg = regs & 0x0f;
				if (reg > 8 ) {
					greska = "Pogresno prosledjen registar insturkciji JEQ!\n";
					PC = prethodniPC;
					skociNaPrekidnuRutinu(1);
					continue;
				}
				
				if (reg == 8) {
					operand = PSW;
				}
				else if (reg == 7) {
					operand = PC;
				}
				else if (reg == 6) {
					operand = SP;
				}
				else {
					operand = registri[reg];
				}
			}
			else if (nacinAdr == 2) { //regind
				unsigned char reg = regs & 0x0f;
				if (reg > 8) {
					greska = "Pogresno prosledjen registar insturkciji JEQ!\n";
					PC = prethodniPC;
					skociNaPrekidnuRutinu(1);
					continue;
				}

				if (reg == 8) {
					operand = PSW;
				}
				else if (reg == 7) {
					operand = PC;
				}
				else if (reg == 6) {
					operand = SP;
				}
				else {
					operand = registri[reg];
				}

				unsigned short nizi = memorija[operand ];
				unsigned short visi = memorija[operand + 1];
				operand = visi * 256 + nizi;
			}
			else if (nacinAdr == 3) { //reginddisp

				short payload = citajDvaBajta(0);
				unsigned char reg = regs & 0x0f;
				if (reg > 8) {
					greska = "Pogresno prosledjen registar insturkciji JEQ!\n";
					PC = prethodniPC;
					skociNaPrekidnuRutinu(1);
					continue;
				}

				if (reg == 8) {
					operand = PSW;
				}
				else if (reg == 7) {
					operand = PC;
				}
				else if (reg == 6) {
					operand = SP;
				}
				else {
					operand = registri[reg];
				}

				unsigned short nizi = memorija[operand + payload];
				unsigned short visi = memorija[operand + payload + 1];
				operand = visi * 256 + nizi;

			}
			else if (nacinAdr == 4) { //memdir

				unsigned short payload = citajDvaBajta(0);
				unsigned short nizi = memorija[payload];
				unsigned short visi = memorija[payload + 1];
				operand = visi * 256 + nizi;
			}
			else if (nacinAdr == 5) { //regdisp

				short payload = citajDvaBajta(0);
				unsigned char reg = regs & 0x0f;
				if (reg > 8) {
					greska = "Pogresno prosledjen registar insturkciji JEQ!\n";
					PC = prethodniPC;
					skociNaPrekidnuRutinu(1);
					continue;
				}

				if (reg == 8) {
					operand = PSW;
				}
				else if (reg == 7) {
					operand = PC;
				}
				else if (reg == 6) {
					operand = SP;
				}
				else {
					operand = registri[reg];
				}

				operand = operand + payload;
			}
			else {
				greska = "Pogresno prosledjen nacin adresiranja instrukciji JMP!\n";
				PC = prethodniPC;
				skociNaPrekidnuRutinu(1);
				continue;
			}

			if (proveriFlegove(1)) {
				PC = operand;
			}

		}
		else if (ins == 82) { //jne
			unsigned char regs = citajBajt();

			unsigned char adrMod = citajBajt();
			unsigned char nacinAdr = adrMod & 0x0f;

			unsigned short operand;

			if (nacinAdr == 0) { //immed
				unsigned short payload = citajDvaBajta(0);
				operand = payload;
			}
			else if (nacinAdr == 1) { //regdir
				unsigned char reg = regs & 0x0f;
				if (reg > 8 ) {
					greska = "Pogresno prosledjen registar insturkciji JNE!\n";
					PC = prethodniPC;
					skociNaPrekidnuRutinu(1);
					continue;
				}
				
				if (reg == 8) {
					operand = PSW;
				}
				else if (reg == 7) {
					operand = PC;
				}
				else if (reg == 6) {
					operand = SP;
				}
				else {
					operand = registri[reg];
				}
			}
			else if (nacinAdr == 2) { //regind
				unsigned char reg = regs & 0x0f;
				if (reg > 8) {
					greska = "Pogresno prosledjen registar insturkciji JNE!\n";
					PC = prethodniPC;
					skociNaPrekidnuRutinu(1);
					continue;
				}

				if (reg == 8) {
					operand = PSW;
				}
				else if (reg == 7) {
					operand = PC;
				}
				else if (reg == 6) {
					operand = SP;
				}
				else {
					operand = registri[reg];
				}

				unsigned short nizi = memorija[operand ];
				unsigned short visi = memorija[operand + 1];
				operand = visi * 256 + nizi;
			}
			else if (nacinAdr == 3) { //reginddisp

				short payload = citajDvaBajta(0);
				unsigned char reg = regs & 0x0f;
				if (reg > 8) {
					greska = "Pogresno prosledjen registar insturkciji JNE!\n";
					PC = prethodniPC;
					skociNaPrekidnuRutinu(1);
					continue;
				}

				if (reg == 8) {
					operand = PSW;
				}
				else if (reg == 7) {
					operand = PC;
				}
				else if (reg == 6) {
					operand = SP;
				}
				else {
					operand = registri[reg];
				}

				unsigned short nizi = memorija[operand + payload];
				unsigned short visi = memorija[operand + payload + 1];
				operand = visi * 256 + nizi;

			}
			else if (nacinAdr == 4) { //memdir

				unsigned short payload = citajDvaBajta(0);
				unsigned short nizi = memorija[payload];
				unsigned short visi = memorija[payload + 1];
				operand = visi * 256 + nizi;
			}
			else if (nacinAdr == 5) { //regdisp

				short payload = citajDvaBajta(0);
				unsigned char reg = regs & 0x0f;
				if (reg > 8) {
					greska = "Pogresno prosledjen registar insturkciji JNE!\n";
					PC = prethodniPC;
					skociNaPrekidnuRutinu(1);
					continue;
				}

				if (reg == 8) {
					operand = PSW;
				}
				else if (reg == 7) {
					operand = PC;
				}
				else if (reg == 6) {
					operand = SP;
				}
				else {
					operand = registri[reg];
				}

				operand = operand + payload;
			}
			else {
				greska = "Pogresno prosledjen nacin adresiranja instrukciji JNE!\n";
				PC = prethodniPC;
				skociNaPrekidnuRutinu(1);
				continue;
			}

			if (proveriFlegove(2)) {
				PC = operand;
			}
		}
		else if (ins == 83) { //jgt
			unsigned char regs = citajBajt();

			unsigned char adrMod = citajBajt();
			unsigned char nacinAdr = adrMod & 0x0f;

			unsigned short operand;

			if (nacinAdr == 0) { //immed
				unsigned short payload = citajDvaBajta(0);
				operand = payload;
			}
			else if (nacinAdr == 1) { //regdir
				unsigned char reg = regs & 0x0f;
				if (reg > 8 ) {
					greska = "Pogresno prosledjen registar insturkciji JGT!\n";
					PC = prethodniPC;
					skociNaPrekidnuRutinu(1);
					continue;
				}
				
				if (reg == 8) {
					operand = PSW;
				}
				else if (reg == 7) {
					operand = PC;
				}
				else if (reg == 6) {
					operand = SP;
				}
				else {
					operand = registri[reg];
				}
			}
			else if (nacinAdr == 2) { //regind
				unsigned char reg = regs & 0x0f;
				if (reg > 8) {
					greska = "Pogresno prosledjen registar insturkciji JGT!\n";
					PC = prethodniPC;
					skociNaPrekidnuRutinu(1);
					continue;
				}

				if (reg == 8) {
					operand = PSW;
				}
				else if (reg == 7) {
					operand = PC;
				}
				else if (reg == 6) {
					operand = SP;
				}
				else {
					operand = registri[reg];
				}

				unsigned short nizi = memorija[operand ];
				unsigned short visi = memorija[operand + 1];
				operand = visi * 256 + nizi;
			}
			else if (nacinAdr == 3) { //reginddisp

				short payload = citajDvaBajta(0);
				unsigned char reg = regs & 0x0f;
				if (reg > 8) {
					greska = "Pogresno prosledjen registar insturkciji JGT!\n";
					PC = prethodniPC;
					skociNaPrekidnuRutinu(1);
					continue;
				}

				if (reg == 8) {
					operand = PSW;
				}
				else if (reg == 7) {
					operand = PC;
				}
				else if (reg == 6) {
					operand = SP;
				}
				else {
					operand = registri[reg];
				}

				unsigned short nizi = memorija[operand + payload];
				unsigned short visi = memorija[operand + payload + 1];
				operand = visi * 256 + nizi;

			}
			else if (nacinAdr == 4) { //memdir

				unsigned short payload = citajDvaBajta(0);
				unsigned short nizi = memorija[payload];
				unsigned short visi = memorija[payload + 1];
				operand = visi * 256 + nizi;
			}
			else if (nacinAdr == 5) { //regdisp

				short payload = citajDvaBajta(0);
				unsigned char reg = regs & 0x0f;
				if (reg > 8) {
					greska = "Pogresno prosledjen registar insturkciji JGT!\n";
					PC = prethodniPC;
					skociNaPrekidnuRutinu(1);
					continue;
				}

				if (reg == 8) {
					operand = PSW;
				}
				else if (reg == 7) {
					operand = PC;
				}
				else if (reg == 6) {
					operand = SP;
				}
				else {
					operand = registri[reg];
				}

				operand = operand + payload;
			}
			else {
				greska = "Pogresno prosledjen nacin adresiranja instrukciji JGT!\n";
				PC = prethodniPC;
				skociNaPrekidnuRutinu(1);
				continue;
			}

			if (proveriFlegove(3)) {
				PC = operand;
			}
		}
		else if (ins == 96) { //xchg

			unsigned char regs = citajBajt();
			unsigned char prviReg = (regs >> 4) & 0x0f;
			unsigned char drugiReg = regs & 0x0f;

			if (prviReg >8 || drugiReg >8) {
				greska = "Pogresno prosledjeni registri insturkciji ADD!\n";
				PC = prethodniPC;
				skociNaPrekidnuRutinu(1);
				continue;
			}

			unsigned short temp;

			if (prviReg == 8) { //1 - PSW
				if (drugiReg == 8) {
					continue;
				}
				else if (drugiReg == 7) { //2 - PC
					temp = PSW;
					PSW = PC;
					PC = temp;
				}
				else if (drugiReg == 6) { //2 - SP
					temp = PSW;
					PSW = SP;
					SP = temp;
				}
				else {
					temp = PSW;
					PSW = registri[drugiReg];
					registri[drugiReg] = temp;
				}
			}
			else if (prviReg == 7) { // 1 - PC
				if (drugiReg == 8) { // 2 - PSW
					temp = PSW;
					PSW = PC;
					PC = temp;
				}
				else if (drugiReg == 7) { 
					continue;
				}
				else if (drugiReg == 6) { //2 - SP
					temp = SP;
					SP = PC;
					PC = temp;
				}
				else {
					temp = PC;
					PC = registri[drugiReg];
					registri[drugiReg] = temp;
				}
			}
			else if (prviReg == 6) { // 1 - SP
				if (drugiReg == 8) { // 2 - PSW
					temp = PSW;
					PSW = SP;
					SP = temp;
				}
				else if (drugiReg == 7) { //2 - PC
					temp = SP;
					SP = PC;
					PC = temp;
				}
				else if (drugiReg == 6) { 
					continue;
				}
				else {
					temp = SP;
					SP = registri[drugiReg];
					registri[drugiReg] = temp;
				}
			}
			else { //1 - R[0..5]
				if (drugiReg == 8) { // 2 - PSW
					temp = PSW;
					PSW = registri[prviReg];
					registri[prviReg] = temp;
				}
				else if (drugiReg == 7) {
					temp = PC;
					PC = registri[prviReg];
					registri[prviReg] = temp;
				}
				else if (drugiReg == 6) { //2 - SP
					temp = SP;
					SP = registri[prviReg];
					registri[prviReg] = temp;
				}
				else {
					temp = registri[prviReg];
					registri[prviReg] = registri[drugiReg];
					registri[drugiReg] = temp;
				}
			}
		}
		else if (ins == 112) { //add
			unsigned char regs = citajBajt();
			unsigned char prviReg = (regs >> 4) & 0x0f;
			unsigned char drugiReg = regs & 0x0f;
			if (prviReg >8 || drugiReg >8) {
				greska = "Pogresno prosledjeni registri insturkciji ADD!\n";
				PC = prethodniPC;
				skociNaPrekidnuRutinu(1);
				continue;
			}
			unsigned short dr;
			if (drugiReg == 8) {
				dr = PSW;
			}
			else if (drugiReg == 7) {
				dr = PC;
			}
			else if (drugiReg == 6) {
				dr = SP;
			}
			else {
				dr = registri[drugiReg];
			}

			if (prviReg == 8) {
				PSW = PSW + dr;
			}
			else if (prviReg == 7) {
				PC = PC + dr;
			}
			else if (prviReg == 6) {
				SP = SP + dr;
			}
			else {
				registri[prviReg] = registri[prviReg] + dr;
			}
			
		}
		else if (ins == 113) { //sub
			unsigned char regs = citajBajt();
			unsigned char prviReg = (regs >> 4) & 0x0f;
			unsigned char drugiReg = regs & 0x0f;
			if (prviReg > 8 || drugiReg > 8) {
				greska = "Pogresno prosledjeni registri insturkciji SUB!\n";
				PC = prethodniPC;
				skociNaPrekidnuRutinu(1);
				continue;
			}
			unsigned short dr;
			if (drugiReg == 8) {
				dr = PSW;
			}
			else if (drugiReg == 7) {
				dr = PC;
			}
			else if (drugiReg == 6) {
				dr = SP;
			}
			else {
				dr = registri[drugiReg];
			}

			if (prviReg == 8) {
				PSW = PSW - dr;
			}
			else if (prviReg == 7) {
				PC = PC - dr;
			}
			else if (prviReg == 6) {
				SP = SP - dr;
			}
			else {
				registri[prviReg] = registri[prviReg] - dr;
			}
		}
		else if (ins == 114) { //mul
			unsigned char regs = citajBajt();
			unsigned char prviReg = (regs >> 4) & 0x0f;
			unsigned char drugiReg = regs & 0x0f;
			if (prviReg >8 || drugiReg >8) {
				greska = "Pogresno prosledjeni registri insturkciji MUL!\n";
				PC = prethodniPC;
				skociNaPrekidnuRutinu(1);
				continue;
			}
			unsigned short dr;
			if (drugiReg == 8) {
				dr = PSW;
			}
			else if (drugiReg == 7) {
				dr = PC;
			}
			else if (drugiReg == 6) {
				dr = SP;
			}
			else {
				dr = registri[drugiReg];
			}

			if (prviReg == 8) {
				PSW = PSW * dr;
			}
			else if (prviReg == 7) {
				PC = PC * dr;
			}
			else if (prviReg == 6) {
				SP = SP * dr;
			}
			else {
				registri[prviReg] = registri[prviReg] * dr;
			}
		}
		else if (ins == 115) { //div
			unsigned char regs = citajBajt();
			unsigned char prviReg = (regs >> 4) & 0x0f;
			unsigned char drugiReg = regs & 0x0f;
			if (prviReg >8 || drugiReg >8) {
				greska = "Pogresno prosledjeni registri insturkciji DIV!\n";
				PC = prethodniPC;
				skociNaPrekidnuRutinu(1);
				continue;
			}
			unsigned short dr;
			if (drugiReg == 8) {
				dr = PSW;
			}
			else if (drugiReg == 7) {
				dr = PC;
			}
			else if (drugiReg == 6) {
				dr = SP;
			}
			else {
				dr = registri[drugiReg];
			}

			if (prviReg == 8) {
				PSW = PSW / dr;
			}
			else if (prviReg == 7) {
				PC = PC / dr;
			}
			else if (prviReg == 6) {
				SP = SP / dr;
			}
			else {
				registri[prviReg] = registri[prviReg] / dr;
			}
		}
		else if (ins == 116) { //cmp
			unsigned char regs = citajBajt();
			unsigned char prviReg = (regs >> 4) & 0x0f;
			unsigned char drugiReg = regs & 0x0f;
			if (prviReg >8 || drugiReg >8) {
				greska = "Pogresno prosledjeni registri insturkciji CMP!\n";
				PC = prethodniPC;
				skociNaPrekidnuRutinu(1);
				continue;
			}
			unsigned short dr;
			if (drugiReg == 8) {
				dr = PSW;
			}
			else if (drugiReg == 7) {
				dr = PC;
			}
			else if (drugiReg == 6) {
				dr = SP;
			}
			else {
				dr = registri[drugiReg];
			}

			unsigned short pr;
			if (prviReg == 8) {
				pr = PSW;
			}
			else if (prviReg == 7) {
				pr = PC;
			}
			else if (prviReg == 6) {
				pr = SP;
			}
			else {
				pr = registri[prviReg];
			}
			postaviFlegove(4, pr, dr);

		}
		else if (ins == 128) { //not
			unsigned char regs = citajBajt();
			unsigned char prviReg = (regs >> 4) & 0x0f;
			if (prviReg >8 ) {
				greska = "Pogresno prosledjen registar insturkciji NOT!\n";
				PC = prethodniPC;
				skociNaPrekidnuRutinu(1);
				continue;
			}

			if (prviReg == 8) {
				PSW = ~PSW;
			}
			else if (prviReg == 7) {
				PC = ~PC;
			}
			else if (prviReg == 6) {
				SP = ~SP;
			}
			else {
				registri[prviReg] = ~registri[prviReg];
			}
			
		}
		else if (ins == 129) { //and
			unsigned char regs = citajBajt();
			unsigned char prviReg = (regs >> 4) & 0x0f;
			unsigned char drugiReg = regs & 0x0f;
			if (prviReg >8 || drugiReg >8) {
				greska = "Pogresno prosledjeni registri insturkciji AND!\n";
				PC = prethodniPC;
				skociNaPrekidnuRutinu(1);
				continue;
			}
			unsigned short dr;
			if (drugiReg == 8) {
				dr = PSW;
			}
			else if (drugiReg == 7) {
				dr = PC;
			}
			else if (drugiReg == 6) {
				dr = SP;
			}
			else {
				dr = registri[drugiReg];
			}

			if (prviReg == 8) {
				PSW = PSW & dr;
			}
			else if (prviReg == 7) {
				PC = PC & dr;
			}
			else if (prviReg == 6) {
				SP = SP & dr;
			}
			else {
				registri[prviReg] = registri[prviReg] & dr;
			}
			
		}
		else if (ins == 130) { //or
			unsigned char regs = citajBajt();
			unsigned char prviReg = (regs >> 4) & 0x0f;
			unsigned char drugiReg = regs & 0x0f;
			if (prviReg >8 || drugiReg >8) {
				greska = "Pogresno prosledjeni registri insturkciji OR!\n";
				PC = prethodniPC;
				skociNaPrekidnuRutinu(1);
				continue;
			}
			unsigned short dr;
			if (drugiReg == 8) {
				dr = PSW;
			}
			else if (drugiReg == 7) {
				dr = PC;
			}
			else if (drugiReg == 6) {
				dr = SP;
			}
			else {
				dr = registri[drugiReg];
			}

			if (prviReg == 8) {
				PSW = PSW | dr;
			}
			else if (prviReg == 7) {
				PC = PC | dr;
			}
			else if (prviReg == 6) {
				SP = SP | dr;
			}
			else {
				registri[prviReg] = registri[prviReg] | dr;
			}
			
		}
		else if (ins == 131) { //xor
			unsigned char regs = citajBajt();
			unsigned char prviReg = (regs >> 4) & 0x0f;
			unsigned char drugiReg = regs & 0x0f;
			if (prviReg >8 || drugiReg >8) {
				greska = "Pogresno prosledjeni registri insturkciji XOR!\n";
				PC = prethodniPC;
				skociNaPrekidnuRutinu(1);
				continue;
			}
			unsigned short dr;
			if (drugiReg == 8) {
				dr = PSW;
			}
			else if (drugiReg == 7) {
				dr = PC;
			}
			else if (drugiReg == 6) {
				dr = SP;
			}
			else {
				dr = registri[drugiReg];
			}

			if (prviReg == 8) {
				PSW = PSW ^ dr;
			}
			else if (prviReg == 7) {
				PC = PC ^ dr;
			}
			else if (prviReg == 6) {
				SP = SP ^ dr;
			}
			else {
				registri[prviReg] = registri[prviReg] ^ dr;
			}
			
		}
		else if (ins == 132) { //test
			unsigned char regs = citajBajt();
			unsigned char prviReg = (regs >> 4) & 0x0f;
			unsigned char drugiReg = regs & 0x0f;
			if (prviReg >8 || drugiReg >8) {
				greska = "Pogresno prosledjeni registri insturkciji TEST!\n";
				PC = prethodniPC;
				skociNaPrekidnuRutinu(1);
				continue;
			}
			unsigned short dr;
			if (drugiReg == 8) {
				dr = PSW;
			}
			else if (drugiReg == 7) {
				dr = PC;
			}
			else if (drugiReg == 6) {
				dr = SP;
			}
			else {
				dr = registri[drugiReg];
			}

			unsigned short pr;
			if (prviReg == 8) {
				pr = PSW;
			}
			else if (prviReg == 7) {
				pr = PC;
			}
			else if (prviReg == 6) {
				pr = SP;
			}
			else {
				pr = registri[prviReg];
			}

			postaviFlegove(2, pr, dr);

		}
		else if (ins == 144) { //shl
			unsigned char regs = citajBajt();
			unsigned char prviReg = (regs >> 4) & 0x0f;
			unsigned char drugiReg = regs & 0x0f;
			if (prviReg >8 || drugiReg >8) {
				greska = "Pogresno prosledjeni registri insturkciji SHL!\n";
				PC = prethodniPC;
				skociNaPrekidnuRutinu(1);
				continue;
			}
			unsigned short dr;
			if (drugiReg == 8) {
				dr = PSW;
			}
			else if (drugiReg == 7) {
				dr = PC;
			}
			else if (drugiReg == 6) {
				dr = SP;
			}
			else {
				dr = registri[drugiReg];
			}

			if (prviReg == 8) {
				PSW = PSW << dr;
				postaviFlegove(31, PSW, dr);
			}
			else if (prviReg == 7) {
				PC = PC << dr;
				postaviFlegove(31, PC, dr);
			}
			else if (prviReg == 6) {
				SP = SP << dr;
				postaviFlegove(31, SP, dr);
			}
			else {
				registri[prviReg] = registri[prviReg] << dr;
				postaviFlegove(31, registri[prviReg], dr);
			}
			
			
		}
		else if (ins == 145) { //shr
			unsigned char regs = citajBajt();
			unsigned char prviReg = (regs >> 4) & 0x0f;
			unsigned char drugiReg = regs & 0x0f;
			if (prviReg >8 || drugiReg >8) {
				greska = "Pogresno prosledjeni registri insturkciji SHR!\n";
				PC = prethodniPC;
				skociNaPrekidnuRutinu(1);
				continue;
			}
			unsigned short dr;
			if (drugiReg == 8) {
				dr = PSW;
			}
			else if (drugiReg == 7) {
				dr = PC;
			}
			else if (drugiReg == 6) {
				dr = SP;
			}
			else {
				dr = registri[drugiReg];
			}

			if (prviReg == 8) {
				PSW = PSW >> dr;
				postaviFlegove(32, PSW, dr);
			}
			else if (prviReg == 7) {
				PC = PC >> dr;
				postaviFlegove(32,PC, dr);
			}
			else if (prviReg == 6) {
				SP = SP >> dr;
				postaviFlegove(32,SP, dr);
			}
			else {
				registri[prviReg] = registri[prviReg] >> dr;
				postaviFlegove(32, registri[prviReg], dr);
			}
			
		}
		else if (ins == 160) { //ldr,pop
		
			unsigned char regs = citajBajt();
			unsigned char prviReg = (regs >> 4) & 0x0f;

			unsigned char adrMod = citajBajt();
			unsigned char nacinAdr = adrMod & 0x0f;

			unsigned short operand;

			if (nacinAdr == 0) { //immed
				unsigned short payload = citajDvaBajta(0);
				operand = payload;
			}
			else if (nacinAdr == 1) { //regdir
				unsigned char reg = regs & 0x0f;
				if (reg > 8 ) {
					greska = "Pogresno prosledjen drugi registar insturkciji LDR!\n";
					PC = prethodniPC;
					skociNaPrekidnuRutinu(1);
					continue;
				}
				
				if (reg == 8) {
					operand = PSW;
				}
				else if (reg == 7) {
					operand = PC;
				}
				else if (reg == 6) {
					operand = SP;
				}
				else {
					operand = registri[reg];
				}
			}
			else if (nacinAdr == 2) { //regind
				unsigned char reg = regs & 0x0f;
				if (reg > 8) {
					greska = "Pogresno prosledjen registar insturkciji LDR!\n";
					PC = prethodniPC;
					skociNaPrekidnuRutinu(1);
					continue;
				}

				if (reg == 8) {
					operand = PSW;
				}
				else if (reg == 7) {
					operand = PC;
				}
				else if (reg == 6) {
					operand = SP;
				}
				else {
					operand = registri[reg];
				}


				unsigned char azur = (adrMod >> 4) & 0x0f;
				if (azur == 4) { //POP
					SP = SP + 2;
				}
				else if (azur != 0) { //0 - LDR
					greska = "Pogresno prosledjen nacin azuriranja kod regind adresiranja!\n";
					PC = prethodniPC;
					skociNaPrekidnuRutinu(1);
					continue;
				}


				unsigned short nizi = memorija[operand];
				unsigned short visi = memorija[operand + 1];
				operand = visi * 256 + nizi;
			}
			else if (nacinAdr == 3) { //reginddisp

				short payload = citajDvaBajta(0);
				unsigned char reg = regs & 0x0f;
				if (reg > 8) {
					greska = "Pogresno prosledjen registar insturkciji LDR!\n";
					PC = prethodniPC;
					skociNaPrekidnuRutinu(1);
					continue;
				}

				if (reg == 8) {
					operand = PSW;
				}
				else if (reg == 7) {
					operand = PC;
				}
				else if (reg == 6) {
					operand = SP;
				}
				else {
					operand = registri[reg];
				}

				unsigned short nizi = memorija[operand+payload];
				unsigned short visi = memorija[operand+payload+ 1];
				operand = visi * 256 + nizi;

			}
			else if (nacinAdr == 4) { //memdir

				unsigned short payload = citajDvaBajta(0);
				//operand = memorija[payload];
				//payload je sad adresa sa koje citamo dva bajta kako bismo dobili operand
	
				unsigned short nizi = memorija[payload];
				unsigned short visi = memorija[payload+1];
				operand = visi * 256 + nizi;

			}
			else {
				greska = "Pogresno prosledjen nacin adresiranja instrukciji LDR!\n";
				PC = prethodniPC;
				skociNaPrekidnuRutinu(1);
				continue;
			}

			if (prviReg > 8) {
				greska = "Pogresno prosledjen prvi registar instrukciji LDR!\n";
				PC = prethodniPC;
				skociNaPrekidnuRutinu(1);
				continue;
			}
			else if (prviReg == 8) {
				PSW = operand;
			}
			else if (prviReg == 7) {
				PC = operand;
			}
			else if (prviReg == 6) {
				SP = operand;
			}
			else {
				registri[prviReg] = operand;
			}


		}
		else if (ins == 176) { //str,push
			unsigned char regs = citajBajt();
			unsigned char prviReg = (regs >> 4) & 0x0f;
			unsigned short pr;
			if (prviReg > 8) {
				greska = "Pogresno prosledjen prvi registar instrukciji STR!\n";
				PC = prethodniPC;
				skociNaPrekidnuRutinu(1);
				continue;
			}
			else if (prviReg == 8) {
				pr = PSW;
			}
			else if (prviReg == 7) {
				pr = PC;
			}
			else if (prviReg == 6) {
				pr = SP;
			}
			else {
				pr = registri[prviReg];
			}

			unsigned char adrMod = citajBajt();
			unsigned char nacinAdr = adrMod & 0x0f;

			unsigned short operand;

			if (nacinAdr == 0) { //immed
				greska = "Instrukcija store i neposredna vrednost su zabranjena kombinacija!\n";
				PC = prethodniPC;
				skociNaPrekidnuRutinu(1);
				continue;
			}
			else if (nacinAdr == 1) { //regdir
				unsigned char reg = regs & 0x0f;
				if (reg > 8 ) {
					greska = "Pogresno prosledjen drugi registar insturkciji STR!\n";
					PC = prethodniPC;
					skociNaPrekidnuRutinu(1);
					continue;
				}
				
				if (reg == 8) {
					PSW = pr;
				}
				else if (reg == 7) {
					PC = pr;
				}
				else if (reg == 6) {
					SP = pr;
				}
				else {
					registri[reg] = pr;
				}
			}
			else if (nacinAdr == 2) { //regind
				unsigned char reg = regs & 0x0f;
				if (reg > 8) {
					greska = "Pogresno prosledjen drugi registar insturkciji STR!\n";
					PC = prethodniPC;
					skociNaPrekidnuRutinu(1);
					continue;
				}

				unsigned char azur = (adrMod >> 4) & 0x0f;
				if (azur == 1) { //PUSH
					SP = SP - 2;
				}
				else if (azur != 0) { //0 - STR
					greska = "Pogresno prosledjen nacin azuriranja kod regind adresiranja!\n";
					PC = prethodniPC;
					skociNaPrekidnuRutinu(1);
					continue;
				}

				if (reg == 8) {
					upisiDvaBajta(pr, PSW, 1);
				}
				else if (reg == 7) {
					upisiDvaBajta(pr, PC, 1);
				}
				else if (reg == 6) {
					upisiDvaBajta(pr, SP, 1);
				}
				else {
					upisiDvaBajta(pr, registri[reg], 1);
				}

				
			}
			else if (nacinAdr == 3) { //reginddisp

				short payload = citajDvaBajta(0);
				unsigned char reg = regs & 0x0f;
				if (reg > 8) {
					greska = "Pogresno prosledjen drugi registar insturkciji STR!\n";
					PC = prethodniPC;
					skociNaPrekidnuRutinu(1);
					continue;
				}

				if (reg == 8) {
					upisiDvaBajta(pr, PSW + payload, 1);
				}
				else if (reg == 7) {
					upisiDvaBajta(pr, PC + payload, 1);
				}
				else if (reg == 6) {
					upisiDvaBajta(pr, SP + payload, 1);
				}
				else {
					upisiDvaBajta(pr, registri[reg] + payload, 1);
				}

			}
			else if (nacinAdr == 4) { //memdir

				unsigned short payload = citajDvaBajta(0);
				upisiDvaBajta(pr, payload, 1);
			}
			else {
				greska = "Pogresno prosledjen nacin adresiranja instrukciji STR!\n";
				PC = prethodniPC;
				skociNaPrekidnuRutinu(1);
				continue;
			}

		}
		else {
			greska = "Ne postoji instrukcija sa prvim bajtom " + ins;
			PC = prethodniPC;
			skociNaPrekidnuRutinu(1);
			continue;
		}
		tick();
		procitajKarakter();
		obradiZahteveZaPrekidom();
	}

	vratiTerminal();

}


int Emulator::hex2int(string h)
{
	h = h.substr(2);
	int result = 0;
	for (int i = 0; i < h.length(); i++) {
		if (h[i] >= 48 && h[i] <= 57)
		{
			result += (h[i] - (char)48) * pow(16, h.length() - i - 1);
		}
		else if (h[i] >= 65 && h[i] <= 70) {
			result += ((int)h[i] - 55) * pow(16, h.length() - i - 1);
		}
		else if (h[i] >= 97 && h[i] <= 102) {
			result += ((int)h[i] - 87) * pow(16, h.length() - i - 1);
		}
	}
	return result;
}

char Emulator::citajBajt()
{
	char ret = memorija[PC++];
	return ret;
}

short Emulator::citajDvaBajta(int endian)
{
	short ret = 0;
	if (endian == 0) { //big
		short visi = memorija[PC++];
		short nizi = memorija[PC++];
	}
	else { //little
		short nizi = memorija[PC++];
		short visi = memorija[PC++];
	}
	if(visi<0 && nizi<0) {
		visi++;
	}
	ret = visi * 256 + nizi;
	return ret;
}

void Emulator::upisiBajt(char vrednost, unsigned short adresa)
{
	memorija[adresa] = vrednost;
}

void Emulator::upisiDvaBajta(short vrednost, unsigned short adresa, int endian)
{
	unsigned char nizi = vrednost & 0xff;
	unsigned char visi = (vrednost >> 8) & 0xff;
	if (endian == 0) { //big
		memorija[adresa++] = visi;
		memorija[adresa] = nizi;
	}
	else { //little
		memorija[adresa++] = nizi;
		memorija[adresa] = visi;
	}
	adresa--;
	if(adresa==65280){ //TERM_OUT
        cout<<(char)vrednost<<flush;
    }
}

void Emulator::postaviFlegove(int br, short reg1, short reg2)
{
	if (br == 2) { //TEST
		short temp = reg1 & reg2;

		//Z
		if (temp == 0) {
			postaviFleg(0, 1);
		}
		else {
			postaviFleg(0, 0);
		}

		//N
		if (temp < 0) {
			postaviFleg(3, 1);
		}
		else {
			postaviFleg(3, 0);
		}
	}
	else if (br == 31 || br==32) { //SHL,SHR
		short temp;
		if (br == 31) {
			temp = reg1 << reg2;
		}
		else {
			temp = reg1 >> reg2;
		}

		//Z
		if (temp == 0) {
			postaviFleg(0, 1);
		}
		else {
			postaviFleg(0, 0);
		}

		//N
		if (temp < 0) {
			postaviFleg(3, 1);
		}
		else {
			postaviFleg(3, 0);
		}

		//C
		if (br == 31) {
			if (reg2 < 16 && ((reg1 >>(16-reg2)) & 1)) {
				postaviFleg(2, 1);
			}
			else {
				postaviFleg(2, 0);
			}
		}
		else {
			if ((reg1 >> (reg2-1)) & 1) {
				postaviFleg(2, 1);
			}
			else {
				postaviFleg(2, 0);
			}
		}
		
	}
	else if (br == 4) { //CMP
		short temp = reg1 - reg2;

		//Z
		if (temp == 0) {
			postaviFleg(0, 1);
		}
		else {
			postaviFleg(0, 0);
		}

		//N
		if (temp < 0) {
			postaviFleg(3, 1);
		}
		else {
			postaviFleg(3, 0);
		}

		//C
		if (reg1 < reg2) {
			postaviFleg(2, 1);
		}
		else {
			postaviFleg(2, 0);
		}

		//O
		if (((reg1 - reg2) < 0 && reg1 > 0 && reg2 < 0) || ((reg1-reg2)>0 && reg1<0 && reg2>0)) {
			postaviFleg(1, 1);
		}
		else {
			postaviFleg(1, 0);
		}
	}
}

bool Emulator::proveriFlegove(int ins)
{
	if (ins == 1) { //JEQ
		return (PSW & 1);
	}
	else if (ins == 2) {//JNE
		return !(PSW & 1);
	}
	else if (ins == 3) { //JGT
		return (!(PSW & 1) && !((PSW & 8) ^ (PSW & 2)));
	}else{
        return false;
    }
}

void Emulator::postaviFleg(int poz, bool sr)
{
	if (sr) { //set
		PSW = PSW | (1 << poz);
	}
	else { //reset
		PSW = PSW & ~(1 << poz);
	}
}

bool Emulator::proveriFleg(int poz)
{
	return (PSW & (1<<poz));
}

void Emulator::obradiZahteveZaPrekidom()
{
	if (proveriFleg(15)) { //I=1
		return;
	}

	if (prekidTajmer && !proveriFleg(13)) {
		prekidTajmer = false;
		skociNaPrekidnuRutinu(2);
		//cout<< "T"<<endl;
		return;
	}

	if (prekidTerminal && !proveriFleg(14)) {
		prekidTerminal = false;
		skociNaPrekidnuRutinu(3);
	}
}

void Emulator::skociNaPrekidnuRutinu(int ulaz)
{
	unsigned short adresaUlaza = (ulaz % 8) * 2;
	unsigned char nizi = memorija[adresaUlaza++];
	unsigned char visi = memorija[adresaUlaza];
	unsigned short adresaPrekRutine = visi * 256 + nizi;

	//stavi PC na stek
	SP = SP - 2;
	upisiDvaBajta(PC, SP, 1);

	//stavi PSW na stek
	SP = SP - 2;
	upisiDvaBajta(PSW, SP, 1);

	//novi PC
	PC = adresaPrekRutine;

	postaviFleg(13, 1);
	postaviFleg(14, 1);
	postaviFleg(15, 1);
}

void Emulator::pokreniTajmer()
{
	radiTajmer = true;
	trenutnoVreme = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

void Emulator::tick()
{
    //cout<<"USAO U TICK"<<endl;
	long long int prethodnoVreme = trenutnoVreme;
	trenutnoVreme = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	long long int perioda = izracunajPerioduGenZP();
    //cout<<"TV = "<<trenutnoVreme<<endl;
    //cout<<"PV = "<<prethodnoVreme<<endl;
	if (perioda == 0) {
		greska = "Nepostojeca vrednost u konfiguracionom registru tim_cfg!\n";
		skociNaPrekidnuRutinu(1);
		return;
	}

	if (!radiTajmer) { //pokreni ga
		radiTajmer = true;
	}
	else if (radiTajmer && (trenutnoVreme - prethodnoVreme) > perioda) {
		radiTajmer = false;
		prekidTajmer = true;
        //cout<<"P"<<endl;

	}
	else {
		trenutnoVreme = prethodnoVreme;
	}
}

long long int Emulator::izracunajPerioduGenZP()
{
	short nizi = memorija[0xFF10];
	short visi = memorija[0xFF11];
	short id = visi * 256 + nizi;

	if (id == 0) {
		return 500000; // 0,5s = 500ms = 500000 mikro sec
	}
	else if (id == 1) {
		return 1000000;
	}
	else if (id == 2) {
		return 1500000;
	}
	else if (id == 3) {
		return 2000000;
	}
	else if (id == 4) {
		return 5000000;
	}
	else if (id == 5) {
		return 10000000;
	}
	else if (id == 6) {
		return 3000000;
	}
	else if (id == 7) {
		return 6000000;
	}
	else {
		return 0;
	}
}

struct termios staraPodesavanja;

void vrati() {
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &staraPodesavanja);
}

bool Emulator::podesiTerminal()
{
    if(tcgetattr(STDIN_FILENO, &staraPodesavanja)==0){

        static struct termios novaPodesavanja = staraPodesavanja;
        novaPodesavanja.c_lflag &= ~(ECHO|ECHONL|ICANON|IEXTEN);
		novaPodesavanja.c_cflag |= CS8;
		novaPodesavanja.c_cflag &= ~(CSIZE|PARENB);
        novaPodesavanja.c_cc[VTIME]=0;
        novaPodesavanja.c_cc[VMIN]=0;

        if(atexit(vrati)==0){

            if(tcsetattr(STDIN_FILENO,TCSAFLUSH,&novaPodesavanja)==0){
                return true;
            }
        }
    }

    return false;
}

void Emulator::vratiTerminal()
{
	vrati();
}

void Emulator::procitajKarakter()
{
    char c;
    if(read(STDIN_FILENO, &c,1)==1){
		upisiDvaBajta(c,0xFF02,1);
        prekidTerminal=true;
    }
}