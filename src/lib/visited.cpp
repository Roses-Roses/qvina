#include "visited.h"

double ele::dist2(std::vector<double> now){
	double out=0;
	for (int i=0; i < size(); i++){
		double d = x[i] - now[i];
		out += d * d;
	}
	return out;
}
double ele::dist2_3D(std::vector<double> now){
	double d, out=0;
	for (int i=0;i<3;i++){
		d = x[i] - now[i];
		out += d * d;
	}
	return out;
}
//static bool comp(std::pair<int, double> p, std::pair<int, double> q)
//{
//	return (p.second<q.second);
//}

/** differences:
 * 1- I check also the value of f
 * 2- point is considered interesting if only half of number of variables contains stationary points
*/
bool ele::check(std::vector<double> now_x, double now_f, std::vector<double> now_d)
{
	bool out=false;
//	int counter=0;//no need currently
	bool newXBigger, newYBigger;
	const long long ONE=0x0000000000000001;
	long long bitMask;
	newYBigger=(now_f - f) > 0;
//	bool effect=false;

	int nowDSize = now_d.size();
	for (int i = 0; i < nowDSize; i++) {
		bitMask = ONE << i;

		if((d_zero & bitMask) || !(now_d[i])){//if any of them is zero
//			counter++;//no need currently
			continue;
		}else{
			const bool nowPositive= now_d[i] > 0;
			const bool dPositive= d_positive & bitMask;
			if (nowPositive ^ dPositive) {//if both derivatives have different signs
//				counter++;//no need currently
				continue;
			}
			else {
				newXBigger=(now_x[i]-x[i]) > 0;
//				if (nowPositive? (newXBigger ^ newYBigger): (!(newXBigger ^ newYBigger))) {//if the higher x have lower f (if both ascending), or vice versa
				if (! (nowPositive ^ newXBigger ^ newYBigger)) {//if the higher x have lower f (if both ascending), or vice versa
//					counter++;//no need currently
//					effect=true;
				}
				//TODO this else is valid only in case the least accepted number is
				//ALLLLLL the variables (to be removed if we want to relax the check later on)
				else{
					return false;
				}
			}
		}
//		if (counter >= (now_d.size())) {
//			return true;
//		}
	}
	return true; //just return true, no need for check (to be removed if we want to relax the check later on)

//	out=((((d_positive^now_positive)|d_zero)|now_zero)==getMask());
////	printf("mask=%ld\td_p=%ld\td_z=%ld\tn_p=%ld\tn_z=%ld\n",getMask(),d_positive,d_zero,now_positive,now_zero);
////	printf("check=%s\n",temp?"true":"false");
////	getchar();
////	}
////	return out;

//	return false;
}

linearvisited* linearvisited::instance = NULL;

linearvisited* linearvisited::getInstance(){
	if (linearvisited::instance ==NULL) {
		static linearvisited self;
		linearvisited::instance= & self;
		std::cout << "lazy initialization done"<< std::endl;
	}
	return instance;
}

/**
 * returns minus number of checks dne if interesting (found at least one point with accepted condition),
 * or number of done checks otherwise (if nothing is found).
 * success is always a negative number, while failure can be >= 0.
 */
int linearvisited::interesting(conf x, double f, change g, int excluded) {
	//n.b. excluded is not used. it is here only for homology with the other function

	int len=list.size();

	std::vector<double> conf_v;
	x.getV(conf_v);
	std::vector<double> change_v;
	g.getV(change_v);
	double dist[len];
	bool maybeChecked[len];
//	memset(maybeChecked,false,sizeof(maybeChecked));

	//fill dist[] with distances from conf
	int count=0;
	ReadLock r_lock(myLock);


	for (int i=0;i<len;i++){
		dist[i]=this->list[i].dist2_3D(conf_v);
		maybeChecked[i]= dist[i] <= 25.0;// (cutoff = 5 )^2
		count++;
	}
//	}
//	r_lock.unlock();

	bool flag=false;
	double min=1e10;
	int p=0;
//	const int maxCheck = get_maxCheck();

//	r_lock.lock();
	const int grandMaxCheck= 1 * conf_v.size();
	const int maxCheck= (count<= grandMaxCheck)? count:grandMaxCheck;

	int i = 0;
	for ( ; i < maxCheck; i++){
		min=1e10;
		for (int j=0;j<len;j++){
			if (maybeChecked[j] && (dist[j]<min)){
				p=j;
				min=dist[j];
			}
		}
		maybeChecked[p]=false;
		if (this->list[p].check(conf_v, f, change_v))
			return -i; //i.e. return success
	}
	r_lock.unlock();
	return i;
}

int circularvisited::interesting(conf x, double f, change g, int excluded){

//	printf("%d   %d\n", get_maxCheck(), get_maxSize());

	int len=size();
	if (len==0){
		return -1000000; //i.e. interesting
	}
	else{
//		if (len<2*n_variable)
		if (!full){
//			printf("len==%d<%d\n",len,10*n_variable);
			return -1000000; //i.e. interesting
		}

		std::vector<double> conf_v;
		x.getV(conf_v);
		std::vector<double> change_v;
		g.getV(change_v);
		double dist[len];
		bool notPicked[len];

		memset(notPicked,true,sizeof(notPicked));
		//fill dist[] with distances from conf
		for (int i=0;i<len;i++){
			dist[i]=this->get(i).dist2(conf_v);
		}

//		bool flag=false;
		double min=1e10;
		int p=0;
		const int maxCheck = get_maxCheck()-excluded;
		int i = 0;
		for ( ; i < maxCheck; i++){
			min=1e10;
			for (int j=0;j<len;j++){
				if (notPicked[j] && (dist[j]<min)){
					p=j;
					min=dist[j];
				}
			}
			notPicked[p]=false;
			if (this->get(p).check(conf_v, f, change_v))
				return -i; //i.e. interesting
		}
		return i;//i.e. not interesting after checking i points
	}
}
