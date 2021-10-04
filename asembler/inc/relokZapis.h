#pragma once

#include<string>
using namespace std;

class RelokZapis {
private:

	//gde se krpi
	string sekcija;
	string offset;

	//kako
	int tip; //0-ABS, 1-PC_REL

	//cime
	int simb;

	//dodatno
	int addend;
	int endian; //1-little, 0-big

public:

	RelokZapis(string sek, string o, int t, int sim, int a, int e) : sekcija(sek), offset(o), tip(t), simb(sim), addend(a), endian(e) {}

	friend ostream& operator<<(ostream& it, const RelokZapis& rk) {
		string stip;
		if (rk.tip == 0) {
			stip = "R_VN16_16";
		}
		else {
			stip = "R_VN16_PC16";
		}

		return it << rk.sekcija << "\t" << rk.offset << "\t" << stip << "\t" << rk.simb << "\t" << rk.addend << "\t"<<rk.endian<<"\n";
	}
};