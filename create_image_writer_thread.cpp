#include<iostream>
#include<fstream>
#include<string>
#include<sstream>
#include<stdlib.h>
#include<stack>
#include<map>
#include<vector>
#include<mutex>
#include<thread>
#include<iomanip>

using namespace std;

#define NB_ELEMENTS 5
#define FL "test_big"
#define LN_NUMBER 17321947
#define NB_THREAD 32
#define MOVEMENT 0.25

struct line {
	double x, y, z;
	double lum;
	string label;
};
double abs2(double a){
	if(a<0)
		return -a;
	else 
		return a;
}
line data[LN_NUMBER];
mutex write_mutex, read_mutex;
vector<thread> workers;
ofstream points_file;
ofstream labels_file;

class BigCube{
	public:
	double bx, by, bz;
	BigCube(double a,double  b,double c)
	{
		this->bx = a;
		this->by = b;
		this->bz = c;
		//read_mutex.lock();
		//cout << "" << a << " " << b << " " << c << endl;
		//read_mutex.unlock();
	}
	stack<line> points;
	void insert(line a)
	{
	//			cout<<"Cube "<<bx<<" - "<< by <<" - "<<bz<<endl;
	//	cout<<"Point"<<a.x<<" - "<<a.y<<" - "<<a.z<<endl<<endl;
		if(a.x>= bx && a.x <= (bx + 1.0) && a.y>=by && a.y<= (by+1.0) && a.z>= bz && a.z<= ((bz+1)))
		{
			//cout<<"found one"<<endl;
			points.push(a);
		}
	}
	void classify()
	{	
		if(points.size()==0)
		{
			return;//cout <<"size"<<points.size()<<endl;
		}
		double x,y,z;
		x=bx;
		y=by;
		z=bz;
		double avg;
		int total;
		stack<line> tmp_stk;
		line top_line;

		//labels
		vector<string> labels;
		vector<string> global_labels;
		int max;
		int cx=0;
		int cy = 0;
		int cz = 0;
		double inc = 0.1;
		double one = 1.0;
		string most_common;
		vector<string>::iterator vi;
		string output_line="";
		while(cy < 10)
		{
			x = bx;
			++cy;
			cx = 0;
			while(cx < 10)
			{
				z = bz;
				++cx;
				//cout <<setprecision(10)<< "x= " << x << " bx= " << bx << endl;
				cz = 0;
				while(cz < 10)
				{
					total = 0;
					avg = 0.0;
					++cz;
					while(!points.empty())
					{
						top_line = points.top();
						points.pop();
						if(top_line.x >= x && top_line.x <= (x+inc) && top_line.y >= y && top_line.y <= (y +inc) && top_line.z >= z && top_line.z <= (z + inc))
						{
							avg = avg + top_line.lum;
							++total;
							if(top_line.label!="null")
								labels.push_back(top_line.label);
						}
						else
							tmp_stk.push(top_line);
					}
					points = tmp_stk;
					stack<line> tmp2_stk;
					tmp_stk = tmp2_stk;
					if(total!=0)
					{
						avg = avg / total;
						if(labels.empty())
						{
							global_labels.push_back("null");
						}
						else
						{
							max = 0;
							most_common = "null";
							map<string, int> m;
							for(vi = labels.begin(); vi != labels.end(); ++vi)
							{
								m[*vi]++;
								if(m[*vi] > max)
								{
									max = m[*vi];
									most_common = *vi;
								}
							}
							global_labels.push_back(most_common);
						}
					}
					else
					{
						global_labels.push_back("null");
					}
					labels.clear();
					output_line = output_line + to_string(avg) + " ";
					z = inc + z;
				}
				x = inc + x;
				output_line = output_line + "\n";
			}
			y=inc+y;

		}

		max = 0;
		most_common = "null";
		map<string, int> mp;
		for(vi = global_labels.begin(); vi != global_labels.end(); ++vi)
		{
			if(*vi == "null") continue;
			mp[*vi]++;
			if(mp[*vi] > max)
			{
				max = mp[*vi];
				most_common = *vi;
			}
		}
		string lbl = "";
		if(mp.size()==0)
		{
			lbl = "null";
		}
		else
			lbl = most_common;

		//cout << "Waiting to write" << endl;
		write_mutex.lock();
		points_file << output_line;
		points_file.flush();
		labels_file << lbl << "\n";
		labels_file.flush();
		//cout << output_line << endl;
		//cout << lbl << endl;
		write_mutex.unlock();
	}
};

double minX, maxX, minY, maxY, minZ, maxZ;
double currentX, currentY, currentZ;
void display_struct()
{
	for(int i=0; i<LN_NUMBER; ++i)
	{
		cout << "" << i << ": x=" << data[i].x << " y=" << data[i].y << " z=" << data[i].z << " lum=" << data[i].lum << " label=" << data[i].label << endl;
	}
}
void getMinMax()
{
	minX = data[0].x;
	maxX = data[0].x;
	minY = data[0].y;
	maxY = data[0].y;
	minZ = data[0].z;
	maxZ = data[0].z;
	for(int i=1; i<LN_NUMBER; ++i)
	{
		if(minX >= data[i].x)
			minX = data[i].x;
		if(maxX <= data[i].x)
			maxX = data[i].x;
		if(minY >= data[i].y)
			minY = data[i].y;
		if(maxY <= data[i].y)
			maxY = data[i].y;
		if(minZ >= data[i].z)
			minZ = data[i].z;
		if(maxZ <= data[i].z)
			maxZ = data[i].z;
	}
	minX -= 1.0;
	currentX = minX;
	currentY = minY;
	currentZ = minZ;
	cout << "MinX: " << minX << " MaxX: " << maxX <<" MinY: "<<minY<<" MaxY: "<<maxY<<" MinZ:  "<<minZ<< " MaxZ: "<<maxZ<< endl;
}
void createBigCube(double t_x, double t_y, double t_z)
{
	read_mutex.unlock();
	BigCube my_cube(t_x, t_y, t_z);
	//cout << "Started filling a cube" << endl;
	for(int i=0; i<LN_NUMBER; ++i)
	{
		my_cube.insert(data[i]);
	}
	//cout << "filled a cube" << endl;
	my_cube.classify();
	//cout << "Completed a cube" << endl;
}
void thread_work()
{
	while(true)
	{
		read_mutex.lock();
		if(currentZ > maxZ)
		{
			read_mutex.unlock();
			break;
		}
		else
		{
			if(currentX > maxX)
			{
				currentX = minX;
				currentY += MOVEMENT;
				if(currentY > maxY)
				{
					currentY = minY;
					currentZ += MOVEMENT;
					if(currentZ > maxZ)
					{
						read_mutex.unlock();
						break;
					}
				}
			}
			else
				currentX += MOVEMENT;
		createBigCube(currentX, currentY, currentZ);
		}
	}
}
int main()
{
	ifstream myfile(FL);
	if(myfile.is_open())
	{
		cout << "Opened File" << endl;
	}
	else
	{
		cout << "Not open" << endl;
		return 0;
	}
	points_file.open("points.txt");
	labels_file.open("labels.txt");
	string l, tmp;
	int i=0;
	int j;
	while(getline(myfile, l))
	{
		j=0;
		stringstream ssin(l);
		string arr[NB_ELEMENTS];
		while(ssin.good() && j<NB_ELEMENTS)
		{
			ssin >> arr[j++];
		}
		while(!ssin.eof())
		{
			ssin >> tmp;
			//cout << tmp << endl;
			arr[j-1] = arr[j-1] + " " + tmp;
		}
		data[i] = {stod(arr[0]), stod(arr[1]), stod(arr[2]), stod(arr[3]), arr[4]};
		++i;
	}
	//display_struct();
	cout << "getting MinMax" << endl;
	getMinMax();
	cout << "Got MinMax, now working" << endl;
	for(int i=0; i<NB_THREAD ; ++i)
	{
		workers.push_back(thread(thread_work));
	}
	for(thread &t: workers)
	{
		if(t.joinable())
			t.join();
	}
	cout <<  "All work completed";
	return 0;
}
