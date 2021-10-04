#include<stdio.h>
#include<iostream>
#include"../inc/linker.h"
#include<list>
#include<regex>

using namespace std;

int main(int argc, char* argv[]) {

    bool find_hex=false, find_linkable=false;
    bool sledeceImeIzlaza=false;
    regex place{"^-place=([a-zA-Z_][a-zA-Z_0-9]*)@(0[xX][0-9a-fA-F]+)$"};

	string argv0(argv[0]);
    string izlaz;

    int poslkosa=argv0.find_last_of("/\\");
    string naziv = argv0.substr(poslkosa+1);

    list <string> ulazniFajlovi;
    list <string> adreseSekcija;

    int modLinkera=-1;

   
	if (naziv=="linker") {
		for(int i=1; i<argc; i++){
            string pom(argv[i]);
            if(sledeceImeIzlaza==true){
                izlaz=pom;
                sledeceImeIzlaza=false;
            }
            else if(pom=="-hex"){
                find_hex=true;
            }
            else if(pom=="-linkable"){
                find_linkable=true;
            }
            else if(pom=="-o"){
                sledeceImeIzlaza=true;
            }
            else if(regex_match(pom,place)){
                adreseSekcija.push_back(pom);
            }
            else{
                ulazniFajlovi.push_back(pom);
            }
        }
        if((find_linkable && find_hex) || (!find_hex && !find_linkable)){
            cout<<"Prilikom pokretanja linkera, obavezno je navodjenje tacno jedne od dve opcije -hex i -linkable!"<<endl;
            return 0;
        }
        else if(find_hex){
            modLinkera = 0;
        }
        else {
            modLinkera = 1;
        }
        Linker ld(modLinkera, ulazniFajlovi, izlaz, adreseSekcija);
        ld.pokreni();
        ld.zatvoriIzlazniFajl();
	}
	else {
		cout << "Neuspesno zadata komanda za pokretanje linkera!" << endl;
	}

	return 0;

}
