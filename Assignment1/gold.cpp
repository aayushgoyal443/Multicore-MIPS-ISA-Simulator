// This is the corresponding code in C++ language and this will be used to produce the gold outputs.

#include <iostream>
using namespace std;

int main(){
    int sum=0;
    int n;
    cin >>n;
    int x0,y0,x1,y1;
    cin >> x0 >> y0;
    n = n-1;
    while(n!=0){
        cin >> x1>>y1;
        sum+= (y1+y0)*(x1-x0);
        x0=x1;
        y0=y1;
        n-=1;
    }
    float output= sum;
    output/=2;
    cout <<output<<endl;
}