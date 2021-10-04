#include<stdio.h>
#include<iostream>
#include"../inc/asembler.h"
using namespace std;

int main(int argc, char* argv[]) {

	string argv0(argv[0]);
	string argv1(argv[1]);
	string argv2(argv[2]);
	string argv3(argv[3]);

	int poslkosa=argv0.find_last_of("/\\");
	string naziv = argv0.substr(poslkosa+1);
	
	if (naziv=="asembler" && argv1=="-o") {
		Asembler asembler(argv3, argv2);
		asembler.pokreni();
		asembler.zatvoriFajlove();
	}
	else {
		cout << "Neuspesno zadata komanda za pokretanje asemblera!" << endl;
	}

	return 0;
    
}
