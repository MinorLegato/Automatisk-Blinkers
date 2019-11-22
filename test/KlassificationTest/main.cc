#include <iostream>
#include <fstream>
#include <math.h>
#include <queue>
#include <vector>


struct IntersecPlacement {
	int type;
	float pos;
};

struct KlassList {
	static const int maxSize = 10;
	static const int typeTypes = 10;
	IntersecPlacement list[maxSize];
	int index = 0;
	int size = 0;
	float pos = 0;
	int type = 0;
	int typeCheck[typeTypes];
	float posSum = 0;

	void push(IntersecPlacement newInput) {
		//at size 10
		if (size == maxSize) {

			posSum -= list[index].pos;
			if (typeCheck[list[index].type] > 0) {
				typeCheck[list[index].type]--;
			}

			list[index] = newInput;
			posSum += list[index].pos;

			typeCheck[list[index].type]++;
			if (typeCheck[list[index].type] == 8) {
				type = typeCheck[list[index].type];
			}
			index = (++index) % maxSize;
		}

		// at size 9
		else if (size == (maxSize-1)) {

			list[index] = newInput;
			posSum += newInput.pos;
			typeCheck[newInput.type]++;

			for (IntersecPlacement temp : list) {
				posSum = posSum + temp.pos;
				typeCheck[temp.type]++;
				if (typeCheck[temp.type] == 8) {
					type = temp.type;
				}
			}
			pos = posSum / maxSize;
			index++;
			size++;
		}
		//size below 9
		else {


			list[index] = newInput;
			posSum += newInput.pos;
			typeCheck[newInput.type]++;

			index++;
			size++;
		}
	}







};


#if 0

// 0 = no blink// 1 = right blink // -1 = left blink
int Klass(std::vector<IntersecPlacement> & que) {


	//Determine true position and intersection type
	float posSum = 0;
	float posAvg = 0;
	int type
		int typeCheck[7];
	for (IntersecPlacement temp : que) {
		posSum = posSum + temp.pos;
		if (++typeCheck[temp.type] == (int)que.size() / 0.8) {
			type = temp.type;
		}
	}
	posAvg = posSum / que.size();



	/* Type       nr
	*	no way		     0
	*	single file way	 1
	*	up-left          3
	*	up-right         5
	*	left-right       6
	*   up-left-right    7
	*   two-files        9
	*/
	switch (type) {

		case 0;
			return 0;
		case 1:

		case 3:


		case 4:
		case 5:
		case 6:
			if (posAvg > 0) return 1;
			else return -1;
		case 7:
			if (posAvg > 0.7) return 1;
			else if (posAvg < -0.7) return -1;
			else return

		case 9:
	}
}

#endif


using namespace std;

int main(void) {
	int type;
	float pos;
	ifstream in("generatedPositionType1.txt");
	if (in.is_open())
	{
		while( getline(in,a) ){


		}

		in.close();

	}
	
	in.getline()

	return 0;
}




