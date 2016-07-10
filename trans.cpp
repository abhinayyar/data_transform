// Main file for data transform code

#include<iostream>
#include<vector>
#include<unordered_map>
#include<sstream>
#include<fstream>
#include<iomanip>
#include <cassert>
#include<algorithm>

#define WORD_SIZE 8 // 4 byte data
using namespace std;

// to convert hex to decimal
unsigned int get_dec(string raw)
{
	stringstream convert(raw);
	unsigned int ret;
	convert >> std::hex >> ret;
	return ret;
}
string int_to_hex(int i )
{
  std::stringstream stream;
  stream << std::setfill ('0') << std::setw(sizeof(int)*2) 
         << std::hex << i;
  return stream.str();
}
// function to get binary string from dec
string get_bin_str(unsigned int val,int ct)
{
	string res;
	int count=ct; // each 4 bit only , need to change for generic code
	while(count>0)
	{
		res.insert(res.begin(),(val&1)+'0');
		val>>=1;
		count--;
	}
	return res;
}
// function to convert hexadecimal to binary // returns output in form of string(0,1)
vector<string> convert_binary(string raw_data)
{
	vector<string> bin_data;
	for(int i=0;i<raw_data.size();i+=WORD_SIZE)
	{
		string cur = raw_data.substr(i,WORD_SIZE);
		string res;
		for(int j = 0; j<cur.size();j++)
		{
			string tmp;
			tmp.push_back(cur[j]);
			unsigned int val = get_dec(tmp);
			res+=get_bin_str(val,4);
		}
		bin_data.push_back(res);
	}
	return bin_data;
}
// To split each line -> SKIP in GPUSIM
vector<string> split(string input,char del)
{
	stringstream ss (input);
	string each;
	vector<string> res;
	while(getline(ss,each,del))
	{
		res.push_back(each);
	}
	return res;
}
// function to read raw data -> SKIP in GPUSIM
void read_data(string file_name,vector<string>& raw_data)
{
	ifstream ifile;
	ifile.open(file_name);
	string input;
	while(getline(ifile,input))
	{
		vector<string> each_line = split(input,' ');
		raw_data.push_back(each_line[3]);
	}
}
// function to handle corner cases for unsym rotate, not inplace algo
void unsym_rotate(vector<string>& matrix,int degree)
{
	int row = matrix.size();
	int col = matrix[0].size();
	
	
	if(degree == 180)
	{
		for(int i=0;i<row;i++) reverse(matrix[i].begin(),matrix[i].end());
		reverse(matrix.begin(),matrix.end());
		return;
	}
	vector<string> rotate_matrix;
	
	for(int i=0;i<col;i++)
	{
		string tmp;
		for(int j=0;j<row;j++)
		{
			if(degree == 90)
			tmp.insert(tmp.begin(),matrix[j][i]);
			else if(degree==270)
			tmp.push_back(matrix[j][i]);
		}
		rotate_matrix.push_back(tmp);
	}
	if(degree == 270)
	reverse(rotate_matrix.begin(),rotate_matrix.end());	
}
// function to try different rotations in matrix
void rotate_mat(vector<string>& matrix,int degree)
{
	int row = matrix.size();
	int col = matrix[0].size();
	
	degree = degree % 360;
	
	// invalid rotation degree if not 90 multiple
	if(degree%90!=0 || degree== 0 || matrix.size()==0) return;
	
	if(row!=col)
	{
		unsym_rotate(matrix,degree);
		return;
	}
	
	
	if(degree == 180)
	{
		for(int i=0;i<row;i++) reverse(matrix[i].begin(),matrix[i].end());
		reverse(matrix.begin(),matrix.end());
		return;
	}
	// rotation for sym matrix, inplace algo
	if( degree == 90)
	{
		reverse(matrix.begin(),matrix.end());
	}
	
	if(degree == 270)
	{
		for(int i=0;i<matrix.size();i++)
		{
			reverse(matrix[i].begin(),matrix[i].end());
		}
	}
	
	if(degree == 90 || degree == 270)
	{
		for(int i=0;i<matrix.size();i++)
		{
			for(int j=i+1;j<matrix[i].size();j++)
			{
				swap(matrix[i][j],matrix[j][i]);
			}
		}
	}
}
bool is_all_zero(string a,int& z_pairs,float& len_zero)
{
	string all_zero(WORD_SIZE*4,'0');
	assert(a.size()==WORD_SIZE*4);
	
	if(a.find(all_zero)!=string::npos) return true;
	
	float len=0;
	int ct=0;
	string name;
	for(int i=0;i<a.size();i++)
	{
		if(a[i]=='0') name.push_back(a[i]);
		else
		{
			ct += name.empty()==false ? 1 : 0;
			len+=name.size();
			name.clear(); 
		}
	}
	z_pairs+=ct;
	len_zero+=((1.0*len)/(1.0*ct));
	return false;
}
// function to get all zero row count
int get_all_zero_row(vector<string> test_data,int& z_pairs,float& len_zero)
{
	int zero_count=0;
	for(int i=0;i<test_data.size();i++)
	{
		if(is_all_zero(test_data[i],z_pairs,len_zero)) zero_count++; 
	}
	return zero_count;
}
string get_xor(string a,string b)
{
	string res;
	assert(a.size()==b.size());
	
	for(int i=0;i<a.size();i++)
	{
		res.push_back(((a[i]-'0')^(b[i]-'0'))+'0');
	}
	return res;
}
// N = N XOR N+1
vector<string> get_dbx(vector<string> raw_data)
{
	vector<string> res;
	for(int i=0;i<raw_data.size()-1;i++)
	{
		res.push_back(get_xor(raw_data[i],raw_data[i+1]));
	}
	res.push_back(raw_data.back());
	return res;
}
long get_num(string a)
{
	long res=0;
	for(int i=0;i<a.size();i++)
	{
		int c = a[i]-'0';
		res|=(c&1);
		if(i!=a.size()-1)
		res<<=1;	
	}
	return res;
}
// N+1 = N+1-N // delta calc
string calc_delta(string a,string b)
{
	long a_num = get_num(a);
	long b_num = get_num(b);
	
	int res = (int)(b_num-a_num);
	string hex_str = int_to_hex(res);
	string bi_val;
	for(int j = 0; j<hex_str.size();j++)
	{
		string tmp;
		tmp.push_back(hex_str[j]);
		unsigned int val = get_dec(tmp);
		bi_val+=get_bin_str(val,4);
	}
	return bi_val;
}
void get_delta_value(vector<string>& raw_data)
{
	vector<string> new_data;
	new_data.push_back(raw_data[0]);
	for(int i=1;i<raw_data.size();i++)
	{
		new_data.push_back(calc_delta(raw_data[i-1],raw_data[i]));
	}
	raw_data.clear();
	raw_data.insert(raw_data.end(),new_data.begin(),new_data.end());		
}
int traverse(vector<string> dbx,int start)
{
	if(start>=dbx.size()) return 0;
	
	string sam(WORD_SIZE*4,'0');
	string cur = dbx[start];
	auto it = cur.find("#");
	if(it!=string::npos)
	{
		cur.assign(cur.substr(it+1));
	}
	else
	{
		// already encoded
		return 0;
	}
	assert(cur.size()==sam.size());
	if(cur.find(sam)!=string::npos)
	{
		return traverse(dbx,start+1)+1;
	}
	return 0;
}
// encode for zeros
void encode_run_len(vector<string>& dbx)
{
	vector<string> tmp_dbx;
	int i=0;
	while(i<dbx.size())
	{
		int max_zero = traverse(dbx,i);
		string start_index;
		string code;
		// handle for both 1 and more than 1 cons zero case
		if(max_zero >=1)
		{
			start_index = max_zero >1 ? get_bin_str(max_zero,5) : "";
			code = max_zero==1 ? "001" : "01";
			tmp_dbx.push_back(code+start_index);
			i+=max_zero;
		}
		else
		{
			tmp_dbx.push_back(dbx[i]);
			i++;
		}		
	}
	dbx.clear();
	dbx.insert(dbx.end(),tmp_dbx.begin(),tmp_dbx.end());
}
bool one_check(vector<string>& tmp_dbx,string code,int len,string act,int it)
{
	string fw_str = act.substr(0,it);
	string bw_str = act.substr(it+len);
	assert(fw_str.size()+bw_str.size()+len==WORD_SIZE*4);
	string az_1(fw_str.size(),'0');
	string az_2(bw_str.size(),'0');
	if(fw_str.find(az_1)!=string::npos && bw_str.find(az_2)!=string::npos)
	{
			string index = get_bin_str(it,5);
			tmp_dbx.push_back(code+index);
			return true;
	}
	return false;
}
bool zero_check(string a)
{
	string zero(WORD_SIZE*4,'0');
	if(a.find(zero)!=string::npos) return true;
	return false;
}
// encode for all 1
void encode_others(vector<string>& dbx,vector<string> dbp)
{
	string one(WORD_SIZE*4,'1');
	string zero(WORD_SIZE*4,'0');
	string cons_one(2,'1');
	string sing_one(1,'1');
	
	vector<string> tmp_dbx;
	for(int i=0;i<dbx.size();i++)
	{
		string cur = dbx[i];
		auto it = cur.find("#");
		if(it!=string::npos)
		{
			// eligible for encode
			int org_index = stoi(cur.substr(0,it));
			// check for all ones
			string act = cur.substr(it+1);
			if(act.find(one)!=string::npos)
			{
				tmp_dbx.push_back("00000");
				continue;	
			}
			
			// cons 2 1's
			auto c_o = act.find(cons_one);
			if(c_o!=string::npos)
			{
				if(one_check(tmp_dbx,"00010",2,act,(int)c_o)) continue;
			}
			
			// single 1's
			
			auto s_o = act.find(sing_one);
			if(s_o!=string::npos)
			{
				if(one_check(tmp_dbx,"00011",1,act,(int)s_o)) continue;
			}
			// DBX!=0 & DBP=0
			assert(org_index<dbp.size());
			if(zero_check(dbp[org_index]))
			{
				tmp_dbx.push_back("00001");
				continue;
			}
			
			// Uncompressed 
		
			tmp_dbx.push_back(act);
			
		}
		else
		{
			tmp_dbx.push_back(dbx[i]);
		}
	}
	dbx.clear();
	dbx.insert(dbx.end(),tmp_dbx.begin(),tmp_dbx.end());
}
// create lookup
void create_lookup(vector<string>& dbx)
{
	for(int i=0;i<dbx.size();i++)
	{
		string num = to_string(i);
		dbx[i]=num+"#"+dbx[i];
	}
}
string form_output(vector<string> dbx)
{
	string output;
	for(int i=0;i<dbx.size();i++)
	{
		output+=dbx[i];
	}
	return output;
}
// Main function for data encode
string do_actual_encode(vector<string> dbp,vector<string> dbx)
{
	assert(dbp.size()==dbx.size());
	string output;
	// Done all encoding on dbp
	create_lookup(dbx);
	encode_run_len(dbx);
	encode_others(dbx,dbp);
	output = form_output(dbx);
	return output;
}
int main(int argc,char *argv[])
{
	if(argc<2) return 0;
	
	vector<string> raw_data;
	read_data(argv[1],raw_data);
	float original_bytes=0.0;
	float encoded_bytes =0.0;
	for(int i=0;i<raw_data.size();i++)
	{
		vector<string> raw_binary_data = convert_binary(raw_data[i]);
		get_delta_value(raw_binary_data);
		int max_degree=90;
		int all_zero_row=0;
		int zero_pairs =0;
		float avg_len_zero =0;
		for(int j=90;j<360;j+=90)
		{
			vector<string> test_data(raw_binary_data.begin(),raw_binary_data.end());
			rotate_mat(test_data,j);
			
			int z_pairs =0;
			float len_zero=0;
			int count = get_all_zero_row(test_data,z_pairs,len_zero);
			if(count > all_zero_row)
			{
				all_zero_row=count;
				max_degree=j;
			}
			else
			{
				max_degree = z_pairs > zero_pairs || len_zero > avg_len_zero ? j : max_degree;
			}
				
		}
		rotate_mat(raw_binary_data,max_degree);
		vector<string> dbp(raw_binary_data.begin(),raw_binary_data.end());
		vector<string> dbx = get_dbx(raw_binary_data);
		// do actual encode now
		string output = do_actual_encode(dbp,dbx);
		// for rotation we can use 2 bit extra for checking which is rotaion, For now skipped it, but 
		// can be added
		original_bytes+= (WORD_SIZE*4*dbp.size())/8;
		encoded_bytes+=(output.size()/8);
	}
	cout<<"Actual Bytes : "<<original_bytes<<" Encoded Bytes : "<<encoded_bytes<<endl;
	return 0;
}