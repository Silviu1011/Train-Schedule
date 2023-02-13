#include "helpers.h"

#define PORT 2048

// Usernames vector
std::vector<std::string> usernames;
void get_usernames() {
    std::string names;
    std::ifstream fin("config.txt");

    std::getline(fin, names);

    fin.close();

    std::string delimiter = " ";
    size_t position = 0;
    std::string token;
    while((position = names.find(delimiter)) != std::string::npos) { //end of the string
        token = names.substr(0, position);
        usernames.push_back(token);
        names.erase(0, position + delimiter.length());
    }
    usernames.push_back(names);    
}

// Date and time implementation
class Date {
public:
    int day;
    int month;
    int year;
    bool operator== (Date& d) {
        if(this->day == d.day && this->month == d.month && this->year == d.year)
            return true;
        return false;
    }
};
class Time {
public:
    int hour;
    int minute;
    bool operator<= (Time& t) {
        if(this->hour < t.hour) 
            return true;
        else if(this->hour > t.hour)
            return false;
        else if(this->hour == t.hour) {
            if(this->minute <= t.minute)
                return true;
            else if(this->minute > t.minute)
                return false;
        }
        return false;
    }
};

// Geting the data from the xml file and adding it to the data vector
class Station {
public:
    std::string city;
    std::string arrivalTime;
    Time arr_time;
    int arrivalDelay;
    std::string departureTime;
    Time dep_time;
    int departureDelay;
};
class Train{
public:
    std::string id;
    std::string date_string;
    Date date;
    std::vector<Station> stations;
};
std::vector<Train> trains; // vector with all the data
void get_data() {
    std::ifstream fin;
    fin.open("data.xml");

    if(!fin.is_open()) {
        std::cout<<"Error at opening the xml file"<<std::endl;
        exit(1);
    }

    xml_document<> doc;
    std::stringstream buffer;
    buffer<<fin.rdbuf();
    fin.close();
    std::string data = buffer.str();
    doc.parse<0>(&data[0]);
    xml_node<>* root = doc.first_node();

    for(xml_node<>* train = root->first_node("train"); train; train = train->next_sibling()) {
        // Current train
        Train train_aux;

        // Train id
        train_aux.id = train->first_node("ID")->value();
        
        // Train date
        train_aux.date_string = train->first_node("date")->value();

        // Train date into Date class
        std::string date_aux = train_aux.date_string;
        std::string delimiter = ".";
        std::string token;
        std::vector<std::string> data;
        size_t position;
        while((position = date_aux.find(delimiter)) != std::string::npos) { //end of the string
            token = date_aux.substr(0, position);
            data.push_back(token);
            date_aux.erase(0, position + delimiter.length());
        }
        data.push_back(date_aux);
        train_aux.date.day = stoi(data[0]);
        train_aux.date.month = stoi(data[1]);
        train_aux.date.year = stoi(data[2]);
        for(xml_node<>* station = train->first_node("station"); station; station = station->next_sibling()) {
            // Current station
            Station station_aux;
            std::string aux;

            // Station location
            station_aux.city = station->first_node("city")->value();

            // Arrival time
            station_aux.arrivalTime = station->first_node("arrival")->value();
            aux = station_aux.arrivalTime;
            if(aux != "NULL") {
                delimiter = ":";
                data.clear();
                while((position = aux.find(delimiter)) != std::string::npos) { //end of the string
                    token = aux.substr(0, position);
                    data.push_back(token);
                    aux.erase(0, position + delimiter.length());
                }
                data.push_back(aux);
                station_aux.arr_time.hour = stoi(data[0]);
                station_aux.arr_time.minute = stoi(data[1]);
            }

            // Arrival delay
            aux = station->first_node("arrivalDelay")->value();
            station_aux.arrivalDelay = stoi(aux);
            
            // Departure time
            station_aux.departureTime = station->first_node("departure")->value();
            aux = station_aux.departureTime;
            if(aux != "NULL") {
                delimiter = ":";
                data.clear();
                while((position = aux.find(delimiter)) != std::string::npos) { //end of the string
                    token = aux.substr(0, position);
                    data.push_back(token);
                    aux.erase(0, position + delimiter.length());
                }
                data.push_back(aux);
                station_aux.dep_time.hour = stoi(data[0]);
                station_aux.dep_time.minute = stoi(data[1]);
            }

            // Departure delay
            aux = station->first_node("departureDelay")->value();
            station_aux.departureDelay = stoi(aux);

            // Adding the station to the station vector
            train_aux.stations.push_back(station_aux);
        }
        // Adding the train to the train vector
        trains.push_back(train_aux);
    }
}

// Checking data
bool check_id(std::string id) {
    for(auto train: trains) {
        if(train.id == id)
            return true;
    }
    return false;
}

bool check_city(std::string city) {
    for(auto train: trains) {
        for(auto station: train.stations) {
            if(station.city == city)
                return true;
        }
    }
    return false;
}

// Communication between client and server 
void write_to_client(int sd, std::string command) {
    int length = htonl(command.length());

    int sentBytes = send(sd, &length, 4, 0);
    if (sentBytes == -1){
        fprintf(stderr, "ERROR: Couldn't send message to client.\n");
        exit(EXIT_FAILURE);
    }

    char cmd_aux[900] = {'\0'};
    strcpy(cmd_aux, command.c_str());
    sentBytes = send(sd, &cmd_aux, command.length() + 1, 0);
    if (sentBytes == -1) {
        fprintf(stderr, "ERROR: Couldn't send the message to client.\n");
        exit(EXIT_FAILURE);
    }
}

std::string read_from_client(int sd) {
    std::string msg;
    int length;

    int sentBytes = recv(sd, &length, 4, 0);
    if(sentBytes == -1) {
        fprintf(stderr, "ERROR: Couldn't read message from client.\n");
        exit(EXIT_FAILURE);
    }
    length = ntohl(length);

    char msg_aux[200] = {'\0'};
    sentBytes = recv(sd, &msg_aux, length + 1, 0);
    if(sentBytes == -1) {
        fprintf(stderr, "ERROR: Couldn't read message from client.\n");
        exit(EXIT_FAILURE);
    }
    msg = msg_aux;

    return msg;
}

// Treating commands
struct command_input {
    int sd; // socket
    int tid; // Thread id
    bool connected = false;
    Date current_date;
    Time current_time;
    std::string cmd;
    std::string username; // client's username
    std::string id; // train id
    std::string station; 
    int delay; // train delay for departure or arrival
};
class Base_command {
public:
    virtual void execute(command_input) = 0;
};
class Get_trains_info: public Base_command {
public:
    void execute(command_input c) override {
        std::cout<<"Thread "<<c.tid<<": Started executing command get_trains_info"<<std::endl;

        std::string msg = "";
        for(auto train: trains) {
            if(train.date == c.current_date) {
                msg += "Train " + train.id + " schedule:\n";
                msg += "Date: " + train.date_string + "\n";
                for(auto station: train.stations) {
                    if(station.arrivalTime == "NULL") { // first statio
                        msg += "Train leaves " + station.city + " at " + station.departureTime;
                        if(station.departureDelay == 0)
                            msg += " according to the schedule\n";
                        else
                            msg += ", " + std::to_string(station.departureDelay) + " minutes late\n";
                    } else if(station.departureTime == "NULL") { // last station
                        msg += "Train arrives at " + station.city + " at " + station.arrivalTime;
                        if(station.arrivalDelay < 0) {
                            int delay = -station.arrivalDelay;
                            msg += ", " + std::to_string(delay) + " minutes early\n";
                        }
                        else if (station.arrivalDelay == 0) 
                            msg += " according to the schedule\n";
                        else
                             msg +=  ", " + std::to_string(station.arrivalDelay) + " minutes late\n";
                    } else {
                        // Arrival
                        msg += "Train arrives at " + station.city + " at " + station.arrivalTime;
                        if(station.arrivalDelay < 0) {
                            int delay = -station.arrivalDelay;
                            msg += ", " + std::to_string(delay) + " minutes early\n";
                        }
                        else if (station.arrivalDelay == 0) 
                            msg += " according to the schedule\n";
                        else
                             msg += ", " + std::to_string(station.arrivalDelay) + " minutes late\n";

                        // Departure
                        msg += "Train leaves " + station.city + " at " + station.departureTime;
                        if(station.departureDelay == 0)
                            msg += " according to the schedule\n";
                        else
                            msg += ", " + std::to_string(station.departureDelay) + " minutes late\n";
                    }
                }
            }
        }
        msg[msg.size() - 1] = '\0';
        write_to_client(c.sd, msg);

        std::cout<<"Thread "<<c.tid<<": Finished executing command get_trains_info"<<std::endl;
    }
};
class Get_arrival_info: public Base_command {
public:
    void execute(command_input c) override {
        std::cout<<"Thread "<<c.tid<<": Started executing command get_arrival_info"<<std::endl;

        bool found = false;
        std::string msg = "";
        Time end;
        end.hour = c.current_time.hour;
        end.minute = 59;
        if(c.station == "") {
            for(auto train: trains) {
                for(auto station: train.stations) {
                    if(station.arrivalTime != "NULL") {
                        if(c.current_time <= station.arr_time && station.arr_time <= end) {
                            found = true;
                            msg += "Train " + train.id + " arrives at " + station.city + " at " + station.arrivalTime;
                            if(station.arrivalDelay < 0) {
                            int delay = -station.arrivalDelay;
                                msg += ", " + std::to_string(delay) + " minutes early\n";
                            }
                            else if (station.arrivalDelay == 0) 
                                msg += " according to the schedule\n";
                            else
                                msg += ", " + std::to_string(station.arrivalDelay) + " minutes late\n";
                        } 
                    }  
                }
            }
            if(found == false)
                msg += "No trains arrive in the current hour";
            else 
                msg[msg.size() - 1] = '\0';
        } else {
            if(check_city(c.station) == false) {
                std::string error = "This city doesn't exist";
                write_to_client(c.sd, error);
            }
            for(auto train: trains) {
                for(auto station: train.stations) {
                    if(station.arrivalTime != "NULL" && station.city == c.station) {
                        if(c.current_time <= station.arr_time && station.arr_time <= end) {
                            found = true;
                            msg += "Train " + train.id + " arrives at " + station.city + " at " + station.arrivalTime;
                            if(station.arrivalDelay < 0) {
                            int delay = -station.arrivalDelay;
                                msg += ", " + std::to_string(delay) + " minutes early\n";
                            }
                            else if (station.arrivalDelay == 0) 
                                msg += " according to the schedule\n";
                            else
                                msg += ", " + std::to_string(station.arrivalDelay) + " minutes late\n";
                        } 
                    }  
                }
            }
            if(found == false)
                msg += "No trains arrive at station " + c.station + " in the current hour";
            else 
                msg[msg.size() - 1] = '\0';
        }
        write_to_client(c.sd, msg);

        std::cout<<"Thread "<<c.tid<<": Finished executing command get_arrival_info"<<std::endl;
    }
};
class Get_departure_info: public Base_command{
public:
    void execute(command_input c) override {
        std::cout<<"Thread "<<c.tid<<": Started executing command get_departure_info"<<std::endl;

        bool found = false;
        std::string msg = "";
        Time end;
        end.hour = c.current_time.hour;
        end.minute = 59;
        if(c.station == "") {
            for(auto train: trains) {
                for(auto station: train.stations) {
                    if(station.departureTime != "NULL") {
                        if(c.current_time <= station.dep_time && station.dep_time <= end) {
                            found = true;
                            msg += "Train " + train.id + " departs from " + station.city + " at " + station.departureTime;
                            if(station.departureDelay == 0)
                                msg += " according to the schedule\n";
                            else
                                msg += ", " + std::to_string(station.departureDelay) + " minutes late\n";
                        } 
                    }  
                }
            }
            if(found == false)
                msg += "No trains depart in the current hour";
            else 
                msg[msg.size() - 1] = '\0';
        } else {
            if(check_city(c.station) == false) {
                std::string error = "This city doesn't exist";
                write_to_client(c.sd, error);
            }
            for(auto train: trains) {
                for(auto station: train.stations) {
                    if(station.arrivalTime != "NULL" && station.city == c.station) {
                        if(c.current_time <= station.dep_time && station.dep_time <= end) {
                            found = true;
                            msg += "Train " + train.id + " departs from " + station.city + " at " + station.departureTime;
                            if(station.departureDelay == 0)
                                msg += " according to the schedule\n";
                            else
                                msg += ", " + std::to_string(station.departureDelay) + " minutes late\n";
                        } 
                    }  
                }  
            }
            if(found == false)
                msg += "No trains depart from station " + c.station + " in the current hour";
            else 
                msg[msg.size() - 1] = '\0';
        }
        write_to_client(c.sd, msg);

        std::cout<<"Thread "<<c.tid<<": Finished executing command get_departure_info"<<std::endl;
    }
};
class Add_arrival_delay: public Base_command {
public:
    void execute(command_input c) override {
        std::cout<<"Thread "<<c.tid<<": Started executing command add_arrival_delay"<<std::endl;

        if(c.connected == true) {
            if(check_id(c.id) == false) {
                std::string error = "This id doesn't exist";
                write_to_client(c.sd, error);
            } else if(check_city(c.station) == false) {
                std::string error = "This city doesn't exist";
                write_to_client(c.sd, error);
            } else {
                bool found = false;
                for(auto& train: trains) {
                    if(train.id == c.id) {
                        for(auto& station: train.stations) {
                            if(station.city == c.station) {
                                if(station.arrivalTime != "NULL") {
                                    found = true;
                                    station.arrivalDelay = c.delay;
                                    if(station.departureTime != "NULL") {
                                        if(c.delay <= 0) 
                                            station.departureDelay = 0;
                                        else 
                                            station.departureDelay = c.delay;
                                    }
                                }
                            } else if(found == true) {
                                if(c.delay <= 0) {
                                    if(station.arrivalTime != "NULL")
                                        station.arrivalDelay = 0;
                                    if(station.departureTime != "NULL")
                                        station.departureDelay = 0;
                                } else {
                                    if(station.arrivalTime != "NULL")
                                        station.arrivalDelay = c.delay;
                                    if(station.departureTime != "NULL")
                                        station.departureDelay = c.delay;
                                }
                            }
                        }
                    }
                }
                std::string msg = "Arrival delay updated succesfully";
                write_to_client(c.sd, msg);
            } 
        } else {
            std::string error = "An user must be connected in order for this command to work";
            write_to_client(c.sd, error);
        }
        
        std::cout<<"Thread "<<c.tid<<": Finished executing command add_arrival_delay"<<std::endl;
    }
};
class Add_departure_delay: public Base_command {
public:
    void execute(command_input c) override {
        std::cout<<"Thread "<<c.tid<<": Started executing command add_departure_delay"<<std::endl;

        if(c.connected == true) {
            if(check_id(c.id) == false) {
                std::string error = "This id doesn't exist";
                write_to_client(c.sd, error);
            } else if(check_city(c.station) == false) {
                std::string error = "This city doesn't exist";
                write_to_client(c.sd, error);
            } else if(c.delay < 0) {
                std::string error = "The delay must be greater or equal to 0";
                write_to_client(c.sd, error);
            } else {
                bool found = false;
                for(auto& train: trains) {
                    if(train.id == c.id) {
                        for(auto& station: train.stations) {
                            if(station.city == c.station) {
                                if(station.departureTime != "NULL") {
                                    found = true;
                                    station.departureDelay = c.delay;
                                }
                            } else if(found == true) {
                                if(station.arrivalTime != "NULL")
                                    station.arrivalDelay = c.delay;
                                if(station.departureTime != "NULL")
                                    station.departureDelay = c.delay;
                            }
                        } 
                    }
                }
                std::string msg = "Departure delay updated succesfully";
                write_to_client(c.sd, msg);
            }
        } else {
            std::string error = "An user must be connected in order for this command to work";
            write_to_client(c.sd, error);
        }

        std::cout<<"Thread "<<c.tid<<": Finished executing command add_departure_delay"<<std::endl;
    }
};
void login(command_input& c) {
    std::cout<<"Thread "<<c.tid<<": Started executing command login"<<std::endl;

    if(c.connected == true) {
        std::string error = "An user is already connected";
        write_to_client(c.sd, error);
    } else {
        bool found = false;
        for(auto user: usernames) {
            if(user == c.username) {
                found = true;
                break;
            }
        }
        if(found == true) {
            c.connected = true;
            std::string msg = "The user " + c.username + " was connected";
            write_to_client(c.sd, msg);
        } else {
            std::string error = "The user " + c.username + " doesn't exist";
            write_to_client(c.sd, error);
        }
    }

    std::cout<<"Thread "<<c.tid<<": Finished executing command login"<<std::endl;
}
void logout(command_input& c) {
    std::cout<<"Thread "<<c.tid<<": Started executing command logout"<<std::endl;

    if(c.connected == false) {
        std::string error = "No user is connected";
        write_to_client(c.sd, error);
    } else {
        c.connected = false;
        std::string msg = "The user " + c.username + " was disconnected succesfully";
        write_to_client(c.sd, msg);
    }

    std::cout<<"Thread "<<c.tid<<": Finished executing command logout"<<std::endl;
}

class Call_command {
    std::map<std::string, Base_command*> commands;
public:
    Call_command() {
        commands["get_trains_info"] = new Get_trains_info;
        commands["get_arrival_info"] = new Get_arrival_info;
        commands["get_departure_info"] = new Get_departure_info;
        commands["add_arrival_delay"] = new Add_arrival_delay;
        commands["add_departure_delay"] = new Add_departure_delay;
    }
    void execute_command_input(command_input c) {
        for(auto pair: commands) {
            if(pair.first == c.cmd) {
                Base_command* agent = pair.second;
                agent->execute(c);
                break;
            }
        }
    }
};

// Command queue
pthread_mutex_t cmdlock = PTHREAD_MUTEX_INITIALIZER;
std::queue<command_input> command_queue;
void* treat_queue(void* argc) {
    Call_command caller;
    while(true) {
        if(command_queue.size() == 0)
            continue;

        // Locking the thread
        pthread_mutex_lock(&cmdlock);

        // Executing the command
        command_input command = command_queue.front();
        caller.execute_command_input(command);
        command_queue.pop();

        // Unlocking the thread
        pthread_mutex_unlock(&cmdlock);
    }
    
    return(NULL);
}

// Treating threads
typedef struct thData{
	int idThread; 
	int cl; 
}thData;
void get_date_time(command_input& c) {
    time_t now = time(0);
    std::string date_time = ctime(&now);
    std::string delimiter = " ";
    std::string token;
    std::vector<std::string> data;
    size_t position;
    while((position = date_time.find(delimiter)) != std::string::npos) { //end of the string
        token = date_time.substr(0, position);
        if(token != " ")
            data.push_back(token);
        date_time.erase(0, position + delimiter.length());
    }
    data.push_back(date_time);

    c.current_date.day = stoi(data[2]); // current day
    // Current month
    if(data[1] == "Jan")
        c.current_date.month = 1;
    else if(data[1] == "Feb") 
        c.current_date.month = 2;
    else if(data[1] == "Mar") 
        c.current_date.month = 3;
    else if(data[1] == "Apr") 
        c.current_date.month = 4;
    else if(data[1] == "May") 
        c.current_date.month = 5;
    else if(data[1] == "Jun") 
        c.current_date.month = 6;
    else if(data[1] == "Jul") 
        c.current_date.month = 7;
    else if(data[1] == "Aug") 
        c.current_date.month = 8;
    else if(data[1] == "Sep") 
        c.current_date.month = 9;
    else if(data[1] == "Oct") 
        c.current_date.month = 10;
    else if(data[1] == "Nov") 
        c.current_date.month = 11;
    else if(data[1] == "Dec") 
        c.current_date.month = 12;
    // Current month
    c.current_date.year = stoi(data[4]);

    // Current time
    std::string time = data[3];
    delimiter = ":";
    data.clear();
    while((position = time.find(delimiter)) != std::string::npos) { //end of the string
        token = time.substr(0, position);
        data.push_back(token);
        time.erase(0, position + delimiter.length());
    }
    c.current_time.hour = stoi(data[0]);
    c.current_time.minute = stoi(data[1]);
}
void work(void* argc) {
    struct thData tdL; 
	tdL = *((struct thData*)argc);
    command_input c;
    c.tid = tdL.idThread;
    c.sd = tdL.cl;
    while(true) {
        // Parse the command parameters
        std::string command = read_from_client(c.sd); // read the command
        std::string delimiter = " ";
        std::vector<std::string> params;
        size_t position = 0;
        std::string token;
        while((position = command.find(delimiter)) != std::string::npos) { //end of the string
            token = command.substr(0, position);
            params.push_back(token);
            command.erase(0, position + delimiter.length());
        }
        params.push_back(command);
        c.cmd = params[0];

        // Get the current date and time
        get_date_time(c);

        // Checking parameters
        if(c.cmd == "get_trains_info") {
            if(params.size() == 1) {
                command_queue.push(c);
            } else {
                std::string error = "The number of parameters doesn't match";
                write_to_client(c.sd, error);
            }
        } else if(c.cmd == "get_arrival_info") {
            if(params.size() == 1) {
                c.station = "";
                command_queue.push(c);
            } else if(params.size() == 2) {
                c.station = params[1];
                command_queue.push(c);
            } else {
                std::string error = "The number of parameters doesn't match";
                write_to_client(c.sd, error);
            }
        } else if(c.cmd == "get_departure_info") {
            if(params.size() == 1) {
                c.station = "";
                command_queue.push(c);
            } else if(params.size() == 2) {
                c.station = params[1];
                command_queue.push(c);
            } else {
                std::string error = "The number of parameters doesn't match";
                write_to_client(c.sd, error);
            }
        } else if(c.cmd == "add_arrival_delay") {
            if(params.size() == 4) {
                c.id = params[1];
                c.station = params[2];
                c.delay = stoi(params[3]);
                command_queue.push(c);
            } else {
                std::string error = "The number of parameters doesn't match";
                write_to_client(c.sd, error);
            }
        } else if(c.cmd == "add_departure_delay") {
            if(params.size() == 4) {
                c.id = params[1];
                c.station = params[2];
                c.delay = stoi(params[3]);
                command_queue.push(c);
            } else {
                std::string error = "The number of parameters doesn't match";
                write_to_client(c.sd, error);
            }
        } else if(c.cmd == "login") {
            if(params.size() == 2) {
                c.username = params[1];
                login(c);
            } else {
                std::string error = "The number of parameters doesn't match";
                write_to_client(c.sd, error);
            }
        } else if(c.cmd == "logout") {
            if(params.size() == 1) {
                logout(c);
            } else {
                std::string error = "The number of parameters doesn't match";
                write_to_client(c.sd, error);
            }
        } else if(c.cmd == "close") {
            if(params.size() == 1) {
                std::string msg = "Client closed";
                write_to_client(c.sd, msg);
                break;
            } else {
                std::string error = "The number of parameters doesn't match";
                write_to_client(c.sd, error);
            }
        } else {
            std::string error = "The command " + c.cmd + " doesn't belong to the protocol";
            write_to_client(c.sd, error);
        }
    }
}

static void *treat(void * arg) {		
	struct thData tdL; 
	tdL= *((struct thData*)arg);	
    std::cout<<"Thread "<<tdL.idThread<<": Started working."<<std::endl; 

	pthread_detach(pthread_self());		
	work((struct thData*)arg);

    std::cout<<"Thread "<<tdL.idThread<<": Finished working."<<std::endl; 
	close ((intptr_t)arg);
	return(NULL);		
}

int main(void) {
    // Initializing the threat that treats the command queue
    pthread_t id;
    pthread_create (&id, NULL, &treat_queue, &id);

    // Get the usernames from the config file
    get_usernames();

    // Getting the date from the xml file
    get_data();

    // Server initialization
    struct sockaddr_in server;	
    struct sockaddr_in from;	
    int i = 0;	
    int sd; // socket descriptor
    pthread_t th[100]; // thread's id list
    
    // Socket initialization
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        fprintf(stderr, "ERROR: Failed to create socket.\n");
        exit(1);
    }

    // Using SO_REUSEADDR option
    int on=1;
    setsockopt(sd ,SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    
    bzero(&server, sizeof(server));
    bzero(&from, sizeof(from));
    
    server.sin_family = AF_INET;	
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(PORT);
    
    // Attaching the socket
    if (bind(sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1) {
        fprintf(stderr, "ERROR: Failed to bind.\n");
        exit(1);
    }

    if (listen (sd, 2) == -1) {
        fprintf(stderr, "ERROR: Failed at listen.\n");
        exit(1);
    }
    while (true) {
        int client;
        thData * td;  
        socklen_t length = sizeof(from);

        std::cout<<"Waiting at port "<<PORT<<std::endl; 

        if ((client = accept (sd, (struct sockaddr *)&from, &length)) < 0) {
            fprintf(stderr, "ERROR: Failed to connect.\n");
            continue;
        }

        td=(struct thData*)malloc(sizeof(struct thData));	
        td->idThread=i++;
        td->cl=client;
        pthread_create(&th[i], NULL, &treat, td);	              
    }  
    return 0;
}