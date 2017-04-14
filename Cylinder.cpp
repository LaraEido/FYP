#include <iostream>
#include <math.h>
#include "json/json.h"
#include "jsoncpp.cpp"
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip>
#include <time.h>
#include <thread>
#include <vector>
#include <dirent.h>
#include <ctime>

#define RADIUS 0.05
#define RADIUS_SQ 0.0025

using namespace std;

ifstream *input_file;
ofstream *output_file;
std::vector<std::thread> workers;

// Lara Eidops Begins ;)))
struct vector_pv {
	double x;
	double y;
	double z;

};
struct point {
	double x;
	double y;
	double z;

};
vector_pv v; v.x = 1/sqrt(3); v.y = 1 / sqrt(3); v.z = 1 / sqrt(3);
vector_pv cross_product(vector_pv A, vector_pv B)
{
	vector_pv result;
	result.x = A.y*B.z - A.z*B.y;
	result.y = A.z*B.x - A.x*B.z;
	result.z = A.x*B.y - A.y*B.x;
	return result;
}

vector_pv unit_vector(vector_pv A)
{
	double norm = sqrt(A.x*A.x + A.y*A.y + A.z*A.z);

	vector_pv result;
	result.x = A.x / norm;
	result.y = A.y / norm;
	result.z = A.z / norm;

	return result;
}

double distance_to_surface(Double a, Double b, Double c, Double d, double xa, double ya, double za)
{
	double norme = a*xa + b*ya + c*za + d;
	if (norme < 0) { norme = -norme; }
	return norme / sqrt(a*a + b*b + c*c);
}

point add_point_to_vector_with_coef(point A, vector_pv v, double k)
{
	point B;
	B.x = A.x + k*v.x;
	B.y = A.y + k*v.y;
	B.z = A.z + k*v.z;

	return B;
}
void cube(point A, vector_pv u)
{
	vector_pv w = cross_product(u, v);
	vector_pv v = cross_product(w, u);
	w = unit_vector(w);
	v = unit_vector(v);

	point A1 = add_point_to_vector_with_coef(add_point_to_vector_with_coef(A, v, 0.05), w, 0.05);
	point A2= add_point_to_vector_with_coef(add_point_to_vector_with_coef(A, v, 0.05), w, -0.05);
	point A3 = add_point_to_vector_with_coef(add_point_to_vector_with_coef(A, v, -0.05), w, 0.05);
	point A4 = add_point_to_vector_with_coef(add_point_to_vector_with_coef(A, v, -0.05), w, -0.05);

	point B1= add_point_to_vector_with_coef(A1, u, 1);
	point B2 = add_point_to_vector_with_coef(A2, u, 1);
	point B3 = add_point_to_vector_with_coef(A3, u, 1);
	point B4 = add_point_to_vector_with_coef(A4, u, 1);



}
// lara EIdops END
double norm(Json::Value A, Json::Value B)
{
	return sqrt((A[0].asDouble() - B[0].asDouble())*(A[0].asDouble() - B[0].asDouble())+(A[1].asDouble() - B[1].asDouble())*(A[1].asDouble() - B[1].asDouble())+(A[2].asDouble() - B[2].asDouble())*(A[2].asDouble() - B[2].asDouble()));
}
bool between(Json::Value A, Json::Value B, double * test)
{
	double dx, dy, dz;	
	double pdx, pdy, pdz;	
	double dot, dsq;

	dx = B[0].asDouble() - A[0].asDouble();	
	dy = B[1].asDouble() - A[1].asDouble();     
	dz = B[2].asDouble() - A[2].asDouble();

	double lengthsq = dx * dx + dy * dy + dz * dz;

	pdx = test[0] - A[0].asDouble();		
	pdy = test[1] - A[1].asDouble();
	pdz = test[2] - A[2].asDouble();

	

	dot = pdx * dx + pdy * dy + pdz * dz;

	

	if( dot <= 0.0 || dot > lengthsq )
	{
		return( false );
	}
	else 
	{
		dsq = (pdx*pdx + pdy*pdy + pdz*pdz) - dot*dot/lengthsq;

		if( dsq > RADIUS_SQ )
		{
			return( false );
		}
		else
		{
			return( true );		
		}
	}
}
double* split(string str)
{
	string s[4];
	stringstream s2(str);
	int i=0;
	while(s2.good() &&i<4)
	{
		s2>>s[i];
		++i;
	}
	double *d=new double[4];
	for(i=0;i<4;i++)
	{
		stringstream s3;
		s3<<setprecision(10)<<s[i];
		s3>>d[i];
	}
	return d;
}
string Search(double *d, Json::Value f)
{
	for (int i=0;i<f.size();i++)
	{
		Json::Value &v= f[i]["geometry"]["coordinates"]; 
		
		for(int j=0;j<v.size()-1;j++)
		{
			if(between(v[j],v[j+1],d))
			{
				return f[i]["properties"]["asset_name"].asString();
			}
		}
	}
	return "null";
}
void thread_work(Json::Value f, int i, string file_name)
{
	string str;
	while(getline(input_file[i], str))
	{
		double *d=split(str);
		string s = Search(d, f);
		output_file[i] << setprecision(10) << d[0] << " " << d[1] << " " << d[2] << " " << d[3] << " " << s << endl;
	}
	time_t now = time(0);
	char* dt = ctime(&now);
	cout << "Thread " << i+1 << " Completed Work on File: " << file_name << "\n Time: " << dt << endl;
}
int main()
{
	vector<string> MyFiles;
	
	DIR *dir;
	struct dirent *entry;
	dir=opendir("../inputLaz");
	while((entry=readdir(dir))!=NULL)
	{
		if(strcmp(entry->d_name,".") && strcmp(entry->d_name,".."))
			MyFiles.push_back(entry->d_name);
	}
	closedir(dir);
	ifstream Vector_Files("marin_vectors.geojson");
	Json::Reader reader;
	Json::Value MyVectors;
	reader.parse(Vector_Files,MyVectors);
	Json::Value &f=MyVectors["features"];

	string str;
	int i=0;
	int j=0;
	input_file = new ifstream[MyFiles.size()];
	output_file = new ofstream[MyFiles.size()];
	for(i=0; i<MyFiles.size(); ++i)
	{
		string inpF = "../inputLaz/";
		string outF = "../outputLaz/";
		inpF.append(MyFiles[i]);
		outF.append(MyFiles[i]);
		
		input_file[i].open(inpF);
		output_file[i].open(outF);
		workers.push_back(thread(thread_work, f, i, MyFiles[i]));
	}

	for(thread &t: workers)
	{
		if(t.joinable())
			t.join();
	}
	delete [] input_file;
	delete [] output_file;
	return 0;
}
