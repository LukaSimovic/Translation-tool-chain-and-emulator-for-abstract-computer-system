#pragma once

#include<regex>
using namespace std;

//pomocni stringovi
string literalDec = "-?[0-9]+";
string literalHex = "0x[0-9A-Fa-f]+";
string simbol = "[a-zA-Z][a-zA-Z0-9_]*";
string litIliSimb = literalDec + "|" + literalHex + "|" + simbol;
string literal = literalDec + "|" + literalHex;
string registar = "r[0-7]|psw";

//regexi za simb i lit
regex literalR{ "^" + literal + "$" };
regex literalDecR{ "^" + literalDec + "$" };
regex literalHexR{ "^" + literalHex + "$" };

//regexi za direktive
regex direktivaEnd{ "\\.end$" };
regex direktivaSection{ "^\\.section " + simbol + "$" };
regex direktivaSkip{ "^\\.skip (" + literal + ")$" };
regex direktivaGlobal{ "^\\.global " + simbol + "(," + simbol + ")*$" };
regex direktivaExtern{ "^\\.extern " + simbol + "(," + simbol + ")*$" };
regex direktivaWord{ "^\\.word (" + litIliSimb + ")(,(" + litIliSimb + "))*$" };
regex direktivaEqu{ "^\\.equ " + simbol + ",(" + literal + ")$" };

//regexi za labelu
regex labela{ "^" + simbol + ":$" };
regex labelaIkomanda{ "^" + simbol + ":(.*)$" };

//regexi za instrukcije
regex insBezOper{"^(halt|ret|iret)$"};
regex ins1Reg{ "^(int|not|push|pop) (r[0-7]|psw)$" };
regex ins2Reg{ "^(add|sub|mul|div|cmp|and|or|xor|test|shr|shl|xchg) (r[0-7]|psw),(r[0-7]|psw)$" };
regex ins1Op{ "^(jmp|jeq|jne|jgt|call) (.*)$" };
regex ins1Reg1Op{ "^(ldr|str) (r[0-7]|psw),(.*)$" };

//regexi za nacine adresiranja
regex skokAps{ "^(" + litIliSimb + ")$" };
regex skokPcRel{ "^%(" + simbol + ")$" };
regex skokMemDir{ "^\\*(" + litIliSimb + ")$" };
regex skokRegDir{ "^\\*(" + registar + ")$" };
regex skokRegInd{ "^\\*\\[(" + registar + ")\\]$" };
regex skokRegIndDisp{ "^\\*\\[(" + registar + ")\\+("+ litIliSimb +")\\]$" };
regex dataAps{ "^\\$(" + litIliSimb + ")$" };
regex dataPcRel{ "^%(" + simbol + ")$" };
regex dataMemDir{ "^(" + litIliSimb + ")$" };
regex dataRegDir{ "^(" + registar + ")$" };
regex dataRegInd{ "^\\[(" + registar + ")\\]$" };
regex dataRegIndDisp{ "^\\[(" + registar + ")\\+(" + litIliSimb + ")\\]$" };