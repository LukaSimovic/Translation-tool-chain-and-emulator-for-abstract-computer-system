#include"../inc/asembler.h"
#include"../inc/regexi.h"
#include<iostream>
#include<sstream>
#include<math.h>

using namespace std;

int Simbol::noviRBR = -1;

Asembler::Asembler()
{
	greske = "";

	//inicijalizacija promenljivih
	trenutnaSekcija = "";
	lc = 0;
	brABSsimbola = 0;
	linijaKoda = 1;

	//dodavanje sekcija UNDEFINED I ABSOLUTE u tabelu simbola pod imenima -1 i 0 respektivno.
	Simbol und("undefined", "undefined", 0, "100", 0);
	dodajSimbUtabSimb(und);
	Simbol abs("absolute", "absolute", 0, "100", 0);
	dodajSimbUtabSimb(abs);
}

Asembler::Asembler(string ulaz, string izlaz)
{
	greske = "";

	ulazni_fajl.open(ulaz);
	if (ulazni_fajl.is_open() == false) {
		cout << "Neuspesno otvoren ulazni fajl!" << endl;
		exit(1);
	}

	izlazni_fajl.open(izlaz);
	if (izlazni_fajl.is_open() == false) {
		cout << "Neuspesno otvoren izlazni fajl!" << endl;
		exit(1);
	}

	//cout << "Fajlovi uspesno otvoreni" << endl << endl;

	//inicijalizacija promenljivih
	trenutnaSekcija = "";
	lc = 0;
	brABSsimbola = 0;
	linijaKoda = 1;

	//dodavanje sekcija UNDEFINED I ABSOLUTE u tabelu simbola pod imenima -1 i 0 respektivno.
	Simbol und("undefined", "undefined", 0, "100", 0);
	dodajSimbUtabSimb(und);
	Simbol abs("absolute", "absolute", 0, "100", 0);
	dodajSimbUtabSimb(abs);
}

void Asembler::postaviUlaz(string u)
{
	ulazni_fajl.open(u);
	if (ulazni_fajl.is_open() == false) {
		cout << "Neuspesno otvoren ulazni fajl!" << endl;
		exit(1);
	}
}

void Asembler::postaviIzlaz(string i)
{
	izlazni_fajl.open(i);
	if (izlazni_fajl.is_open() == false) {
		cout << "Neuspesno otvoren izlazni fajl!" << endl;
		exit(1);
	}
}

void Asembler::zatvoriFajlove()
{
	ulazni_fajl.close();
	izlazni_fajl.close();

	//cout << endl << "Fajlovi uspesno zatvoreni" << endl;
}

void Asembler::pokreni()
{
	bool pp = prviProlaz();
	if (pp) {
		//cout << "Uspesan prvi prolaz!" << endl;
		linijaKoda = 1;
		ulazni_fajl.clear();
		ulazni_fajl.seekg(0, ulazni_fajl.beg);

		bool dp = drugiProlaz();
		if (dp) {
			ispisiTabSimb();
			ispisiTabSek();
			ispisiTabRelZap();
		}
		else {
			cout << "\nPROGRAM PREKINUT ZBOG GRESKE: " << endl;
			cout << greske << endl;
		}

	}
	else {
		cout << "\nPROGRAM PREKINUT ZBOG GRESKE: " << endl;
		cout << greske << endl;
	}


}

bool Asembler::prviProlaz()
{
	bool bioEnd = false;
	string red;

	while (true) {

		if (red == "") {
			if (!getline(ulazni_fajl, red)) {
				break;
			}
		}

		//komentari
		int taraba;
		if ((taraba = red.find("#")) != string::npos) {
			if (taraba != 0) { //komentar nije ceo red, vec postoji nesto pre #
				red = red.substr(0, taraba);
			}
			else { //komentar je ceo red, preskoci red
				linijaKoda++;
				red = "";
				continue;
			}
		}

		//brisanje tabova i duplih razmaka
		red = srediRed(red);

		//sintaksna analiza
		if (regex_match(red, labela)) {
			//cout << "Pronadjena samo labela" << endl;

			string ime = red.substr(0, red.size() - 1);
			if (trenutnaSekcija != "") {
				if (simbVecUTab(ime) != -2) {

					int poz = simbVecUTab(ime), pom = -1;
					list <Simbol> ::iterator it;
					for (it = tabelaSimbola.begin(); it != tabelaSimbola.end(); ++it) {
						if (pom == poz) {
							if ((*it).dohvatiLGE() == "010" && (*it).dohvatiSekciju() == "-1") { //promeniti sekciju i ofset samo
								(*it).postaviSekciju(trenutnaSekcija);
								(*it).postaviOfset(lc);
								brABSsimbola++;
							}
							else if ((*it).dohvatiLGE() == "001") {
								stringstream lss;
								string slinija;
								lss << linijaKoda;
								lss >> slinija;
								greske = "Linija " + slinija + ": Simbol sa imenom " + ime + " je vec uvezen iz drugog fajla, ne moze se definisati simbol sa istim imenom!";
								return false;
							}
							else if ((*it).dohvatiSekciju() != "-1") {
								stringstream lss;
								string slinija;
								lss << linijaKoda;
								lss >> slinija;
								greske = "Linija " + slinija + ": Simbol sa  imenom " + ime + " je vec definisan!";
								return false;
							}
							break;
						}
						else {
							pom++;
						}
					}
				}
				else { //upisi nov simbol u tabelu
					Simbol novi(ime, trenutnaSekcija, lc, "100", -1);
					dodajSimbUtabSimb(novi);
				}
			}
			else {
				stringstream lss;
				string slinija;
				lss << linijaKoda;
				lss >> slinija;
				greske = "Linija " + slinija + ": Labela " + ime + " mora biti definisana u okviru neke sekcije!";
				return false;
			}



		}
		else if (regex_match(red, labelaIkomanda)) {
			//cout << "Pronadjena labela i nesto posle toga" << endl;
			// cout << "\tNesto posle toga: ";
			int pom = red.find(":");

			//labela
			string lab = red.substr(0, pom);

			if (trenutnaSekcija != "") {
				if (simbVecUTab(lab) != -2) {

					int poz = simbVecUTab(lab), pom = -1;
					list <Simbol> ::iterator it;
					for (it = tabelaSimbola.begin(); it != tabelaSimbola.end(); ++it) {
						if (pom == poz) {
							if ((*it).dohvatiLGE() == "010" && (*it).dohvatiSekciju() == "-1") { //promeniti sekciju i ofset samo
								(*it).postaviSekciju(trenutnaSekcija);
								(*it).postaviOfset(lc);
								brABSsimbola++;
							}
							else if ((*it).dohvatiLGE() == "001") {
								stringstream lss;
								string slinija;
								lss << linijaKoda;
								lss >> slinija;
								greske = "Linija " + slinija + ": Simbol sa imenom " + lab + " je vec uvezen iz drugog fajla, ne moze se definisati simbol sa istim imenom!";
								return false;
							}
							else if ((*it).dohvatiSekciju() != "-1") {
								stringstream lss;
								string slinija;
								lss << linijaKoda;
								lss >> slinija;
								greske = "Linija " + slinija + ": Simbol sa  imenom " + lab + " je vec definisan!";
								return false;
							}
							break;
						}
						else {
							pom++;
						}
					}

				}
				else { //upisi nov simbol u tabelu
					Simbol novi(lab, trenutnaSekcija, lc, "100", -1);
					dodajSimbUtabSimb(novi);
				}
			}
			else {
				stringstream lss;
				string slinija;
				lss << linijaKoda;
				lss >> slinija;
				greske = "Linija " + slinija + ": Labela " + lab + " mora biti definisana u okviru neke sekcije!";
				return false;
			}



			//direktiva ili naredba
			red = red.substr(pom + 1);
			continue; //ostatak se obradjuje u sledecoj iteraciji

		}
		else if (regex_match(red, direktivaGlobal)) {
			//cout << "Pronadjena global direktiva" << endl;

			int razmak = red.find(" ");
			red = red.substr(razmak + 1); //sad je red lista simbola, npr: "a,b,c,d"
			string simb = "";
			bool poslednji = false;
			while ((red.find(",") != string::npos) || poslednji == false) {

				int zarez = red.find(",");
				simb = red.substr(0, zarez);

				//obrada simbola
				if (simbVecUTab(simb) != -2) {

					int poz = simbVecUTab(simb), pom = -1;
					list <Simbol> ::iterator it;
					for (it = tabelaSimbola.begin(); it != tabelaSimbola.end(); ++it) {
						if (pom == poz) {
							string loc = (*it).dohvatiLGE();
							if (loc == "100") { // samo oznaciti da je globalan
								(*it).postaviLGE("010");
							}
							else if (loc == "010") { // vec oznacen kao globalan
								stringstream lss;
								string slinija;
								lss << linijaKoda;
								lss >> slinija;
								greske = "Linija " + slinija + ": Simbol " + simb + " je vec oznacen kao globan, ne moze se dva puta izvoziti!";
								return false;
							}
							else if (loc == "001") { // uvezen iz neke druge tabele
								stringstream lss;
								string slinija;
								lss << linijaKoda;
								lss >> slinija;
								greske = "Linija " + slinija + ": Simbol " + simb + " je uvezen iz neke druge tabele i ne moze se izvoziti!";
								return false;
							}
							break;
						}
						else {
							pom++;
						}

					}

				}
				else { //dodaj u tab simb kao nedefinisan
					Simbol novi(simb, "-1", -1, "010", -1);
					dodajSimbUtabSimb(novi);
				}

				if (red.find(",") == string::npos) {
					poslednji = true;
				}
				else {
					red = red.substr(zarez + 1);
				}

			}
		}
		else if (regex_match(red, direktivaSection)) {
			//cout << "Pronadjena section direktiva" << endl;

			int razmak = red.find(" ");
			string ime = red.substr(razmak + 1);
			if (simbVecUTab(ime) == -2) {

				if (trenutnaSekcija != "") { //zavrsi prethodnu sekciju

					list <Simbol> ::iterator it;
					for (it = tabelaSimbola.begin(); it != tabelaSimbola.end(); ++it) {
						int rb = (*it).dohvatiRBR();
						stringstream ss2;
						string str_rb;
						ss2 << rb;
						ss2 >> str_rb;
						if (str_rb == trenutnaSekcija) {
							(*it).postaviSize(lc);
						}
					}
				}
				lc = 0;
				stringstream ss;
				ss << (dohvMaxRBRTabSimb() + 1);
				ss >> trenutnaSekcija;


				Simbol noviSimb(ime, ime, 0, "100", 0);
				dodajSimbUtabSimb(noviSimb);
			}
			else {
				stringstream lss;
				string slinija;
				lss << linijaKoda;
				lss >> slinija;
				greske = "Linija " + slinija + ": Sekcija ne sme biti dva puta definisana niti se sme zvati isto kao simbol!";
				return false;
			}


		}
		else if (regex_match(red, direktivaEnd)) {
			//cout << "Pronadjena end direktiva" << endl;

			if (trenutnaSekcija != "") {
				list <Simbol> ::iterator it;
				for (it = tabelaSimbola.begin(); it != tabelaSimbola.end(); ++it) {
					int rb = (*it).dohvatiRBR();
					stringstream ss2;
					string str_rb;
					ss2 << rb;
					ss2 >> str_rb;
					if (str_rb == trenutnaSekcija) {
						(*it).postaviSize(lc);
					}
				}
			}
			bioEnd = true;
			lc = 0;
			trenutnaSekcija = "";
			brABSsimbola = 0;
			break; //izlaz iz while petlje
		}
		else if (regex_match(red, direktivaExtern)) {
			//cout << "Pronadjena extern direktiva" << endl;

			int razmak = red.find(" ");
			red = red.substr(razmak + 1); //sad je red lista simbola, npr: "a,b,c,d"
			string simb = "";
			bool poslednji = false;
			while ((red.find(",") != string::npos) || poslednji == false) {

				int zarez = red.find(",");
				simb = red.substr(0, zarez);

				//obrada simbola
				if (simbVecUTab(simb) != -2) { //greska
					stringstream lss;
					string slinija;
					lss << linijaKoda;
					lss >> slinija;
					greske = "Linija " + slinija + ": Simbol " + simb + " je vec definisan u fajlu pa se ne moze uvoziti";
					return false;
				}
				else {
					Simbol novi(simb, "-1", -1, "001", -1);
					dodajSimbUtabSimb(novi);
				}

				if (red.find(",") == string::npos) {
					poslednji = true;
				}
				else {
					red = red.substr(zarez + 1);
				}
			}


		}
		else if (regex_match(red, direktivaEqu)) {
			//cout << "Pronadjena equ direktiva" << endl;
			int razmak = red.find(" ");
			int zarez = red.find(",");
			string simb = red.substr(razmak + 1, zarez - razmak - 1);
			string vrednost = red.substr(zarez + 1);

			if (simbVecUTab(simb) != -2) { //moze ako je pre toga bilo .global simb

				int poz = simbVecUTab(simb), pom = -1;
				list <Simbol> ::iterator it;
				for (it = tabelaSimbola.begin(); it != tabelaSimbola.end(); ++it) {
					if (pom == poz) {
						if ((*it).dohvatiLGE() == "010" && (*it).dohvatiSekciju() == "-1") { //promeniti sekciju i ofset samo
							(*it).postaviSekciju("0");
							(*it).postaviOfset(brABSsimbola * 2);

						}
						else if ((*it).dohvatiLGE() == "001") {
							stringstream lss;
							string slinija;
							lss << linijaKoda;
							lss >> slinija;
							greske = "Linija " + slinija + ": Simbol " + simb + " je vec uvezen iz drugog fajla, ne moze se definisati simbol sa istim imenom!";
							return false;
						}
						else if ((*it).dohvatiSekciju() != "-1") {
							stringstream lss;
							string slinija;
							lss << linijaKoda;
							lss >> slinija;
							greske = "Linija " + slinija + ": Simbol " + simb + " je vec definisan!";
							return false;
						}
						break;
					}
					else {
						pom++;
					}
				}

			}
			else {
				Simbol novi(simb, "0", brABSsimbola * 2, "100", -1);
				dodajSimbUtabSimb(novi);
			}

			stringstream ss;
			ss << hex << (brABSsimbola * 2);
			string os = ss.str(); //offset
			brABSsimbola++;
			list <Simbol> ::iterator it2;
			for (it2 = tabelaSimbola.begin(); it2 != tabelaSimbola.end(); ++it2) {
				if ((*it2).dohvatiRBR() == 0) {
					(*it2).postaviSize(2 * brABSsimbola);
				}
			}

			vrednost = lit2sadrzaj(vrednost);
			vrednost = obrniBajtove(vrednost); //little-endian
			SadrzajSekcije sadsek("0", os, vrednost);
			dodajRedUtabSek(sadsek);
		}
		else if (regex_match(red, direktivaSkip)) {
			//cout << "Pronadjena skip direktiva" << endl;

			int razmak = red.find(" ");
			string lit = red.substr(razmak + 1);

			if (regex_match(lit, literalDecR)) {
				stringstream ss(lit);
				int num = 0;
				ss >> num;
				lc += num;
			}
			else { //lit je hex
				int dec = hex2int(lit);
				lc += dec;
			}

		}
		else if (regex_match(red, direktivaWord)) {
			//cout << "Pronadjena word direktiva" << endl;
			int brZareza = 0;
			int z = 0;
			while ((z = red.find(",")) != string::npos) {
				brZareza++;
				red = red.substr(z + 1);
			}
			lc += 2 * (brZareza + 1);
		}
		else if (regex_match(red, insBezOper)) {
			lc += 1;
		}
		else if (regex_match(red, ins1Reg)) {
			if (red.find("p") != string::npos) { //int i not ne sadrze slovo p (za razliku od push i pop)
				lc += 3;
			}
			else {
				lc += 2;
			}
		}
		else if (regex_match(red, ins2Reg)) {
			lc += 2;
		}
		else if (regex_match(red, ins1Op)) { //ins skoka

			int razmak = red.find(" ");
			string ins = red;
			red = red.substr(razmak + 1);
			if (regex_match(red, skokRegDir) || regex_match(red, skokRegInd)) {
				lc += 3;
			}
			else if (regex_match(red, skokAps) || regex_match(red, skokPcRel) || regex_match(red, skokMemDir) || regex_match(red, skokRegIndDisp)) {
				lc += 5;
			}
			else {
				stringstream lss;
				string slinija;
				lss << linijaKoda;
				lss >> slinija;
				greske = "Linija " + slinija + ": Pogresno uneta instrukcija: " + ins + ", neispravan operand!";
				return false;
			}
		}
		else if (regex_match(red, ins1Reg1Op)) { //ldr,str
			int zarez = red.find(",");
			string ins = red;
			red = red.substr(zarez + 1);
			if (regex_match(red, dataRegDir) || regex_match(red, dataRegInd)) {
				lc += 3;
			}
			else if (regex_match(red, dataAps) || regex_match(red, dataPcRel) || regex_match(red, dataMemDir) || regex_match(red, dataRegIndDisp)) {
				lc += 5;
			}
			else {
				stringstream lss;
				string slinija;
				lss << linijaKoda;
				lss >> slinija;
				greske = "Linija " + slinija + ": Pogresno uneta instrukcija: " + ins + ", neispravan operand!";
				return false;
			}
		}
		else if (red == "") { //preskoci prazan red
			linijaKoda++;
			continue;
		}
		else {
			stringstream lss;
			string slinija;
			lss << linijaKoda;
			lss >> slinija;
			greske = "Linija " + slinija + ": NEISPRAVNO UNETA KOMANDA";
			return false;
		}
		red = "";
		linijaKoda++;
	}

	//provera da li se asembliranje zavrsilo end direktivom
	if (bioEnd == false) {
		greske = "GRESKA: Nedostaje .end direktiva!";
		return false;
	}

	//provera da li je svaki simbol koji se izvozi definisan
	list <Simbol> ::iterator it;
	for (it = tabelaSimbola.begin(); it != tabelaSimbola.end(); ++it) {

		if ((*it).dohvatiSekciju() == "-1" && (*it).dohvatiLGE() != "001") {
			greske = "GRESKA: Postoji simbol koji se izvozi, a nije definisan!";
			return false;
		}
	}

	return true;
}

bool Asembler::drugiProlaz()
{
	string red = "";
	while (true) {

		if (red == "") { //procitaj novi red
			if (!getline(ulazni_fajl, red)) { //kraj fajla, izlaz iz while(true)
				break;
			}
		}

		//komentari
		int taraba;
		if ((taraba = red.find("#")) != string::npos) {
			if (taraba != 0) { //komentar nije ceo red, vec postoji nesto pre #
				red = red.substr(0, taraba);
			}
			else { //komentar je ceo red, preskoci red
				red = "";
				linijaKoda++;
				continue;
			}
		}


		//brisanje tabova i duplih razmaka
		red = srediRed(red);

		if (red == "") {
			linijaKoda++;
			continue;
		}
		else if (regex_match(red, direktivaGlobal) || regex_match(red, direktivaExtern) || regex_match(red, labela) || regex_match(red, direktivaEnd) || regex_match(red, direktivaEqu)) {
			//cout << "Ne radim nista, obradjeno sve u prvom prolazu!" << endl;
		}
		else if (regex_match(red, labelaIkomanda)) {
			//cout << "Labela i komanda"<<endl;

			int pom = red.find(":");
			red = red.substr(pom + 1);
			continue;
		}
		else if (regex_match(red, direktivaWord)) {
			//cout << "Word direktiva" << endl;

			int razmak = red.find(" ");
			red = red.substr(razmak + 1); //sad je red lista inicijalizatora, npr: "a,b,c,d"
			string inic = "";
			bool poslednji = false;
			while ((red.find(",") != string::npos) || poslednji == false) {

				int zarez = red.find(",");
				inic = red.substr(0, zarez);

				//obrada jednog inicijalizatora
				stringstream ss;
				ss << hex << lc;
				string os = ss.str(); //offset

				string pom = "";
				if (regex_match(inic, literalR)) {
					inic = lit2sadrzaj(inic);
					inic = obrniBajtove(inic); //little-endian
					SadrzajSekcije sadsek(trenutnaSekcija, os, inic);
					dodajRedUtabSek(sadsek);
				}
				else { //simbol
					inic = obradiSimbolAbsolute(inic, os, 1);
					if (inic == "") { return false; }
					inic = obrniBajtove(inic);
					SadrzajSekcije sadsek(trenutnaSekcija, os, inic);
					dodajRedUtabSek(sadsek);
				}

				if (red.find(",") == string::npos) {
					poslednji = true;
				}
				else {
					red = red.substr(zarez + 1);
				}

				lc += 2;
			}
		}
		else if (regex_match(red, direktivaSkip)) {
			//cout << "Skip direktiva" << endl;

			int razmak = red.find(" ");
			string lit = red.substr(razmak + 1);

			stringstream ss;
			ss << hex << lc;
			string os = ss.str(); //offset
			string sadrzaj = "";
			if (regex_match(lit, literalDecR)) {
				int brNula = 0;
				stringstream ss0;
				ss0 << lit;
				ss0 >> brNula;

				for (int i = 0; i < brNula; i++) {
					sadrzaj += "00";
					if (i != brNula - 1)
						sadrzaj += " ";
				}

				lc += brNula;
			}
			else { //lit je hex
				int dec = hex2int(lit);

				for (int i = 0; i < dec; i++) {
					sadrzaj += "00";
					if (i != dec - 1)
						sadrzaj += " ";
				}

				lc += dec;
			}
			SadrzajSekcije sadsek(trenutnaSekcija, os, sadrzaj);
			dodajRedUtabSek(sadsek);


		}
		else if (regex_match(red, direktivaSection)) {

			lc = 0;

			int razmak = red.find(" ");
			red = red.substr(razmak + 1);

			int rb = simbVecUTab(red);

			stringstream ss;
			ss << rb;
			ss >> trenutnaSekcija;
		}
		else if (regex_match(red, insBezOper)) {

			stringstream ss;
			ss << hex << lc;
			string os = ss.str(); //offset
			string sadrzaj;

			if (red == "halt") {
				sadrzaj = "00";
			}
			else if (red == "ret") {
				sadrzaj = "40";
			}
			else if (red == "iret") {
				sadrzaj = "20";
			}

			SadrzajSekcije sadsek(trenutnaSekcija, os, sadrzaj);
			dodajRedUtabSek(sadsek);

			lc += 1;
		}
		else if (regex_match(red, ins1Reg)) {

			stringstream ss;
			ss << hex << lc;
			string os = ss.str(); //offset
			string sadrzaj;

			int razmak = red.find(" ");
			string ins = red.substr(0, razmak);
			red = red.substr(razmak + 1); //registar
			red = reg2sadrzaj(red);

			if (ins == "int") {
				sadrzaj = "10 " + red + "f";
				lc += 2;
			}
			else if (ins == "not") {
				sadrzaj = "80 " + red + "0";
				lc += 2;
			}
			else if (ins == "push") {
				sadrzaj = "b0 " + red + "6 12";
				lc += 3;
			}
			else if (ins == "pop") {
				sadrzaj = "a0 " + red + "6 42";
				lc += 3;
			}

			SadrzajSekcije sadsek(trenutnaSekcija, os, sadrzaj);
			dodajRedUtabSek(sadsek);
		}
		else if (regex_match(red, ins2Reg)) {

			stringstream ss;
			ss << hex << lc;
			string os = ss.str(); //offset
			string sadrzaj;

			int razmak = red.find(" ");
			string ins = red.substr(0, razmak);
			red = red.substr(razmak + 1);
			int zarez = red.find(",");
			string prviReg = red.substr(0, zarez);
			string drugiReg = red.substr(zarez + 1);
			prviReg = reg2sadrzaj(prviReg);
			drugiReg = reg2sadrzaj(drugiReg);

			if (ins == "add") {
				sadrzaj = "70 " + prviReg + drugiReg;
			}
			else if (ins == "sub") {
				sadrzaj = "71 " + prviReg + drugiReg;
			}
			else if (ins == "mul") {
				sadrzaj = "72 " + prviReg + drugiReg;
			}
			else if (ins == "div") {
				sadrzaj = "73 " + prviReg + drugiReg;
			}
			else if (ins == "cmp") {
				sadrzaj = "74 " + prviReg + drugiReg;
			}
			else if (ins == "or") {
				sadrzaj = "82 " + prviReg + drugiReg;
			}
			else if (ins == "xor") {
				sadrzaj = "83 " + prviReg + drugiReg;
			}
			else if (ins == "and") {
				sadrzaj = "81 " + prviReg + drugiReg;
			}
			else if (ins == "test") {
				sadrzaj = "84 " + prviReg + drugiReg;
			}

			SadrzajSekcije sadsek(trenutnaSekcija, os, sadrzaj);
			dodajRedUtabSek(sadsek);

			lc += 2;
		}
		else if (regex_match(red, ins1Op)) { //ins skoka

			stringstream ss;
			ss << hex << lc;
			string os = ss.str(); //offset
			string sadrzaj;

			ss.str("");
			ss << hex << (lc + 3);
			string os2 = ss.str(); //adresa adresnog polja ins skoka (ona ce biti upisana kao mesto koriscenja u rel zapisu)

			int razmak = red.find(" ");
			string ins = red.substr(0, razmak);
			string oper = red.substr(razmak + 1);

			if (ins == "jmp") {
				sadrzaj = "50 f";
			}
			else if (ins == "jgt") {
				sadrzaj = "53 f";
			}
			else if (ins == "jeq") {
				sadrzaj = "51 f";
			}
			else if (ins == "jne") {
				sadrzaj = "52 f";
			}
			else if (ins == "call") {
				sadrzaj = "30 f";
			}

			if (regex_match(oper, skokRegDir)) {

				oper = reg2sadrzaj(oper.substr(1));
				sadrzaj += oper + " 01";
				lc += 3;
			}
			else if (regex_match(oper, skokRegInd)) {
				int zz = oper.find("]");
				oper = reg2sadrzaj(oper.substr(2, zz - 2));
				sadrzaj += oper + " 02";
				lc += 3;
			}
			else if (regex_match(oper, skokAps)) {

				sadrzaj += "f 00 "; //ostalo jos adresno polje

				if (regex_match(oper, literalR)) {
					oper = lit2sadrzaj(oper);
					sadrzaj += oper;
				}
				else { //operand je simbol
					string ret = obradiSimbolAbsolute(oper, os2, 0);
					if (ret == "") { return false; }
					else { sadrzaj += ret; }
				}
				lc += 5;
			}
			else if (regex_match(oper, skokPcRel)) {
				oper = oper.substr(1); //odseci %
				sadrzaj += "7 05 "; //ostalo jos adresno polje

				string ret = obradiSimbolPcRelative(oper, os2, 0);
				if (ret == "") { return false; }
				else { sadrzaj += ret; }
				lc += 5;
			}
			else if (regex_match(oper, skokMemDir)) {

				oper = oper.substr(1); //odseci *
				sadrzaj += "F 04 "; //ostalo jos adresno polje

				if (regex_match(oper, literalR)) {
					oper = lit2sadrzaj(oper);
					sadrzaj += oper;
				}
				else { //operand je simbol
					string ret = obradiSimbolAbsolute(oper, os2, 0);
					if (ret == "") { return false; }
					else { sadrzaj += ret; }

				}
				lc += 5;
			}
			else if (regex_match(oper, skokRegIndDisp)) {
				int plus = oper.find("+");
				string reg = reg2sadrzaj(oper.substr(2, plus - 2));
				sadrzaj += reg + " 03 "; //ostalo jos adresno polje
				string pom = oper.substr(plus + 1, oper.length() - plus - 2);
				if (regex_match(pom, literalR)) {
					pom = lit2sadrzaj(pom);
					sadrzaj += pom;
				}
				else { //pomeraj je simbol
					string ret = obradiSimbolAbsolute(pom, os2, 0);
					if (ret == "") { return false; }
					else { sadrzaj += ret; }
				}
				lc += 5;
			}

			SadrzajSekcije sadsek(trenutnaSekcija, os, sadrzaj);
			dodajRedUtabSek(sadsek);

		}
		else if (regex_match(red, ins1Reg1Op)) { //ldr,str

			stringstream ss;
			ss << hex << lc;
			string os = ss.str(); //offset
			string sadrzaj;

			ss.str("");
			ss << hex << (lc + 3);
			string os2 = ss.str(); //adresa adresnog polja ins skoka (ona ce biti upisana kao mesto koriscenja u rel zapisu)

			int zarez = red.find(",");
			string ins = red.substr(0, 3);
			string reg = red.substr(4, zarez - 4);
			string oper = red.substr(zarez + 1);

			if (ins == "ldr") {
				sadrzaj = "a0 ";
			}
			else if (ins == "str") {
				sadrzaj = "b0 ";
			}
			reg = reg2sadrzaj(reg);
			sadrzaj += reg;


			if (regex_match(oper, dataRegDir)) {
				oper = reg2sadrzaj(oper);
				sadrzaj += oper + " 01";
				lc += 3;
			}
			else if (regex_match(oper, dataRegInd)) {
				oper = reg2sadrzaj(oper.substr(1, oper.length() - 2));
				sadrzaj += oper + " 02";
				lc += 3;
			}
			else if (regex_match(oper, dataAps)) {
				oper = oper.substr(1); //skloni dolar
				if (regex_match(oper, literalR)) {
					oper = lit2sadrzaj(oper);
					sadrzaj += "f 00 " + oper;
				}
				else { //operand je simbol
					string ret = obradiSimbolAbsolute(oper, os2, 0);
					if (ret == "") { return false; }
					else { sadrzaj += "f 00 " + ret; }
				}
				lc += 5;
			}
			else if (regex_match(oper, dataPcRel)) {
				oper = oper.substr(1); //odseci %
				string ret = obradiSimbolPcRelative(oper, os2, 0);
				if (ret == "") { return false; }
				else if(ret.find("-")!=string::npos){
					ret = ret.substr(1);
					sadrzaj+=ret;
				}
				else { sadrzaj += "7 03 " + ret; }
				lc += 5;
			}
			else if (regex_match(oper, dataMemDir)) {
				if (regex_match(oper, literalR)) {
					oper = lit2sadrzaj(oper);
					sadrzaj += "f 04 " + oper;
				}
				else { //operand je simbol
					string ret = obradiSimbolAbsolute(oper, os2, 0);
					if (ret == "") { return false; }
					else { sadrzaj += "f 04 " + ret; }
				}
				lc += 5;
			}
			else if (regex_match(oper, dataRegIndDisp)) {
				int plus = oper.find("+");
				string reg = oper.substr(1, plus - 1);
				string pom = oper.substr(plus + 1, oper.length() - plus - 2);
				reg = reg2sadrzaj(reg);
				if (regex_match(pom, literalR)) {
					pom = lit2sadrzaj(pom);
					sadrzaj += reg + " 03 " + pom;
				}
				else { //pomeraj je simbol
					string ret = obradiSimbolAbsolute(pom, os2, 0);
					if (ret == "") { return false; }
					else { sadrzaj += reg + " 03 " + ret; }
				}
				lc += 5;
			}
			SadrzajSekcije sadsek(trenutnaSekcija, os, sadrzaj);
			dodajRedUtabSek(sadsek);
		}

		//isprazni red da bi uzeo sledeci
		linijaKoda++;
		red = "";

	}


	return true;
}

void Asembler::dodajSimbUtabSimb(Simbol simb)
{
	tabelaSimbola.push_back(simb);
}

int Asembler::simbVecUTab(string ime)
{
	list <Simbol> ::iterator it;
	for (it = tabelaSimbola.begin(); it != tabelaSimbola.end(); ++it) {

		if (ime == (*it).dohvatiIme()) {
			return (*it).dohvatiRBR();
		}
	}

	return -2;
}

int Asembler::dohvMaxRBRTabSimb()
{
	return tabelaSimbola.back().dohvatiRBR();
}

string Asembler::srediRed(string red)
{
	istringstream linija(red);
	red = "";
	string rec;
	while (linija >> rec) {
		red += rec;
		red += " ";
	}
	red = red.substr(0, red.size() - 1); //skloni poslednji razmak

	return red;
}

int Asembler::hex2int(string h)
{
	h = h.substr(2);
	int result = 0;
	for (int i = 0; i < h.length(); i++) {
		if (h[i] >= 48 && h[i] <= 57)
		{
			result += (h[i] - 48) * pow(16, h.length() - i - 1);
		}
		else if (h[i] >= 65 && h[i] <= 70) {
			result += (h[i] - 55) * pow(16, h.length() - i - 1);
		}
		else if (h[i] >= 97 && h[i] <= 102) {
			result += (h[i] - 87) * pow(16, h.length() - i - 1);
		}
	}
	return result;
}

string Asembler::lit2sadrzaj(string lit)
{
	string pom = "";
	if (regex_match(lit, literalDecR)) { //LITERAL JE DEC
		stringstream iss(lit);
		int br;
		iss >> br;
		stringstream hss;
		hss << hex << br;
		lit = hss.str();
	}
	else { //LITERAL JE HEX
		lit = lit.substr(2); //odseci 0x
	}
	int duzina = lit.length();
	if (duzina > 4) { //odseci vise bite
		lit = lit.substr(duzina - 4);
	}
	else if (duzina < 4) {
		for (int i = 0; i < 4 - duzina; i++) {
			pom += "0";
		}
		pom += lit;
		lit = pom;
	}
	//little-endian
	string visiBajt = lit.substr(0, 2);
	string niziBajt = lit.substr(2);
	lit = visiBajt + " " + niziBajt;
	return lit;
}

string Asembler::sadrzaj2lit(string sadrzaj)
{
	string visiBajt = sadrzaj.substr(0, 2);
	string niziBajt = sadrzaj.substr(3);
	sadrzaj = visiBajt + niziBajt;
	sadrzaj = "0x" + sadrzaj;
	return sadrzaj;
}

string Asembler::obrniBajtove(string rec)
{
	string visiBajt = rec.substr(0, 2);
	string niziBajt = rec.substr(3);
	rec = niziBajt + " " + visiBajt;
	return rec;
}

string Asembler::reg2sadrzaj(string reg)
{
	if (reg.find("r") != string::npos) {
		return reg.substr(1);
	}
	else { //PSW
		return "8";
	}
}

void Asembler::ispisiTabSimb()
{
	izlazni_fajl << "TABELA SIMBOLA:" << endl << "Ime\t\tSekcija\t\tOffset\tLGE\tSize\tRBR" << endl <<
		"-------------------------------------------------------------" << endl;

	list <Simbol> ::iterator it;
	for (it = tabelaSimbola.begin(); it != tabelaSimbola.end(); ++it) {
		izlazni_fajl << *it;
	}


}

void Asembler::dodajRedUtabSek(SadrzajSekcije ss)
{
	tabelaSekcija.push_back(ss);
}

void Asembler::ispisiTabSek()
{
	izlazni_fajl << "\nTABELE SEKCIJA:" << endl;
	list <Simbol> ::iterator itSimb;
	list <SadrzajSekcije> ::iterator itSek;
	for (itSimb = tabelaSimbola.begin(); itSimb != tabelaSimbola.end(); ++itSimb) {

		if ((*itSimb).dohvatiSize() != -1 && (*itSimb).dohvatiIme() != "undefined") { //sekcija je

			izlazni_fajl << " --- SEKCIJA: " << (*itSimb).dohvatiIme() << " ---" << endl;
			izlazni_fajl << "Offset\tSadrzaj" << endl;

			for (itSek = tabelaSekcija.begin(); itSek != tabelaSekcija.end(); ++itSek) {

				stringstream ssrbr;
				ssrbr << (*itSimb).dohvatiRBR();
				string rb;
				ssrbr >> rb;

				if ((*itSek).dohvatiSekciju() == rb) {
					izlazni_fajl << (*itSek);
				}
			}

			izlazni_fajl << "\n";
		}
	}

}

void Asembler::dodajRelZapUtab(RelokZapis rk)
{
	tabelaRelokZapisa.push_back(rk);
}

string Asembler::obradiSimbolAbsolute(string simb, string os, int endian)
{
	if (simbVecUTab(simb) != -2) {
		int rb = simbVecUTab(simb);
		list <Simbol> ::iterator it;
		for (it = tabelaSimbola.begin(); it != tabelaSimbola.end(); ++it) {
			if ((*it).dohvatiRBR() == rb) {
				if ((*it).dohvatiSekciju() == "0") { //simb definisan sa equ
					int o = (*it).dohvatiOffset();
					string pom;
					stringstream ss2;
					ss2 << hex << o;
					pom = ss2.str();
					list <SadrzajSekcije> ::iterator it2;
					for (it2 = tabelaSekcija.begin(); it2 != tabelaSekcija.end(); ++it2) {

						if ((*it2).dohvatiSekciju() == "0" && (*it2).dohvatiOffset() == pom) {
							simb = (*it2).dohvatiSadrzaj();
							simb = obrniBajtove(simb);
							break;
						}
					}
				}
				else { //mora da se pravi rel zapis
					if ((*it).dohvatiLGE() == "100") { //lokalni
						int vr = (*it).dohvatiOffset();
						stringstream ss2;
						ss2 << vr;
						ss2 >> simb;
						simb = lit2sadrzaj(simb);

						string sek = (*it).dohvatiSekciju();
						stringstream ss3(sek);
						ss3 >> rb;
					}
					else { //globalni
						simb = "00 00";
					}
					RelokZapis relzap(trenutnaSekcija, os, 0, rb, 0, endian);
					dodajRelZapUtab(relzap);
				}
				break;
			}

		}
	}
	else {
		stringstream lss;
		string slinija;
		lss << linijaKoda;
		lss >> slinija;
		greske = "Linija " + slinija + ": Simbol " + simb + " ne postoji u tabeli simbola, pa se ne moze prevesti instrukcija!";
		return "";
	}
	return simb;
}

string Asembler::obradiSimbolPcRelative(string simb, string os, int endian)
{
	if (simbVecUTab(simb) != -2) {

		int rb = simbVecUTab(simb);
		list <Simbol> ::iterator it;
		for (it = tabelaSimbola.begin(); it != tabelaSimbola.end(); ++it) {

			if (rb == (*it).dohvatiRBR()) {
				if ((*it).dohvatiSekciju() == "0") { //equ
					simb = "-f 04 ";
					string a = ""; 
					int o = (*it).dohvatiOffset();
					string pom;
					stringstream ss2;
					ss2 << hex << o;
					pom = ss2.str();
					list <SadrzajSekcije> ::iterator it2;
					for (it2 = tabelaSekcija.begin(); it2 != tabelaSekcija.end(); ++it2) {

						if ((*it2).dohvatiSekciju() == "0" && (*it2).dohvatiOffset() == pom) {
							a = (*it2).dohvatiSadrzaj();
							a = obrniBajtove(a);
							simb+=a;
							break;
						}
					}
					
				}
				else {
					if (trenutnaSekcija == (*it).dohvatiSekciju()) { //ne treba relok zapis, samo izracunati pomeraj
						int tek_vr_simb = (*it).dohvatiOffset();
						int adr_sled_ins = hex2int("0x" + os) + 2;
						int pom = tek_vr_simb - adr_sled_ins;
						stringstream ss;
						ss << hex << pom;
						simb = "0x" + ss.str();
						simb = lit2sadrzaj(simb);
					}
					else { //mora rel zap
						if ((*it).dohvatiLGE() == "100") {
							simb = "00 00";
							int tek_vr_simb = (*it).dohvatiOffset();
							string sek = (*it).dohvatiSekciju();
							stringstream ss(sek);
							int rb_sekcije;
							ss >> rb_sekcije;
							RelokZapis relzap(trenutnaSekcija, os, 1, rb_sekcije, tek_vr_simb - 2, endian);
							dodajRelZapUtab(relzap);
						}
						else { //global
							simb = "00 00";
							RelokZapis relzap(trenutnaSekcija, os, 1, rb, -2, endian);
							dodajRelZapUtab(relzap);
						}

					}
				}
				break;
			}

		}
	}
	else {
		stringstream lss;
		string slinija;
		lss << linijaKoda;
		lss >> slinija;
		greske = "Linija " + slinija + ": Simbol " + simb + " ne postoji u tabeli simbola, pa se ne moze prevesti instrukcija!";
		return "";
	}
	return simb;
}

void Asembler::ispisiTabRelZap()
{
	izlazni_fajl << "\nRELOKACIONI ZAPISI:" << endl;
	izlazni_fajl << "Sekcija\tOffset\tTip relokacije\tSimbol\tAddend\tEndian\n------------------------------------------------\n";
	list <RelokZapis> ::iterator it;
	for (it = tabelaRelokZapisa.begin(); it != tabelaRelokZapisa.end(); ++it) {
		izlazni_fajl << (*it);
	}
}


