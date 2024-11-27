#include <iomanip>
#include <iostream>
#include <string>

using namespace std;

bool no_es_numero(string str) {
    for (char i: str) {
        if ( '0' < i || '9' > i || '.' != i ) {
            return false;
        } 
    }
    return true;
}

bool tratar_argv(int argc, char* argv[], float notas[6], float pesos[6]) {
    if (argc < 13) {
        cout<<"Argumentos insuficientes\n";
        return false;
    }

    for (int i=1; i<argc; i++) {
        if (no_es_numero(string(argv[i]))) {
            cout<<"Argumento no numerico"<<endl;
            return false;
        }
    }

    for (int i=0; i<6; i++) {
        /* cout<<argv[i+1]<<' '<<argv[i+7]<<endl; */
        notas[i] = stof(argv[i+1]);
        pesos[i] = stof(argv[i+7]);
    }

    float test=0;
    for (int i=0;i<6;i++) {
        test+=pesos[i];
    }
    if (test != (1.f)) {
        cout<<"W: Los pesos no suman 1.00"<<endl;
    }
    return true;
}

void procesar(float &count, float &nota_pond, float &peso_falta, bool zero_count[], float notas[], float pesos[]) {
    for (int i=0; i<6; i++) {
        if (notas[i] < 1) {
            zero_count[i] = true;
        } else {
            zero_count[i] = false;
        }
    }

    for (int i=0;i<6;i++) {
        if ( !zero_count[i] ) {
            nota_pond += notas[i]*pesos[i];
        } else {
            peso_falta+=pesos[i];
            count++;
        }
    }
}

int main(int argc, char* argv[]) {

    //first
    float notas[6];
    float pesos[6];

    if (!tratar_argv(argc, argv, notas, pesos)) {
        return 1;
    }

    bool zero_count[6];
    float count=0, nota_pond=0, peso_falta=0;

    procesar(count, nota_pond, peso_falta, zero_count, notas, pesos);

    /* cout<<peso_falta<<endl; */
    cout<<"Nota acumulada (actual): "<<nota_pond<<endl;

    //Usando  
    cout<<"Tendrias que sacar: "<<((10.5f-nota_pond)/peso_falta)<<" para aprobar"<<endl;

    /* int opc; */
    /* while (true) { */
    /*     cout<<"Modificar notas " */
    /* } */
}
