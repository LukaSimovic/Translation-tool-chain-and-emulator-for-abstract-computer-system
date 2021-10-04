#include "../inc/linker.h"
#include<iostream>
#include<sstream>
#include<math.h>
Linker::Linker(int m, list<string> as)
{
	brSekcija = 0;
	mod = m;
	if (as.size() != 0 && m==0) {
		adreseSekcija = as;
		brSekcija = as.size();
	}

	
}

Linker::Linker(int m, list<string> ulazi, string izlaz, list<string> as)
{
	mod = m;
	if (as.size() != 0 && m == 0) {
		adreseSekcija = as;
		brSekcija = as.size();
	}

	nazivi_ulaznih_fajlova = ulazi;

	izlazni_fajl.open(izlaz);
	if (izlazni_fajl.is_open() == false) {
		cout << "Neuspesno otvoren izlazni fajl!" << endl;
		exit(1);
	}

}

void Linker::postaviUlaze(list<string> ulazi)
{
	nazivi_ulaznih_fajlova = ulazi;
}

void Linker::postaviIzlaz(string izlaz)
{
	izlazni_fajl.open(izlaz);
	if (izlazni_fajl.is_open() == false) {
		cout << "Neuspesno otvoren izlazni fajl!" << endl;
		exit(1);
	}
}

void Linker::zatvoriIzlazniFajl()
{
	izlazni_fajl.close();
}

void Linker::pokreni()
{
	//cout << "Uspeo sam da pokrenem linker! MOD: " << mod << endl;
	
	if (!obradiUlazneFajlove()) {
		cout << greska << endl;
		exit(1);
	}
	if (mod == 0) { //hex
		if (!rasporediSekcijeHex()) {
			cout << greska << endl;
			exit(1);
		}
		if (!azurirajTabSimbolaHex()) {
			cout << greska << endl;
			exit(1);
		}
		if (!obradiRelokZapise()) {
			cout << greska << endl;
			exit(1);
		}
		generisiZapisHEX();
	}
	else { //linkable
		rasporediSekcijeLinkable();
		azurirajSadrzajeSekcija();
		azurirajTabSimbolaIRelZapiseLinkable();
		ispisiTabele();
	}

	//ispisiPomocneTabele();
}

bool Linker::obradiUlazneFajlove()
{
	int cntFajl = 1;
	list <string> ::iterator it;
	for (it = nazivi_ulaznih_fajlova.begin(); it != nazivi_ulaznih_fajlova.end(); ++it, ++cntFajl) {

		ifstream ulazni_fajl;
		ulazni_fajl.open(*it);
		if (ulazni_fajl.is_open() == false) {
			greska = "Neuspesno otvoren ulazni fajl "+(*it)+"!\n";
			return false;
		}

		string red; int cntRed;

		//TABELA SIMBOLA
		cntRed = 1;
		while (getline(ulazni_fajl, red)) {
			if (cntRed < 6) {
				cntRed++;
				continue;
			}
			if (red == "")
				break;

			istringstream linija(red);
			string podaci[6];
			int pombr = 0;
			string rec;
			while (linija >> rec) {
				podaci[pombr++] = rec;
			}
			if (!(podaci[3] == "100" && podaci[0] != podaci[1])) { //ne ubacuju se lokalni simboli (osim sekcija)
				Simbol simb(cntFajl, podaci[0], podaci[1], podaci[2], podaci[4], podaci[3], podaci[5]);
				tabelaSimbola.push_back(simb);
			}

			cntRed++;
		}
		//provera da li postoji visestruka definicija simbola
		list<Simbol> ::iterator it;
		for (it = tabelaSimbola.begin(); it != tabelaSimbola.end(); ++it) {

			if ((*it).getSize() == "-1") { //samo ako je simbol, nije sekcija
				string ime = (*it).getSimbol();
				list<Simbol> ::iterator it2;
				for (it2 = tabelaSimbola.begin(); it2 != tabelaSimbola.end(); ++it2) {

					if ((*it2).getSimbol() == ime && (*it).getFajl()!=(*it2).getFajl()) {
						if ((*it).getSekcija() != "-1" && (*it2).getSekcija() != "-1") { //nijedan od simbola nije uvezen, oba su definicija
							greska = "Visestruka definicija simbola " + ime + "!\n";
							return false;
						}
					}

				}
			}
			
		}

		//SADRZAJ SEKCIJA
		cntRed = 1; string sekc;
		while (getline(ulazni_fajl, red)) {
			if (red == "RELOKACIONI ZAPISI:") {
				break;
			}
			if (red == "") {
				cntRed = 2;
				continue;
			}
			if (cntRed < 4) {
				if (cntRed == 2) {
					int dvetacke = red.find(":");
					sekc = red.substr(dvetacke + 2);
					int razmak = sekc.find(" ");
					sekc = sekc.substr(0, razmak);
				}
				cntRed++;
				continue;
			}
			
			string of, sad;
			int tab = red.find('\t');
			of = red.substr(0, tab);
			sad = red.substr(tab + 1);
			
			SadrzajSekcije sadsek(cntFajl,sekc, of, sad);
			tabelaSadrzajaSekcija.push_back(sadsek);

			cntRed++;
		}

		//RELOKACIONI ZAPISI
		cntRed = 1;
		while (getline(ulazni_fajl, red)) {
			if (cntRed < 3) {
				cntRed++;
				continue;
			}
			if (red == "") {
				break;
			}

			istringstream linija(red);
			string podaci[6];
			int pombr = 0;
			string rec;
			while (linija >> rec) {
				podaci[pombr++] = rec;
			}
			RelokZapis relzap(cntFajl, podaci[0], podaci[1], podaci[2], podaci[3], podaci[4],podaci[5]);
			tabelaRelZap.push_back(relzap);

			cntRed++;
		}

		ulazni_fajl.close();
	}

	return true;
}

bool Linker::rasporediSekcijeHex()
{
	//RAZRESAVANJE PLACE OPCIJA
	list<string> ::iterator it;
	for (it = adreseSekcija.begin(); it != adreseSekcija.end(); ++it) {
		string opc = *it;
		int monkey = opc.find("@");
		int jednako = opc.find("=");
		string nazivSekc = opc.substr(jednako + 1, monkey - jednako-1);
		string vrednost = opc.substr(monkey + 1);
		int vr = hex2int(vrednost);
		Sekcija sekc(0, nazivSekc, 0, vr,true);
		tabelaSekcija.push_back(sekc);
	}

	//UBACIVANJE SVIH SEKCIJA
	list<Simbol> ::iterator it2; int cnt = 0;
	for (it2 = tabelaSimbola.begin(); it2 != tabelaSimbola.end(); ++it2, ++cnt) {

		if ((*it2).getSimbol() == (*it2).getSekcija() && !sekcijaVecUbacena((*it2).getSimbol())) {

			bool p = false; int poc = 0; int ret;
			if ((ret = postojiSekcijaSaPlace((*it2).getSimbol()))!=-1) {
				p = true; poc = ret;
			}
			Sekcija nova((*it2).getFajl(), (*it2).getSimbol(), string2int((*it2).getSize()), poc,p);
			tabelaSekcija.push_back(nova); //prvi ubacen, nadji iz ostalih fajlova

			nadjiIUbaciOstale((*it2).getSimbol(), poc + string2int((*it2).getSize()), p,cnt);
		}
	}

	//DODAVANJE SEKCIJA LINKERA ZA SEKCIJE KOJE NEMAJU PLACE
	list<Sekcija> ::iterator it3; string preskoci = "";
	for (it3 = tabelaSekcija.begin(); it3 != tabelaSekcija.end(); ++it3) {

		if ((*it3).getFajl() != 0 && (*it3).getIme() != preskoci) {
			preskoci = (*it3).getIme();
			if (postojiSekcijaSaPlace((*it3).getIme())==-1) {
				Sekcija nova(0, (*it3).getIme(), 0, 0, 0);
				tabelaSekcija.push_back(nova);
				brSekcija++;
			}
		}
	}

	//IZRACUNAJ SIZE SEKCIJAMA LINKERA
	list<Sekcija> ::iterator it4; 
	for (it4 = tabelaSekcija.begin(); it4 != tabelaSekcija.end(); ++it4) {

		if ((*it4).getFajl() == 0) { //ovo je sekcija linkera
			list<Sekcija> ::iterator it5; int siz = 0;
			for (it5 = tabelaSekcija.begin(); it5 != tabelaSekcija.end(); ++it5) {
				if ((*it4).getIme() == (*it5).getIme() && (*it5).getFajl() != 0) {
					siz += (*it5).getSize();
				}
			}
			(*it4).setSize(siz);
		}
	}

	//IZRACUNAJ POCETNE ADRESE ONIM SEKCIJAMA KOJE IH NEMAJU
	list<Sekcija> ::iterator it6; bool prva = true;
	for (it6 = tabelaSekcija.begin(); it6 != tabelaSekcija.end(); ++it6) {

		if ((*it6).getFajl() == 0 && (*it6).getPlace()==false) { //ovo je sekcija linkera kojoj treba postavljanje adrese

			if (adreseSekcija.size() == 0 && prva) { //adresni prostor pocinje od 0
				(*it6).setPocAdr(0);
				prva = false;
			}
			else {
				(*it6).setPocAdr(maxPrvaSlobodnaAdresa());
			}
			
			list<Sekcija> ::iterator it7;
			for (it7 = tabelaSekcija.begin(); it7 != tabelaSekcija.end(); ++it7) {
				
				if ((*it6).getIme() == (*it7).getIme() && (*it7).getFajl() != 0) {
					(*it7).setPocAdr((*it7).getPocAdr() + (*it6).getPocAdr());
				}
			}
			
		}
	}

	//PROVERA PREKLAPANJA SEKCIJA
	list<Sekcija> ::iterator it8; bool prvaProvera;
	Sekcija poslednja = tabelaSekcija.back();
	for (it8 = tabelaSekcija.begin(); it8 != tabelaSekcija.end(); ++it8) {

		if ((*it8).getFajl() == 0) {
			/*
			if ((*it8).getIme() == poslednja.getIme() && (*it8).getFajl()==poslednja.getFajl()) {
				break; //ako je sekcija poslednja u tabeli, ne poredi dalje
			}
			*/
			list<Sekcija> ::iterator it9;
			prvaProvera = true;
			for (it9 = ++it8; it9 != tabelaSekcija.end(); ++it9) {
				if (prvaProvera) {
					prvaProvera = false;
					it8--;
				}
				if ((*it9).getFajl() == 0) {
					int poc1 = (*it8).getPocAdr();
					int kraj1 = poc1 + (*it8).getSize();
					int poc2 = (*it9).getPocAdr();
					int kraj2 = poc2 + (*it9).getSize();

					if (poc1<=poc2 && kraj1>poc2) {
						greska = "Sekcije " + (*it8).getIme() + " i " + (*it9).getIme() + " se preklapaju!\n";
						return false;
					}
					else if (poc1 >= poc2 && poc1 < kraj2) {
						greska = "Sekcije " + (*it8).getIme() + " i " + (*it9).getIme() + " se preklapaju!\n";
						return false;
					}
				}
			}
			if (prvaProvera) {
				prvaProvera = false;
				it8--;
			}
			
		}
	}

	return true;
}

int Linker::maxPrvaSlobodnaAdresa() {
	int max = -1; int sizeMaxa;
	list<Sekcija> ::iterator it;
	for (it = tabelaSekcija.begin(); it != tabelaSekcija.end(); ++it) {

		if ((*it).getFajl()==0 && (*it).getPocAdr() > max) {
			max = (*it).getPocAdr();
			sizeMaxa = (*it).getSize();
		}
	}

	return max+sizeMaxa;
}

bool Linker::sekcijaVecUbacena(string sekc) {
	list<Sekcija> ::iterator it;
	for (it = tabelaSekcija.begin(); it != tabelaSekcija.end(); ++it) {
		if ((*it).getIme() == sekc && (*it).getFajl()!=0) {
			return true;
		}
	}
	return false;
}

int Linker::postojiSekcijaSaPlace(string sekc) {

	list<Sekcija> ::iterator it;
	for (it = tabelaSekcija.begin(); it != tabelaSekcija.end(); ++it) {
		if ((*it).getIme() == sekc && (*it).getPlace() == true) {
			return (*it).getPocAdr();
		}
	}
	return -1;

}

void Linker::nadjiIUbaciOstale(string simb,int poc, bool p,int cnt) {

	list<Simbol> ::iterator it; int pom = 0;
	for (it = tabelaSimbola.begin(); it != tabelaSimbola.end(); ++it) {
		if (pom <= cnt) {
			pom++;
			continue; //preskoci prvih cnt, gledaj dalje
		}
		if ((*it).getSimbol() == simb) {
			Sekcija nova((*it).getFajl(), (*it).getSimbol(), string2int((*it).getSize()), poc, p);
			tabelaSekcija.push_back(nova);
			poc += string2int((*it).getSize());
		}
	}
}


bool Linker::azurirajTabSimbolaHex() {

	list <Simbol> ::iterator it;
	for (it = tabelaSimbola.begin(); it != tabelaSimbola.end(); ++it) {

		if ((*it).getSize() == "-1") { //simbol
			if ((*it).getSekcija() != "-1") { //samo ako nije eksterni
				if ((*it).getSekcija() != "0") {
					string nazivSekc;
					string sekc = (*it).getSekcija();
					int fajl = (*it).getFajl();
					list <Simbol> ::iterator it2;
					for (it2 = tabelaSimbola.begin(); it2 != tabelaSimbola.end(); ++it2) {
						if ((*it2).getFajl() == fajl && (*it2).getRbr() == sekc) {
							nazivSekc = (*it2).getSekcija();
							break;
						}
					}
					list<Sekcija> ::iterator it3;
					for (it3 = tabelaSekcija.begin(); it3 != tabelaSekcija.end(); ++it3) {

						if ((*it3).getFajl() == fajl && (*it3).getIme() == nazivSekc) {
							int stariOffset = string2int((*it).getOffset());
							int pomeraj = (*it3).getPocAdr();
							(*it).setOffset(int2string(pomeraj + stariOffset));
							break;
						}
					}
				}
				else { //simbol je apsolutni, trazi vrednost u sadrzaju sekcija
					int fajl = (*it).getFajl();
					string ofs = (*it).getOffset(); //od pocetka absolute sekcije u fajlu int fajl
					list<SadrzajSekcije> ::iterator it2;
					for (it2 = tabelaSadrzajaSekcija.begin(); it2 != tabelaSadrzajaSekcija.end(); ++it2) {
						if ((*it2).getSekcija() == "absolute" && (*it2).getFajl() == fajl && (*it2).getOffset() == ofs) {
							//sadrzaj je njegova vrednost
							string sadrzaj = (*it2).getSadrzaj();
							string visiBajtPodatka = sadrzaj.substr(3);
							string niziBajtPodatka = sadrzaj.substr(0, 2);
							int vr = hex2int("0x" + visiBajtPodatka + niziBajtPodatka);
							(*it).setOffset(int2string(vr));
						}
					}
				}
			}
			
			
		}
		else { //sekcija
			int fajl = (*it).getFajl();
			string sekc = (*it).getSekcija();
			list<Sekcija> ::iterator it3;
			for (it3 = tabelaSekcija.begin(); it3 != tabelaSekcija.end(); ++it3) {

				if ((*it3).getFajl() == fajl && (*it3).getIme() == sekc) {
					int pomeraj = (*it3).getPocAdr();
					(*it).setOffset(int2string(pomeraj));
					break;
				}
			}
		}
	}

	return true;
}

bool Linker::obradiRelokZapise() {

	list<RelokZapis> ::iterator it;
	for (it = tabelaRelZap.begin(); it != tabelaRelZap.end(); ++it) {
		string vrednost="";
		string imeSekcije="";
		string imeSimbola;

		int fajl = (*it).getFajl();
		string rbsekcije = (*it).getSekcija();
		string simbol = (*it).getSimbol(); //redni broj simbola => nadji ga u tabeli simbola
		list<Simbol> ::iterator it2;
		for (it2 = tabelaSimbola.begin(); it2 != tabelaSimbola.end(); ++it2) {

			//NALAZENJE VREDNOSTI SIMBOLA KOJI SE UBACUJE
			if ((*it2).getFajl() == fajl && (*it2).getRbr() == simbol) {
				imeSimbola = (*it2).getSimbol();
				if ((*it2).getSekcija() == "-1") { //nadji simbol istog imena u nekom drugom fajlu, tu je definisan
					list<Simbol> ::iterator it3;
					for (it3 = tabelaSimbola.begin(); it3 != tabelaSimbola.end(); ++it3) {
						if ((*it3).getFajl() != (*it2).getFajl() && (*it2).getSimbol() == (*it3).getSimbol() && (*it3).getSekcija() != "-1") {
							vrednost = (*it3).getOffset();
						}
					}
				}
				else {
					vrednost = (*it2).getOffset();
				}
				
				
			}

			//NALAZENJE IMENA SEKCIJE U KOJU SE UBACUJE
			if ((*it2).getRbr() == rbsekcije && (*it2).getFajl() == fajl) {
				imeSekcije = (*it2).getSekcija();
			}

		}

		if (vrednost == "") {
			greska = "Simbol " + imeSimbola + " nije definisan ni u jednom fajlu!\n";
			return false;
		}

		razresiRelZapis(fajl, imeSekcije, hex2int("0x"+(*it).getOffset()), string2int(vrednost), (*it).getTip(), (*it).getAddend(), (*it).getEndian());

	}

	return true;
}

void Linker::razresiRelZapis(int fajl, string sekcija, int offset, int vrednost, string tip, string addend, string endian) {

	list<SadrzajSekcije> ::iterator it;
	for (it = tabelaSadrzajaSekcija.begin(); it != tabelaSadrzajaSekcija.end(); ++it) {

		if (fajl == (*it).getFajl() && sekcija == (*it).getSekcija()) {
			int trenOffset = hex2int("0x"+(*it).getOffset());
			string sadrzaj = (*it).getSadrzaj();
			int sledOffset = sledeciOffset(trenOffset, sadrzaj);
			if (offset >= trenOffset && offset < sledOffset) { //to je taj red gde azuriram sadrzaj
				string prviDeo;
				if (sadrzaj.size() == 5) {
					prviDeo = "";
				}
				else { //size=14, a od sadrzaj[9] se menja
					prviDeo = sadrzaj.substr(0, 9);
					sadrzaj = sadrzaj.substr(9);
				}

				if (tip == "R_VN16_16") { //apsolutno

					if (endian == "0") { //big-endian
						string visiBajt = sadrzaj.substr(0, 2);
						string niziBajt = sadrzaj.substr(3);
						sadrzaj = visiBajt + niziBajt;
						int novaVrednost = hex2int("0x"+sadrzaj) + vrednost;
						sadrzaj = int2sadrzaj(novaVrednost);
					}
					else { //little-endian
						string niziBajt = sadrzaj.substr(0, 2);
						string visiBajt = sadrzaj.substr(3);
						sadrzaj = visiBajt + niziBajt;
						int novaVrednost = hex2int("0x"+sadrzaj) + vrednost;
						sadrzaj = int2sadrzaj(novaVrednost);
						niziBajt = sadrzaj.substr(3);
						visiBajt = sadrzaj.substr(0, 2);
						sadrzaj = niziBajt + " " + visiBajt;
					}
					(*it).setSadrzaj(prviDeo + sadrzaj);
				}
				else { //PC relativno
					int adrPoljeOffset;
					//izracunaj adresu adresnog polja
					list<Simbol> ::iterator it2;
					for (it2 = tabelaSimbola.begin(); it2 != tabelaSimbola.end(); ++it2) {
						if (fajl == (*it2).getFajl() && sekcija == (*it2).getSimbol()) {
							adrPoljeOffset = string2int((*it2).getOffset()) + offset;
							break;
						}
					}
					sadrzaj = int2sadrzaj(string2int(addend) + vrednost - adrPoljeOffset);
					(*it).setSadrzaj(prviDeo + sadrzaj);
				}
			}
		}
	}
}

int Linker::sledeciOffset(int trenutniOffset, string trenutniSadrzaj) {
	int brrazmaka = 0;
	int pos;
	while ((pos = trenutniSadrzaj.find(" ")) != string::npos) {
		brrazmaka++;
		trenutniSadrzaj = trenutniSadrzaj.substr(pos+1);
	}

	return trenutniOffset + brrazmaka + 1;
}
void Linker::generisiZapisHEX() {

	//cout << "Broj sekcija: " << brSekcija << endl;

	int staraPocAdr = -1;
	int obradjenoBajtova = 0;
	string red, adresa;
	for (int i = 0; i < brSekcija; i++) {

		//odredi minimalnu adresu
		int min = -1;
		string sekc;
		list<Sekcija> ::iterator it;
		for (it = tabelaSekcija.begin(); it != tabelaSekcija.end(); ++it) {

			if ((*it).getFajl() == 0 && (*it).getIspisana() == false) {
				if (min == -1 || (*it).getPocAdr() < min) {
					min = (*it).getPocAdr();
					sekc = (*it).getIme();
				}
			}
		}

		//ispisi sadrzaj te sekcije
		int pocAdr = min;
		int pocetniVisak = 0;
		while (pocAdr % 8 != 0) {
			pocAdr--;
			pocetniVisak++;
		}

		if (pocAdr != staraPocAdr) {
			if (obradjenoBajtova != 0) {
				while (obradjenoBajtova < 8) {
					obradjenoBajtova++;
					red += "00 ";
				}
				red = red.substr(0, red.size() - 1);
				izlazni_fajl << red << endl;
			}
			obradjenoBajtova = pocetniVisak;
			adresa = int2sadrzaj(pocAdr);
			adresa = adresa.substr(0, 2) + adresa.substr(3);
			red = adresa + ":\t";
			for (int p = 0; p < pocetniVisak; p++) {
				red += "00 ";
			}
		}
		else { //nadovezuju se sekcije
			int razlika = pocetniVisak - obradjenoBajtova;
			for (int p = 0; p < razlika; p++) {
				red += "00 ";
				obradjenoBajtova++;
			}
		}

		
		list<SadrzajSekcije> ::iterator it2;
		for (it2 = tabelaSadrzajaSekcija.begin(); it2 != tabelaSadrzajaSekcija.end(); ++it2) { //obradi sekciju

			if (sekc == (*it2).getSekcija()) { //obradi red

				string sadrz = (*it2).getSadrzaj();
				while (sadrz.length() != 2) { //do pretposlednjeg bajta sadrzaja
					string pom = sadrz.substr(0, 2);
					red += pom + " ";
					obradjenoBajtova++;
					
					if (obradjenoBajtova == 8) { //ispisi red i restartuj promenljive
						red = red.substr(0, red.size() - 1);
						izlazni_fajl << red << endl;
						obradjenoBajtova = 0;
						pocAdr += 8;
						adresa = int2sadrzaj(pocAdr);
						adresa = adresa.substr(0, 2) + adresa.substr(3);
						red = adresa + ":\t";
					}

					sadrz = sadrz.substr(3);	
				}
				//poslednji bajt sadrzaja
				string pom = sadrz.substr(0, 2);
				red += pom + " ";
				obradjenoBajtova++;

				if (obradjenoBajtova == 8) { //ispisi red i restartuj promenljive
					red = red.substr(0, red.size() - 1);
					izlazni_fajl << red << endl;
					obradjenoBajtova = 0;
					pocAdr += 8;
					adresa = int2sadrzaj(pocAdr);
					adresa = adresa.substr(0, 2) + adresa.substr(3);
					red = adresa + ":\t";
				}

			}
			
			
		}
		staraPocAdr = pocAdr;

		//oznaci sekciju kao ispisanu
		list<Sekcija> ::iterator it3;
		for (it3 = tabelaSekcija.begin(); it3 != tabelaSekcija.end(); ++it3) {

			if ((*it3).getFajl()==0 && (*it3).getIme() == sekc) {
				(*it3).setIspisana();
				break;
			}
		}
	}
	
	if (obradjenoBajtova != 0) {
		/*
		while (obradjenoBajtova < 8) {
			obradjenoBajtova++;
			red += "00 ";
		}
		*/
		red = red.substr(0, red.size() - 1);
		izlazni_fajl << red << endl;
	}
	
}

void Linker::rasporediSekcijeLinkable() {

	string sekcija;
	int ukSize = 0;

	list<Simbol> ::iterator it;
	for (it = tabelaSimbola.begin(); it != tabelaSimbola.end(); ++it) {

		if ((*it).getSize()!="-1") {
			sekcija = (*it).getSimbol();
			ukSize = 0;
			if (sekcijaVecUbacena(sekcija)) {
				continue;
			}
			list<Simbol> ::iterator it2;
			for (it2 = tabelaSimbola.begin(); it2 != tabelaSimbola.end(); ++it2) {

				if ((*it2).getSimbol() == sekcija) {
					Sekcija nova((*it2).getFajl(), sekcija, string2int((*it2).getSize()), ukSize, false);
					tabelaSekcija.push_back(nova);
					ukSize += string2int((*it2).getSize());
				}
			}
			Sekcija nova(0, sekcija, ukSize, 0, false);
			tabelaSekcija.push_back(nova);
		}
	}
}

void Linker::azurirajSadrzajeSekcija() {

	list<Sekcija> ::iterator it;
	for (it = tabelaSekcija.begin(); it != tabelaSekcija.end(); ++it) {
		if ((*it).getFajl() == 0) {
			string sekc = (*it).getIme(); //za ovu sekciju prodji kroz sadrzaj i nadovezi sekcije

			list<SadrzajSekcije> ::iterator it2;
			for (it2 = tabelaSadrzajaSekcija.begin(); it2 != tabelaSadrzajaSekcija.end(); ++it2) {

				if ((*it2).getSekcija() == sekc) {
					int stariOffset = hex2int("0x" + (*it2).getOffset());
					int fajl = (*it2).getFajl();

					list<Sekcija> ::iterator it3;
					for (it3 = tabelaSekcija.begin(); it3 != tabelaSekcija.end(); ++it3) {

						if ((*it3).getFajl() == fajl && (*it3).getIme() == sekc) {
							int pocOffset = (*it3).getPocAdr();
							stringstream ss;
							ss << hex << (stariOffset + pocOffset);
							(*it2).setOffset(ss.str());
							(*it2).setFajl(0);
							break;
						}
					}
				}
			}
		}
	}

	//SREDI ABSOLUTE SEKCIJU
	int novOffset = 0;
	list<Simbol> ::iterator itA;
	for (itA = tabelaSimbola.begin(); itA != tabelaSimbola.end(); ++itA) {
		if ((*itA).getSekcija() == "0") { //samo apsolutni
			int fajl = (*itA).getFajl();
			string offset = (*itA).getOffset();

			list<SadrzajSekcije> ::iterator it4;
			for (it4 = tabelaSadrzajaSekcija.begin(); it4 != tabelaSadrzajaSekcija.end(); ++it4) {
				if ((*it4).getSekcija() == "absolute" && fajl == (*it4).getFajl() && offset == (*it4).getOffset()) {
					stringstream ss2;
					ss2 << hex << novOffset;
					(*it4).setOffset(ss2.str());
					(*it4).setFajl(0);
					(*itA).setOffset(int2string(novOffset));

					novOffset += 2;

					break;
				}
			}
		}
	}
	Sekcija abs(0, "absolute", novOffset, 0, false);
	tabelaSekcija.push_back(abs);
	//obrisi iz sadrzaja sve absolute koji nisu u tab simbola
	list<SadrzajSekcije> ::iterator it5;
	for (it5 = tabelaSadrzajaSekcija.begin(); it5 != tabelaSadrzajaSekcija.end();) {
		if ((*it5).getSekcija() == "absolute" && (*it5).getFajl() !=0) {
			it5 = tabelaSadrzajaSekcija.erase(it5);
		}
		else {
			++it5;
		}
	}
}

void Linker::azurirajTabSimbolaIRelZapiseLinkable() {
	//dodaj zajednicke sekcije u tab simbola
	int noviRB = 1;
	list<Sekcija> ::iterator it;
	for (it = tabelaSekcija.begin(); it != tabelaSekcija.end(); ++it) {
		bool nasao = false;
		if ((*it).getFajl() == 0 && (*it).getIme() != "absolute") {
			string sekc = (*it).getIme();
			Simbol novaSekcija(0, sekc, sekc, "0", int2string((*it).getSize()), "100", int2string(noviRB++));
			tabelaSimbola.push_back(novaSekcija);
		}
	}

	list<Simbol> ::iterator it2;
	for (it2 = tabelaSimbola.begin(); it2 != tabelaSimbola.end(); ++it2) {
		if ((*it2).getFajl() != 0 && (*it2).getSize() == "-1" && (*it2).getSekcija() != "-1" && (*it2).getSekcija() != "0") { //definicija simbola, nije absolute
			string ime = (*it2).getSimbol();
			int fajl = (*it2).getFajl();
			string rbsekcije = (*it2).getSekcija();
			string imesekcije = "";
			string rbsekcijelinkera;
			string noviOffset;

			//izvuci ime sekcije
			list<Simbol> ::iterator it3;
			for (it3 = tabelaSimbola.begin(); it3 != tabelaSimbola.end(); ++it3) {
				if (rbsekcije == (*it3).getRbr() && fajl == (*it3).getFajl()) {
					imesekcije = (*it3).getSimbol();
					break;
				}
			}

			//izvuci novi offset
			list<Sekcija> ::iterator it4;
			for (it4 = tabelaSekcija.begin(); it4 != tabelaSekcija.end(); ++it4) {
				if (fajl == (*it4).getFajl() && imesekcije == (*it4).getIme()) {
					int pocadr = (*it4).getPocAdr();
					int starioff = string2int((*it2).getOffset());
					noviOffset = int2string(pocadr + starioff);
					break;
				}
			}

			//izvuci rb sekcije linkera
			list<Simbol> ::iterator it5;
			for (it5 = tabelaSimbola.begin(); it5 != tabelaSimbola.end(); ++it5) {
				if ((*it5).getFajl() == 0 && (*it5).getSimbol() == imesekcije) {
					rbsekcijelinkera = (*it5).getRbr();
					break;
				}
			}

			//dodaj simbol linkera
			Simbol novi(0, ime, rbsekcijelinkera, noviOffset, "-1", "010", int2string(noviRB++));
			tabelaSimbola.push_back(novi);
		}
		if ((*it2).getFajl() != 0 && (*it2).getSekcija() == "0") { //absolute simbol
			Simbol novi(0, (*it2).getSimbol(), "0", (*it2).getOffset(), "-1", "010",  int2string(noviRB++));
			tabelaSimbola.push_back(novi);
		}

	}
	
	azurirajRelokZapise();

	//obrisi simbole koji nisu simboli linkera i dodaj und i abs sekcije
	list<Simbol> ::iterator it6;
	for (it6 = tabelaSimbola.begin(); it6 != tabelaSimbola.end();) {
		if ((*it6).getFajl() != 0) {
			it6 = tabelaSimbola.erase(it6);
		}
		else {
			++it6;
		}
	}

	list<Sekcija> ::iterator it7;
	for (it7 = tabelaSekcija.begin(); it7 != tabelaSekcija.end(); ++it7) {
		if ((*it7).getIme() == "absolute") {
			Simbol abs(0, "absolute", "absolute", "0",  int2string((*it7).getSize()), "100", "0");
			tabelaSimbola.push_front(abs);
			break;
		}
	}
	
	Simbol und(0,"undefined", "undefined", "0",  "0", "100", "-1");
	tabelaSimbola.push_front(und);
	
}

void Linker::azurirajRelokZapise() {
	
	list<RelokZapis> ::iterator it;
	for (it = tabelaRelZap.begin(); it != tabelaRelZap.end(); ++it) {
		int fajl = (*it).getFajl();
		string rbsimbola = (*it).getSimbol();
		string rbsekcije = (*it).getSekcija();
		string offset = (*it).getOffset();
		string imesekcije,imesimbola;
		string novOffset;
		string novrbsekcije, novrbsimbola;

		//nadji ime sekcije i ime simbola
		list<Simbol> ::iterator it2;
		for (it2 = tabelaSimbola.begin(); it2 != tabelaSimbola.end(); ++it2) {
			if (fajl == (*it2).getFajl() && rbsekcije == (*it2).getRbr()) {
				imesekcije = (*it2).getSimbol();
				
			}
			if (fajl == (*it2).getFajl() && rbsimbola == (*it2).getRbr()) {
				imesimbola = (*it2).getSimbol();

			}
		}

		//nadji pocetnu adresu sekcije pa izracunaj nov offset
		list<Sekcija> ::iterator it3;
		for (it3 = tabelaSekcija.begin(); it3 != tabelaSekcija.end(); ++it3) {
			if (fajl == (*it3).getFajl() && imesekcije == (*it3).getIme()) {
				int pocadr = (*it3).getPocAdr();
				int stariOffset = hex2int("0x"+(*it).getOffset());
				stringstream ss;
				ss << hex << (pocadr + stariOffset);
				ss >> novOffset;
				break;
			}
		}

		//nadji nov rb simbola i nov rb sekcije
		list<Simbol> ::iterator it4;
		for (it4 = tabelaSimbola.begin(); it4 != tabelaSimbola.end(); ++it4) {
			if ((*it4).getFajl() == 0 && (*it4).getSimbol() == imesekcije) {
				novrbsekcije = (*it4).getRbr();
			}
			if ((*it4).getFajl() == 0 && (*it4).getSimbol() == imesimbola) {
				novrbsimbola = (*it4).getRbr();
			}
		}

		(*it).setFajl(0);
		(*it).setSekcija(novrbsekcije);
		(*it).setOffset(novOffset);
		(*it).setSimbol(novrbsimbola);

		//AKO JE LOKALNI SIMBOL, MORA SE PROMENITI VREDNOST NA:
		// - MESTU KORISCENJA (ABS), - ADDEND (PC_REL)
		
		list<Simbol> ::iterator it5;
		for (it5 = tabelaSimbola.begin(); it5 != tabelaSimbola.end(); ++it5) {

			if ((*it5).getFajl()==0 && novrbsimbola == (*it5).getRbr()) {
				if ((*it5).getSimbol() == (*it5).getSekcija()) { //za simbol je poslat redni broj sekcije, tako da je simbol lokalan
					int pomeraj=0;
					list<Sekcija> ::iterator it6;
					for (it6 = tabelaSekcija.begin(); it6 != tabelaSekcija.end(); ++it6) {
						if ((*it6).getFajl() == fajl && (*it6).getIme() == imesimbola) {
							pomeraj = (*it6).getPocAdr();
							break;
						}
					}

					if ((*it).getTip() == "R_VN16_16") { //abs
						
						list<SadrzajSekcije> ::iterator it7;
						for (it7 = tabelaSadrzajaSekcija.begin(); it7 != tabelaSadrzajaSekcija.end(); ++it7) {
							if (imesekcije == (*it7).getSekcija()) {
								int os = hex2int("0x" + novOffset);
								int trenOffset = hex2int("0x" + (*it7).getOffset());
								string sadrzaj = (*it7).getSadrzaj();
								int sledOffset = sledeciOffset(trenOffset, sadrzaj);
								if (os >= trenOffset && os < sledOffset) { //to je taj red gde azuriram sadrzaj

									string prviDeo;
									if (sadrzaj.size() == 5) {
										prviDeo = "";
									}
									else { //size=14, a od sadrzaj[9] se menja
										prviDeo = sadrzaj.substr(0, 9);
										sadrzaj = sadrzaj.substr(9);
									}


									if ((*it).getEndian() == "0") { //big-endian
											string visiBajt = sadrzaj.substr(0, 2);
											string niziBajt = sadrzaj.substr(3);
											sadrzaj = visiBajt + niziBajt;
											int novaVrednost = hex2int("0x" + sadrzaj) + pomeraj;
											sadrzaj = int2sadrzaj(novaVrednost);
									}
									else { //little-endian
											string niziBajt = sadrzaj.substr(0, 2);
											string visiBajt = sadrzaj.substr(3);
											sadrzaj = visiBajt + niziBajt;
											int novaVrednost = hex2int("0x" + sadrzaj) + pomeraj;
											sadrzaj = int2sadrzaj(novaVrednost);
											niziBajt = sadrzaj.substr(3);
											visiBajt = sadrzaj.substr(0, 2);
											sadrzaj = niziBajt + " " + visiBajt;
									}
									(*it7).setSadrzaj(prviDeo + sadrzaj);
									
								}
							}
						}
						
					}
					else { //pc_rel
						int addend = string2int((*it).getAddend());
						addend += pomeraj;
						(*it).setAddend(int2string(addend));
					}
				}
			}

		}
		
		
	}
	
}

void Linker::ispisiTabele()
{
	izlazni_fajl << "TABELA SIMBOLA:" << endl << "Ime\t\tSekcija\t\tOffset\tLGE\tSize\tRBR" << endl <<
		"-------------------------------------------------------------" << endl;

	list <Simbol> ::iterator it;
	for (it = tabelaSimbola.begin(); it != tabelaSimbola.end(); ++it) {
		izlazni_fajl << *it;
	}


	izlazni_fajl << "\nTABELE SEKCIJA:" << endl;
	list <Simbol> ::iterator itSimb;
	list <SadrzajSekcije> ::iterator itSek;
	for (itSimb = tabelaSimbola.begin(); itSimb != tabelaSimbola.end(); ++itSimb) {

		if ((*itSimb).getSize() != "-1" && (*itSimb).getSimbol() != "undefined") { //sekcija je

			izlazni_fajl << " --- SEKCIJA: " << (*itSimb).getSimbol() << " ---" << endl;
			izlazni_fajl << "Offset\tSadrzaj" << endl;

			for (itSek = tabelaSadrzajaSekcija.begin(); itSek != tabelaSadrzajaSekcija.end(); ++itSek) {

				if ((*itSek).getSekcija() == (*itSimb).getSimbol()) {
					izlazni_fajl << (*itSek);
				}
			}

			izlazni_fajl << "\n";
		}
	}


	izlazni_fajl << "\nRELOKACIONI ZAPISI:" << endl;
	izlazni_fajl << "Sekcija\tOffset\tTip relokacije\tSimbol\tAddend\tEndian\n------------------------------------------------\n";
	list <RelokZapis> ::iterator it2;
	for (it2 = tabelaRelZap.begin(); it2 != tabelaRelZap.end(); ++it2) {
		izlazni_fajl << (*it2);
	}
}

void Linker::ispisiPomocneTabele() {
	izlazni_fajl << "\n\nTABELA SEKCIJA:" << endl;
	izlazni_fajl << "Fajl\tIme\tSize\tPoc adr\tPlace\n------------------------------------------------\n";
	list <Sekcija> ::iterator it;
	for (it = tabelaSekcija.begin(); it != tabelaSekcija.end(); ++it) {
		izlazni_fajl << (*it);
	}
}

int Linker::hex2int(string h)
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

int Linker::string2int(string s)
{
	int ret;
	stringstream ss(s);
	ss >> ret;
	return ret;
}

string Linker::int2string(int i)
{
	stringstream ss;
	ss << i;
	return ss.str();
}

string Linker::int2sadrzaj(int br) {
	stringstream ss;
	ss << hex << br;
	string sadrzaj = ss.str();
	string pom = "";
	int duzina = sadrzaj.length();
	if (duzina > 4) { //odseci vise bite
		sadrzaj = sadrzaj.substr(duzina - 4);
	}
	else if (duzina < 4) {
		for (int i = 0; i < 4 - duzina; i++) {
			pom += "0";
		}
		pom += sadrzaj;
		sadrzaj = pom;
	}

	string visiBajt = sadrzaj.substr(0, 2);
	string niziBajt = sadrzaj.substr(2);
	sadrzaj = visiBajt + " " + niziBajt;
	return sadrzaj;
}