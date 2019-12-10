// ============================================ KLASSIFICATION ============================================== //

/*Pos
*    -1 -> 1//left -> right
*/
/*      Type             #
     ROAD_NONE       = (0),
     ROAD_UP         = (1 << 0),
     ROAD_LEFT       = (1 << 1),
     ROAD_RIGHT      = (1 << 2),
     ROAD_TWO_LANES  = (1 << 3),
	*/

struct InterPos {
	int type;
	float pos;
};

struct InterPosList {
	static const int maxSize = 10;
	static const int typeTypes = 10;
	static const int difListSize = (int)maxSize * 0.5;
	static const int typeThreshold = (int)maxSize * 0.8;

	InterPos list[maxSize];
	int index = 0;
	int size = 0;

	//position
	float pos = 0;
	float posSum = 0;

	//type of intersection/road scenario
	int type = 0;
	int typeCheck[typeTypes];
	
	//Difference in pos
	float posPrev = 0;
	float difList[difListSize];
	int difListIndex = 0;
	float posDifAvg = 0;

	//-1 left blink//1 right blink//0 no blink
	int blink = 0;

	void push(InterPos newInput) {
		//at size 10
		if (size == maxSize) {

			if (typeCheck[list[index].type] > 0) {
				typeCheck[list[index].type]--;
			}

			list[index] = newInput;
			posSum += list[index].pos;

			typeCheck[list[index].type]++;
			if (typeCheck[list[index].type] == typeThreshold) {
				type = typeCheck[list[index].type];
			}

			posSum = 0;
			for(InterPos a : list){
				posSum += a.pos;
			}
			pos = posSum / maxSize;


			//-----pos difference-----///
			difList[difListIndex] = newInput.pos - posPrev;
			float posDifSum = 0;
			
			for (float dif : difList) {
				posDifSum += dif;
			}
			posDifAvg = posDifSum / difListSize;

			posPrev = list[index].pos;
			difListIndex = (++difListIndex) % difListSize;
			///-----------------------////

			index = (++index) % maxSize;
		}

		// at size 9
		else if (size == (maxSize-1)) {

			list[index] = newInput;
			typeCheck[newInput.type]++;

			for (InterPos temp : list) {
				if (typeCheck[temp.type] >= typeThreshold) {
					type = temp.type;
				}
			}


			difList[difListIndex] = newInput.pos - posPrev;
			float posDifSum = 0;
			for (float dif : difList) {
				posDifSum += dif;
			}
			posDifAvg = posDifSum / difListSize;

			posPrev = list[index].pos;

			index = (++index) % maxSize;
			difListIndex = (++difListIndex) % difListSize;
			size++;
		}
		//size below 9
		else {
			list[index] = newInput;
			typeCheck[newInput.type]++;

			if (size != 0) {
				difList[difListIndex] = newInput.pos - posPrev;
				difListIndex = (++difListIndex) % difListSize;
			}
			if (size >= difListSize) {
				float posDifSum = 0;
				for (float dif : difList) {
					posDifSum += dif;
				}
				posDifAvg = posDifSum / difListSize;
			}
			
			posPrev = list[index].pos;
			index++;
			size++;
		}
	}

	int analyze() {
		switch (type) {
		case 3:
			leftUp();
		case 4:

		case 5:
			rightUp();
		case 6:
			leftRight();
		case 7:
			leftRightUp();
		
        case 8:
            twoFiles();
        }
		
		return blink;
	}

	/*
		Threeway intersecion
	*/
	void leftRight() {
		if (pos > 0) blink = 1;
		else blink = -1;
	}

	void leftUp() {
		if (pos < 0) blink = -1;
	}

	void rightUp() {
		if (pos < 0) blink = 1;
	}


	//4-wayintersection
	void leftRightUp() {
		if (pos > 0.5) blink = 1;
		else if (pos < -0.5)  blink = -1;
	}

	//File change
	void twoFiles() {
		if (posDifAvg > -0.03) blink = -1;
		else if (posDifAvg > 0.03) blink = 1;
	}
};
