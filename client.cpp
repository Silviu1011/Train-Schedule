#include "helpers.h"

void write_to_server(int sd, std::string command) {
    int length = htonl(command.length());

    int sentBytes = send(sd, &length, 4, 0);
    if (sentBytes == -1){
        fprintf(stderr, "ERROR: Couldn't send message to server.\n");
        exit(EXIT_FAILURE);
    }

    char cmd_aux[200] = {'\0'};
    strcpy(cmd_aux, command.c_str());
    sentBytes = send(sd, &cmd_aux, command.length() + 1, 0);
    if (sentBytes == -1) {
        fprintf(stderr, "ERROR: Couldn't send the message to server.\n");
        exit(EXIT_FAILURE);
    }
}

std::string read_from_server(int sd) {
    std::string msg;
    int length;

    int sentBytes = recv(sd, &length, 4, 0);
    if(sentBytes == -1) {
        fprintf(stderr, "ERROR: Couldn't read message from server.\n");
        exit(EXIT_FAILURE);
    }
    length = ntohl(length);

    char msg_aux[900] = {'\0'};
    sentBytes = recv(sd, &msg_aux, length + 1, 0);
    if(sentBytes == -1) {
        fprintf(stderr, "ERROR: Couldn't read message from server.\n");
        exit(EXIT_FAILURE);
    }
    msg = msg_aux;

    return msg;
}

int main(int argc, char* argv[]) {
    // Checking if the command line contains all the information
    if(argc != 3) {
        fprintf(stderr, "Sintax: %s <IP> <PORT>.\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    // Socket initialization and connection to the server
    int sd = socket(AF_INET, SOCK_STREAM, 0); // socket descriptor
    if(sd == -1) {
        fprintf(stderr, "ERROR: Failed to create socket.\n");
        exit(EXIT_FAILURE);
    }
    int port = atoi(argv[2]);
    struct sockaddr_in server;
    server.sin_family = AF_INET; // socket family
    server.sin_addr.s_addr = inet_addr(argv[1]); // server IP adress
    server.sin_port = htons(port); // connection port
    int connection = connect(sd, (struct sockaddr*)&server, sizeof(struct sockaddr)); // conncetion to the server
    if(connection == -1) {
        fprintf(stderr, "ERROR: Failure to connect to server.\n");
        exit(EXIT_FAILURE);
    }

    std::cout<<"The command are: "<<std::endl;
    std::cout<<"1. get_trains_info"<<std::endl;
    std::cout<<"2. get_arrival_info [station]"<<std::endl;
    std::cout<<"3. get_departure_info [station]"<<std::endl;
    std::cout<<"4. add_arrival_delay <ID> <station> <delay>"<<std::endl;
    std::cout<<"5. add_departure_delay <ID> <station> <delay>"<<std::endl;
    std::cout<<"6. login <username>"<<std::endl;
    std::cout<<"7. logout"<<std::endl;
    std::cout<<"8. close"<<std::endl;

    while(true) {
        // Reading from terminal
        std::string command = "";
        std::cout << "client:$ ";
        std::getline(std::cin, command);

        // Writing to server
        write_to_server(sd, command);
        std::cout<<read_from_server(sd)<<std::endl;
        if(command == "close")
            break;
    }

    return 0;
}