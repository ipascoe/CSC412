#include <stdio.h>
#include <stdlib.h>

int gcd(int a, int b);

int main(int argc, char *argv[]) {
    int i = atoi(argv[1]);
    if(argc<2) {
        printf("prog launched with no argument.\nProperusage:  prog m [n]\n");
    } else if(argc>3) {
        printf("prog launched with too many arguments.\nProperusage: prog m [n]\n");
    } else if(argc == 3) {
        if(i<0 || atoi(argv[2])<0){
          printf("The arguments must be strictly positive integers");
        } else {
        int result = gcd(i,atoi(argv[2]));
        printf("The gcd of %d and %d is %d.\n",i,atoi(argv[2]),result);
        }
    } else {
        if(i<0){
            printf("The arguments must be strictly positive integers");
        } else {
            printf("The list of divisors of %d is: ",i);
            for(int k=i; k>0; k--){
                if((i%k)==0){
                    int divisor = i/k;
                    if(k>1){
                        printf("%d, ", divisor);
                    } else {
                        printf("%d.\n", divisor);
                    }
                }
            }
        }
    } return 0;
}

//I use Euclid's recursive gcd method here
int gcd(int a, int b){
    if(a==b) {
        return a;
    } else if (a>b) {
        return gcd(a-b,b);
    } else {
        return gcd(a,b-a);
    }
}