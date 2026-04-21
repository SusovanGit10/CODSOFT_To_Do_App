#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <ctime>
#include <regex>
#include <iomanip>

using namespace std;

// Colors
const string RED="\033[31m", YELLOW="\033[33m", GREEN="\033[32m", RESET="\033[0m";

// Filter enum
enum Filter {
    ALL = 0,
    COMPLETED,
    PENDING,
    OVERDUE
};

class Task {
public:
    string description, priority, dueDate;
    bool completed;

    Task(string d,string p,string date,bool c=false){
        description=d; priority=p; dueDate=date; completed=c;
    }
};

// ---------- Helpers ----------

string normalizePriority(string p){
    for(auto &c:p) c=tolower(c);
    if(p=="high") return "High";
    if(p=="medium") return "Medium";
    if(p=="low") return "Low";
    return "";
}

string getValidPriority(){
    string in;
    while(true){
        cout<<"Enter priority (High/Medium/Low): ";
        getline(cin,in);
        string p=normalizePriority(in);
        if(p!="") return p;
        cout<<"Invalid priority.\n";
    }
}

string coloredPriority(const string& p){
    if(p=="High") return RED+"High"+RESET;
    if(p=="Medium") return YELLOW+"Medium"+RESET;
    if(p=="Low") return GREEN+"Low"+RESET;
    return p;
}

int priorityValue(const string& p){
    if(p=="High") return 3;
    if(p=="Medium") return 2;
    return 1;
}

bool isLeapYear(int y){
    return (y%4==0 && y%100!=0)||(y%400==0);
}

bool isValidDate(const string& d){
    regex pat(R"(\d{2}-\d{2}-\d{4})");
    if(!regex_match(d,pat)) return false;

    int day=stoi(d.substr(0,2));
    int m=stoi(d.substr(3,2));
    int y=stoi(d.substr(6,4));

    if(m<1||m>12) return false;

    int days[]={31,28,31,30,31,30,31,31,30,31,30,31};
    if(isLeapYear(y)) days[1]=29;

    return day>=1 && day<=days[m-1];
}

int dateToInt(const string& d){
    if(d.size()!=10) return 0;
    return stoi(d.substr(6,4))*10000 +
           stoi(d.substr(3,2))*100 +
           stoi(d.substr(0,2));
}

int todayDate(){
    time_t t=time(0);
    tm* n=localtime(&t);
    return (n->tm_year+1900)*10000+(n->tm_mon+1)*100+n->tm_mday;
}

// ---------- Sorting ----------

void autoSort(vector<Task>& t){
    sort(t.begin(),t.end(),[](Task a,Task b){
        if(priorityValue(a.priority)!=priorityValue(b.priority))
            return priorityValue(a.priority)>priorityValue(b.priority);
        return dateToInt(a.dueDate)<dateToInt(b.dueDate);
    });
}

void sortByPriority(vector<Task>& t){
    sort(t.begin(),t.end(),[](Task a,Task b){
        return priorityValue(a.priority)>priorityValue(b.priority);
    });
    cout<<"Sorted by priority.\n";
}

void sortByDate(vector<Task>& t){
    sort(t.begin(), t.end(), [](Task a, Task b){
        if(dateToInt(a.dueDate)!=dateToInt(b.dueDate))
            return dateToInt(a.dueDate)<dateToInt(b.dueDate);
        return priorityValue(a.priority)>priorityValue(b.priority);
    });
    cout<<"Sorted by date.\n";
}

// ---------- File ----------

void loadTasks(vector<Task>& t){
    ifstream f("tasks.txt");
    if(!f) return;

    string d,p,date; bool s;
    while(getline(f,d)){
        getline(f,p); getline(f,date);
        f>>s; f.ignore();
        t.push_back(Task(d,p,date,s));
    }
    autoSort(t);
}

void saveTasks(const vector<Task>& t){
    ofstream f("tasks.txt");
    for(auto& x:t)
        f<<x.description<<"\n"<<x.priority<<"\n"<<x.dueDate<<"\n"<<x.completed<<"\n";
}

// ---------- Core ----------

void addTask(vector<Task>& t){
    cin.ignore();
    string d,date;

    cout<<"Enter task: ";
    getline(cin,d);

    string p=getValidPriority();

    do{
        cout<<"Enter due date (DD-MM-YYYY): ";
        getline(cin,date);
    }while(!isValidDate(date));

    t.push_back(Task(d,p,date));
    autoSort(t);

    cout<<"Task added.\n";
}

// ---------- View ----------

void viewTasks(const vector<Task>& t, int filter=ALL){
    if(t.empty()){ cout<<"No tasks.\n"; return; }

    int today=todayDate();

    const int W1=5,W2=30,W3=12,W4=15,W5=15;

    cout<<"\n"<<left<<setw(W1)<<"No"
        <<setw(W2)<<"Description"
        <<setw(W3)<<"Priority"
        <<setw(W4)<<"Due Date"
        <<setw(W5)<<"Status"<<endl;

    cout<<string(W1+W2+W3+W4+W5,'-')<<endl;

    int displayIndex = 0;

    for(int i=0;i<t.size();i++){

        bool overdue = (!t[i].completed &&
                        dateToInt(t[i].dueDate)<today);

        if(filter==COMPLETED && !t[i].completed) continue;
        if(filter==PENDING && (t[i].completed || overdue)) continue;
        if(filter==OVERDUE && !overdue) continue;

        displayIndex++;

        string desc=t[i].description;
        if(desc.size()>W2-1)
            desc=desc.substr(0,W2-4)+"...";

        string status;

        if(t[i].completed)
            status = GREEN + string("✔ Completed") + RESET;
        else if(overdue)
            status = RED + string("⚠ Overdue") + RESET;
        else
            status = string("⏳ Pending");

        cout<<setw(W1)<<to_string(displayIndex)+"."
            <<setw(W2)<<desc;

        cout<<coloredPriority(t[i].priority);

        int pad=W3 - t[i].priority.size();
        if(pad>0) cout<<string(pad,' ');

        cout<<setw(W4)<<t[i].dueDate
            <<setw(W5)<<status;

        cout<<endl;
    }
}

// ---------- Other ----------

void markCompleted(vector<Task>& t){
    int i; cout<<"Enter number: "; cin>>i;
    if(i<1||i>t.size()) return;
    t[i-1].completed=true;
}

void deleteTask(vector<Task>& t){
    int i; cout<<"Enter number: "; cin>>i;
    if(i<1||i>t.size()) return;

    cout<<"Confirm (y/n): ";
    char c; cin>>c;
    if(c=='y'||c=='Y') t.erase(t.begin()+i-1);
}

void searchTask(const vector<Task>& t){
    cin.ignore();
    string k; cout<<"Keyword: "; getline(cin,k);
    for(int i=0;i<t.size();i++)
        if(t[i].description.find(k)!=string::npos)
            cout<<i+1<<". "<<t[i].description<<"\n";
}

// ---------- Improved Edit ----------

void editTask(vector<Task>& t){
    int i; 
    cout<<"Enter task number: "; 
    cin>>i;

    if(i<1||i>t.size()){
        cout<<"Invalid number.\n";
        return;
    }

    cin.ignore();

    cout<<"New description: ";
    getline(cin,t[i-1].description);

    cout<<"New priority (High/Medium/Low): ";
    string p; getline(cin,p);

    string np=normalizePriority(p);
    if(np!="") t[i-1].priority=np;
    else cout<<"Invalid priority. Keeping old value.\n";

    string date;
    do{
        cout<<"New due date (DD-MM-YYYY): ";
        getline(cin,date);
    }while(!isValidDate(date));

    t[i-1].dueDate=date;

    autoSort(t);

    cout<<"Task updated.\n";
}

// ---------- Menu ----------

void menu(){
    cout<<"\n====== TO-DO LIST ======\n";
    cout<<"1.Add\n2.View All\n3.Mark\n4.Delete\n5.Search\n6.Edit\n";
    cout<<"7.Sort Priority\n8.Sort Date\n";
    cout<<"9.View Completed\n10.View Pending\n11.View Overdue\n12.Exit\nChoice: ";
}

// ---------- Main ----------

int main(){
    vector<Task> t;
    loadTasks(t);

    int c;
    do{
        menu();
        cin>>c;

        switch(c){
            case 1:addTask(t);break;
            case 2:viewTasks(t,ALL);break;
            case 3:markCompleted(t);break;
            case 4:deleteTask(t);break;
            case 5:searchTask(t);break;
            case 6:editTask(t);break;
            case 7:sortByPriority(t);break;
            case 8:sortByDate(t);break;
            case 9:viewTasks(t,COMPLETED);break;
            case 10:viewTasks(t,PENDING);break;
            case 11:viewTasks(t,OVERDUE);break;
            case 12:saveTasks(t);break;
        }

    }while(c!=12);

    return 0;
}