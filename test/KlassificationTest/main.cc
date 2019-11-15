#include <iostream>
#include <fstream>
#include <math.h>
#include <queue>

struct IntersecPlacement {
	int type;
	float pos;
};

using namespace std;
int main(void) {
	queue<IntersecPlacement> que; // Queue of past states

	//Input
	ifstream file;
	file.open("../generatedKlassFiles/generatedPositionType1.txt");


	float posSum = 0;
	int typeSum = 0;
	float posAvg = 0;
	float typeAvg = 0;

	if (file.is_open()) {
		while (!file.eof()) {
			
			IntersecPlacement temp;
			
			

			file >> temp.type >> temp.pos;
			que.push(temp);

			posSum = posSum + temp.pos;
			typeSum = typeSum + temp.type;

			cout << typeSum << "\n";

			posAvg = posSum/que.size();
			typeAvg = typeSum/que.size();
			
			cout << typeAvg << " " << posAvg << "\n";

			if(que.size()==10){
				temp = que.front();
				que.pop();
				posSum -=  temp.pos;
				typeSum -=  temp.type;
			}
		}
	}
	file.close();
	return 0;
}


