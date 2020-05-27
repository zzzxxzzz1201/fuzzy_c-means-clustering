// homework.cpp : 此檔案包含 'main' 函式。程式會於該處開始執行及結束執行。
//

#include "pch.h"
#include <iostream>
#include <cmath>
#include <opencv2/opencv.hpp>
#include <vector>
#include <set>

using namespace cv;
using namespace std;

void random_map(int num, int map_row, int map_col, vector<pair<int, int>> &record_point);//num是point數目
void rand_vertex(int num, int map_row, int map_col, vector<pair<int, int>> &record_vertex);//num是vertex數目
void random_color(vector<vector<int>> &random_color, int cluster_num);
void fuzzy(vector<vector<double>> &matrix_U, vector<pair<int, int>> &record_point, vector<pair<int, int>> &record_vertex, int m);
double distance(pair<int, int> a, pair<int, int> b);
bool error_diffrence_type1(vector<pair<int, int>> &record_vertex, vector<pair<int, int>> &new_record_vertex, double error);
bool error_diffrence_type2(vector<pair<int, int>> &record_vertex, vector<pair<int, int>> &new_record_vertex, double error);
void print_map_final(int map_row, int map_col, vector<vector<double>> &matrix_U, vector<vector<int>> &ran_color, vector<pair<int, int>> &record_point, vector<pair<int, int>> &record_vertex, vector<pair<int, int>> &new_record_vertex, int n);
void print_map_with_vertex(int map_row, int map_col, vector<vector<double>> &matrix_U, vector<vector<int>> &ran_color, vector<pair<int, int>> &record_point, vector<pair<int, int>> &record_vertex, vector<pair<int, int>> &new_record_vertex, int n);
void print_matrix_U(vector< vector<double> > matrix_U);
void new_vertex_and_error(vector<vector<double>> &matrix_U, vector<pair<int, int>> &record_point, vector<pair<int, int>> &new_record_vertex, int record_vertex_num, int m);
void rand_vertex_use_matrixU(int map_row, int map_col, int cluster_num, int m, vector<pair<int, int>> &record_point, vector< vector<double> > &matrix_U, vector<pair<int, int>> &record_vertex);
//--------------------------------------【main( )函數】---------------------------------------
//          描述：控制臺應用程式的入口函數，我們的程式從這里開始執行
//-----------------------------------------------------------------------------------------------
int main()
{
	int i = 0; //列印圖片用
	int map_row = 600;                     //圖片row
	int map_col = 600;				     //圖片col
	int point_num = 200;                   //幾個點
	int cluster_num = 10;                  //幾個cluster
	int m = 10;      //自己設的次方
	double error = 100;
	vector< vector<double> > matrix_U(cluster_num, vector<double>(point_num));  //紀錄隸屬矩陣
	vector< vector<int> > ran_color(cluster_num, vector<int>(3));
	vector<pair<int, int>> record_vertex;       //紀錄vertex
	vector<pair<int, int>> record_point;       //紀錄point
	vector<pair<int, int>> new_record_vertex;
	random_map(point_num, map_row, map_col, record_point);     //把map畫出來   事前工作
	random_color(ran_color, cluster_num);
	rand_vertex(cluster_num, map_row, map_col, record_vertex);//隨機vertex     事前工作
	//rand_vertex_use_matrixU(map_row, map_col, cluster_num, m, record_point, matrix_U, record_vertex);//和上面的rand_vertex可以互換

	do
	{
		fuzzy(matrix_U, record_point, record_vertex, m);
		print_matrix_U(matrix_U); //可拿掉
		new_vertex_and_error(matrix_U, record_point, new_record_vertex, record_vertex.size(), m);//改成傳進去大小record_vertex.size()
		//print_map_final(map_row, map_col, matrix_U, ran_color, record_point, record_vertex, new_record_vertex, i++); //保持同一張i設0
		print_map_with_vertex(map_row, map_col, matrix_U, ran_color, record_point, record_vertex, new_record_vertex, i++);
		waitKey(500);
	} while (error_diffrence_type1(record_vertex, new_record_vertex, error)); //這裡可以替換別的error方法
	cout << "end" << endl;
	waitKey(0);
	system("pause");
	return 0;
}

void random_map(int num, int map_row, int map_col, vector<pair<int, int>> &record_point)
{
	if (map_row*map_col < num)
	{
		cout << "vertex 數目過多" << endl;
		return;
	}
	cv::RNG rnger(cv::getTickCount());
	pair<int, int> rand_point;                 //用point會錯
	set<pair<int, int>> vertex_set;            //紀錄剛開始的V(用在剛開始初始時不重複)

	pair<int, int> a;
	int time = 0;  //紀錄vertex數目
	while (time < num)
	{
		rand_point.first = rnger.uniform(0, map_row);
		rand_point.second = rnger.uniform(0, map_col);
		if (vertex_set.find(rand_point) == vertex_set.end())
		{
			vertex_set.insert(rand_point);
			record_point.push_back(rand_point);
			time++;
		}
	}
	return;
}

void rand_vertex(int num, int map_row, int map_col, vector<pair<int, int>> &record)
{
	cv::RNG rnger(cv::getTickCount());
	pair<int, int> rand_vertex;

	for (int j = 0; j < num; j++)
	{
		bool same = false;                            //這個要放裡面
		rand_vertex.first = rnger.uniform(0, map_row);
		rand_vertex.second = rnger.uniform(0, map_col);
		for (int i = 0; i < record.size(); i++)
		{
			if (rand_vertex == record.at(i))
			{
				same = true;
				j--;        //沒有增加vertex  要把迴圈次數減回去
				break;
			}
		}
		if (same == false)
			record.push_back(rand_vertex);
	}
}

void rand_vertex_use_matrixU(int map_row, int map_col, int cluster_num, int m, vector<pair<int, int>> &record_point, vector< vector<double> > &matrix_U, vector<pair<int, int>> &record_vertex)//用矩陣U找最初的V(V是CLUSTER的群心)
{
	cv::RNG rnger(cv::getTickCount());
	pair<int, int> rand_vertex;

	int row = matrix_U.size();
	if (row == 0)
		return;
	int col = matrix_U.at(0).size();
	for (int i = 0; i < col; i++)
	{
		int value = 100;
		for (int j = 0; j < row - 1; j++)
		{
			int random_num = 0;//隨機選的值
			random_num = rnger.uniform(0, value);
			matrix_U.at(j).at(i) = (double)random_num / (double)100;
			value -= random_num;
		}
		matrix_U.at(row - 1).at(i) = (double)value / (double)100;
	}
	new_vertex_and_error(matrix_U, record_point, record_vertex, cluster_num, m);
	return;
}

void fuzzy(vector<vector<double>> &matrix_U, vector<pair<int, int>> &record_point, vector<pair<int, int>> &record_vertex, int m)
{
	int row = matrix_U.size();
	if (row == 0)
		return;
	int col = matrix_U.at(0).size();

	for (int i = 0; i < col; i++)
	{
		bool distance_zero = false;
		for (int j = 0; j < row; j++)          //matrix_U.at(row).at(col)
		{
			double total = 0;
			double upper = distance(record_point.at(i), record_vertex.at(j));
			for (int k = 0; k < record_vertex.size(); k++)
			{
				if (!isnormal((upper / distance(record_point.at(i), record_vertex.at(k)))))//其實應該要是if (distance(record_point.at(i), record_vertex.at(k))) == 0);
				{
					for (int l = 0; l < record_vertex.size(); l++)
					{
						if (l == k)
							matrix_U.at(l).at(i) = 1;              //原因是算出來distance是0的話代表太接近了  直接射程1  不然會出現 inf  的錯誤
						else
							matrix_U.at(l).at(i) = 0;
					}
					distance_zero = true;
					break;
				}

				total += pow(upper / distance(record_point.at(i), record_vertex.at(k)), (double)2 / (m - 1));   //2/(m-1)已經修改
			}
			if (distance_zero == true)
				break;
			matrix_U.at(j).at(i) = 1 / total;
		}
	}
}

double distance(pair<int, int> a, pair<int, int> b)
{
	int x = a.first - b.first;
	int y = a.second - b.second;
	return sqrt(pow(x, 2) + pow(y, 2));
}

void new_vertex_and_error(vector<vector<double>> &matrix_U, vector<pair<int, int>> &record_point, vector<pair<int, int>> &new_record_vertex, int record_vertex_num, int m)//p394 11.6.8
{
	new_record_vertex.clear();//把先前的資料刪掉

	for (int i = 0; i < record_vertex_num; i++)
	{
		pair<double, double> total_upper(0, 0);
		double total_bottom = 0;
		for (int j = 0; j < record_point.size(); j++)
		{
			total_upper.first += pow(matrix_U.at(i).at(j), m)*record_point.at(j).first;
			total_upper.second += pow(matrix_U.at(i).at(j), m)*record_point.at(j).second;
			total_bottom += pow(matrix_U.at(i).at(j), m);
		}
		total_upper.first = total_upper.first / total_bottom;
		total_upper.second = total_upper.second / total_bottom;
		new_record_vertex.push_back(pair<int, int>((int)total_upper.first, (int)total_upper.second));
	}
	return;
}

bool error_diffrence_type1(vector<pair<int, int>> &record_vertex, vector<pair<int, int>> &new_record_vertex, double error)  //全部組的誤差加起來大於error，true代表要做下一輪
{
	double total_error = 0;
	for (int i = 0; i < record_vertex.size(); i++)
	{
		total_error += distance(record_vertex.at(i), new_record_vertex.at(i));
		if (total_error > error)
		{
			cout << "________________________________________________\n";
			for (int j = 0; j < record_vertex.size(); j++)
			{
				record_vertex.at(j) = new_record_vertex.at(j);
			}
			return true;
		}
	}

	cout << "________________________________________________\n";
	return false;
}
bool error_diffrence_type2(vector<pair<int, int>> &record_vertex, vector<pair<int, int>> &new_record_vertex, double error) //每組內比較超過error就不行要重做，true代表要做下一輪
{
	for (int i = 0; i < record_vertex.size(); i++)
	{
		if (distance(record_vertex.at(i), new_record_vertex.at(i)) > error)
		{
			cout << "________________________________________________\n";
			for (int j = 0; j < record_vertex.size(); j++)
			{
				record_vertex.at(j) = new_record_vertex.at(j);
			}
			return true;
		}
	}
	cout << "________________________________________________\n";
	return false;
}

void print_map_final(int map_row, int map_col, vector<vector<double>> &matrix_U, vector<vector<int>> &ran_color, vector<pair<int, int>> &record_point, vector<pair<int, int>> &record_vertex, vector<pair<int, int>> &new_record_vertex, int n)//i是第幾次進來
{
	int num = 0;
	int cluster_num = record_vertex.size();
	Mat copy_one(map_row, map_col, CV_8UC3, Scalar(255, 255, 255));

	for (int i = 0; i < record_point.size(); i++)
	{
		double max = 0;
		int belong_claster = 0;  //屬於哪一個cluster;
		for (int j = 0; j < cluster_num; j++)
		{
			if (max < matrix_U.at(j).at(i))
			{
				max = matrix_U.at(j).at(i);
				belong_claster = j;
			}
		}

		circle(copy_one, Point(record_point.at(i).first, record_point.at(i).second), 4, Scalar(ran_color.at(belong_claster).at(0), ran_color.at(belong_claster).at(1), ran_color.at(belong_claster).at(2)), -1, 8);
	}
	imshow("rand_map_with_vertex" + to_string(n), copy_one);
}

void random_color(vector<vector<int>> &random_color, int cluster_num) //d
{
	cv::RNG rnger(cv::getTickCount());
	for (int i = 0; i < random_color.size(); i++)
	{
		for (int j = 0; j < random_color.at(i).size(); j++)
			random_color.at(i).at(j) = rnger.uniform(0, 255);
	}
}

void print_matrix_U(vector< vector<double> > matrix_U)//印出矩陣U  P382
{
	int row = matrix_U.size();
	if (row == 0)
		return;
	int col = matrix_U.at(0).size();
	for (int i = 0; i < col; i++)
	{
		cout << "point " << i << ": ";
		for (int j = 0; j < row - 1; j++)
		{
			printf("%1.2f , ", matrix_U.at(j).at(i));
		}
		printf("%1.2f\n", matrix_U.at(row - 1).at(i));
	}
}

void print_map_with_vertex(int map_row, int map_col, vector<vector<double>> &matrix_U, vector<vector<int>> &ran_color, vector<pair<int, int>> &record_point, vector<pair<int, int>> &record_vertex, vector<pair<int, int>> &new_record_vertex, int n)//劃出cluster的中心
{
	int num = 0;
	int cluster_num = record_vertex.size();
	Mat copy_one(map_row, map_col, CV_8UC3, Scalar(255, 255, 255));

	for (int i = 0; i < record_point.size(); i++)
	{
		double max = 0;
		int belong_claster = 0;  //屬於哪一個cluster;
		for (int j = 0; j < cluster_num; j++)
		{
			if (max < matrix_U.at(j).at(i))
			{
				max = matrix_U.at(j).at(i);
				belong_claster = j;
			}
		}

		circle(copy_one, Point(record_point.at(i).first, record_point.at(i).second), 3, Scalar(ran_color.at(belong_claster).at(0), ran_color.at(belong_claster).at(1), ran_color.at(belong_claster).at(2)), -1, 8);
	}
	int x_old;
	int y_old;
	int x_new;
	int y_new;
	for (int i = 0; i < record_vertex.size(); i++)
	{
		x_old = record_vertex.at(i).first;
		y_old = record_vertex.at(i).second;
		line(copy_one, Point(x_old - 5, y_old), Point(x_old + 5, y_old), Scalar(ran_color.at(i).at(0), ran_color.at(i).at(1), ran_color.at(i).at(2)), 2, 8);//十字原本的點，X是新的點
		line(copy_one, Point(x_old, y_old - 5), Point(x_old, y_old + 5), Scalar(ran_color.at(i).at(0), ran_color.at(i).at(1), ran_color.at(i).at(2)), 2, 8);//十字原本的點，X是新的點
		x_new = new_record_vertex.at(i).first;
		y_new = new_record_vertex.at(i).second;
		line(copy_one, Point(x_new - 4, y_new - 4), Point(x_new + 4, y_new + 4), Scalar(ran_color.at(i).at(0), ran_color.at(i).at(1), ran_color.at(i).at(2)), 2, 8);//十字原本的點，X是新的點
		line(copy_one, Point(x_new - 4, y_new + 4), Point(x_new + 4, y_new - 4), Scalar(ran_color.at(i).at(0), ran_color.at(i).at(1), ran_color.at(i).at(2)), 2, 8);//十字原本的點，X是新的點

		line(copy_one, Point(x_old, y_old), Point(x_new, y_new), Scalar(ran_color.at(i).at(0), ran_color.at(i).at(1), ran_color.at(i).at(2)), 2, 8);//新點和原本點距離
	}
	imshow("rand_map_with_vertex" + to_string(n), copy_one);
}