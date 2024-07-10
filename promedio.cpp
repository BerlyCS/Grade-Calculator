#include <iostream>
#include <string>

using namespace std;

int main(int argc, char* argv[]) {
    float args[12];

    for (int i=0; i<12; i++) {
        args[i] = stof(argv[i]);
    }

    int zero_count = 0;
    
    (nc1 > 0) ?  : zero_count++;
    (ne1 > 0) ?  : zero_count++;
    (nc2 > 0) ?  : zero_count++;
    (ne2 > 0) ?  : zero_count++;
    (nc3 > 0) ?  : zero_count++;
    (ne3 > 0) ?  : zero_count++;

    cout<<"Notas con 0: "<<zero_count<<endl;

    
    
}
