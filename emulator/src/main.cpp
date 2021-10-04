#include<iostream>
#include"../inc/emulator.h"
int main(int argc, char* argv[]) {

	string argv0(argv[0]);
	string argv1(argv[1]);
	
	int poslkosa=argv0.find_last_of("/\\");
	string naziv = argv0.substr(poslkosa+1);

	if (argc==2 && naziv == "emulator") {
		Emulator em(argv1);
		em.pokreni();
	}
	else {
		cout << "Neuspesno zadata komanda za pokretanje emulatora!" << endl;
	}

	return 0;
}
