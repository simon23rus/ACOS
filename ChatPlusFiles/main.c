//
//  main.c
//  ChatPlusFiles
//
//  Created by Simon Fedotov on 13.07.15.
//  Copyright (c) 2015 Simon23Rus. All rights reserved.
//



/*
 /file - To send some files
 /text - To send text message
*/

#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/shm.h>
#include <math.h>
#include <semaphore.h>
#include <dispatch/dispatch.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <limits.h>
#include <time.h>

#define BROADCAST -1
#define SMS "/text"
#define FILE "/file"

int SocketFD;
struct sockaddr_in OurAddress;
int UsersSockets[30];
int UsersNumber = 30;


long long ToSend(int SocketFileDesc, void* OurMessage, long long MessageSize, int Flags) {
    long long Sent = 0;
    
    while (Sent < MessageSize) {
        int BytesNumber = send(SocketFileDesc, OurMessage + Sent, MessageSize - Sent, 0);
        if(BytesNumber < 0)
            return 0;
        
        Sent += BytesNumber;
    }
    
    return Sent;
}



long long ToRecieve(int SocketFileDesc, void* OurMessage, long long MessageSize, int Flags) {
    long long Recieved = 0;
    
    while (Recieved < MessageSize) {
        int BytesNumber = recv(SocketFileDesc, OurMessage + Recieved, MessageSize - Recieved, 0);
        if(BytesNumber < 0)
            return 0;
        
        Recieved += BytesNumber;
    }
    
    return Recieved;
}





void* ServerJob(int* UserID) {
    int Counter = 0;
    printf("---------Connected New User with _[%d]_ ID-----------\n", *UserID);
    
    int OurUserID = *UserID;
    while (1) {
        char* KindOfData = malloc(255);
        int ForWhoID;
        long long MessageLength;
        char* Recieved = malloc(40000000000);
        
        
        
        int IsHere = recv(UsersSockets[OurUserID], (void*)&ForWhoID, sizeof(int), 0);
        if(IsHere <= 0) {
            printf("%d User has disconnected!\n", OurUserID);
            UsersSockets[OurUserID] = -1;
            break;
        }
        
        ToRecieve(UsersSockets[OurUserID], KindOfData, 5 * sizeof(char), 0);
        if(!strcmp(KindOfData, SMS)) {
            ToRecieve(UsersSockets[OurUserID], (void*)&MessageLength, sizeof(long long), 0);
            ToRecieve(UsersSockets[OurUserID], Recieved, MessageLength, 0);
            
            if(MessageLength == 0)
                continue;
            
            
            //        printf("[Server've got the message!]\n");
            
            time_t DatTime;
            struct tm * TimeInfo;
            
            time ( &DatTime );
            TimeInfo = localtime ( &DatTime );
            char* OurTime = malloc(255);
            strcpy(OurTime, asctime(TimeInfo));
            OurTime[strlen(OurTime) - 1] = '\0';
            
            printf("[%s] | [%d]User -> %s\n", OurTime, OurUserID, Recieved);
            memset(OurTime, '\0', strlen(OurTime));
            free(OurTime);
            
            
                if(ForWhoID == BROADCAST) {
                    for(int i = 0; i < UsersNumber; ++i) {
                        if(UsersSockets[i] != -1 && OurUserID != i) {
                            ToSend(UsersSockets[i], KindOfData, 5 * sizeof(char), 0);
                            ToSend(UsersSockets[i], (void*)&MessageLength, sizeof(long long), 0);
                            ToSend(UsersSockets[i], Recieved, MessageLength, 0);
                            ToSend(UsersSockets[i], (void*)&OurUserID, sizeof(int), 0);
                        }
                    }
                }
                
                else {
                    ToSend(UsersSockets[ForWhoID], KindOfData, 5 * sizeof(char), 0);
                    ToSend(UsersSockets[ForWhoID], (void*)&MessageLength, sizeof(long long), 0);
                    ToSend(UsersSockets[ForWhoID], Recieved, MessageLength, 0);
                    ToSend(UsersSockets[ForWhoID], (void*)&OurUserID, sizeof(int), 0);
                }
        }
        
        else { //File sending
            printf("U R HERE\n");
            printf("TO WGO %d\n", ForWhoID);
            printf("KIND %s\n", KindOfData);
            int NameLen = 0;
            long long DataLen = 0;
            char* FileName = malloc(1 << 10);
            memset(FileName, 0, 1 << 10);

            
            ToRecieve(UsersSockets[OurUserID], (void*)&NameLen, sizeof(int), 0);
            ToRecieve(UsersSockets[OurUserID], (void*)&DataLen, sizeof(long long), 0);
            ToRecieve(UsersSockets[OurUserID], FileName, NameLen, 0);
            ToRecieve(UsersSockets[OurUserID], Recieved, DataLen, 0);
        
            printf("NameLen %d\n", NameLen);
            
            if(NameLen == 0)
                continue;
            printf("_-'-_|_-'-_[Server've got the message!]_-'-_|_-'-_\n_!_!_INFO_!_!_\n");
            
            time_t DatTime;
            struct tm * TimeInfo;
            
            time ( &DatTime );
            TimeInfo = localtime ( &DatTime );
            char* OurTime = malloc(255);
            strcpy(OurTime, asctime(TimeInfo));
            OurTime[strlen(OurTime) - 1] = '\0';
            
            printf("[%s] | [%d]User -> %s\n", OurTime, OurUserID, FileName);
            memset(OurTime, '\0', strlen(OurTime));
            free(OurTime);
            
            
            
            if(ForWhoID == BROADCAST) {
                for(int i = 0; i < UsersNumber; ++i) {
                    if(UsersSockets[i] != -1 && OurUserID != i) {
                        ToSend(UsersSockets[i], KindOfData, 5 * sizeof(char), 0);
                        ToSend(UsersSockets[i], (void*)&NameLen, sizeof(int), 0);
                        ToSend(UsersSockets[i], (void*)&DataLen, sizeof(long long), 0);
                        ToSend(UsersSockets[i], FileName, NameLen, 0);
                        ToSend(UsersSockets[i], Recieved, DataLen, 0);
                        ToSend(UsersSockets[i], (void*)&OurUserID, sizeof(int), 0);

                    }
                }
            }
            
            else {
                ToSend(UsersSockets[ForWhoID], KindOfData, 5 * sizeof(char), 0);
                ToSend(UsersSockets[ForWhoID], (void*)&NameLen, sizeof(int), 0);
                ToSend(UsersSockets[ForWhoID], (void*)&DataLen, sizeof(long long), 0);
                ToSend(UsersSockets[ForWhoID], FileName, NameLen, 0);
                ToSend(UsersSockets[ForWhoID], Recieved, DataLen, 0);
                ToSend(UsersSockets[ForWhoID], (void*)&OurUserID, sizeof(int), 0);
            }
            
            printf("End\n");
            free(FileName);
            NameLen = 0;

        }
        
        memset(Recieved, '\0', strlen(Recieved));
        free(Recieved);
    }
    
    
    pthread_exit(NULL);
    
    
}


void* ClientRecieving(void* SavePath) {
    printf("--------Hello, Client Thread--------\n");
    
    while(1) {
        int Res = connect(SocketFD, (struct sockaddr*)&OurAddress, sizeof(OurAddress));
        long long MessageLength;
        int FromWho;
        char* Message = malloc(40000000000);
        char* KindOfData = malloc(255);
        
        long long ErrOrNotNull = ToRecieve(SocketFD, KindOfData, 5 * sizeof(char), 0);
        
        if(!strcmp(KindOfData, SMS)) {
            long long ErrOrNotFirst = ToRecieve(SocketFD, (void*)&MessageLength, sizeof(long long), 0);
            long long ErrOrNotSecond = ToRecieve(SocketFD, Message, MessageLength, 0);
            
            long long ErrOrNotThird = ToRecieve(SocketFD, (void*)&FromWho, sizeof(int), 0);
            
            if(ErrOrNotNull <= 0 && ErrOrNotFirst <= 0 && ErrOrNotSecond <= 0 && ErrOrNotThird)
                continue;
            
            printf("[%d]User -->> %s\n", FromWho, Message);
        }
        
        else {
            int NameLen = 0;
            char* FileName = malloc(1 << 10);
            memset(FileName, 0, 1 << 10);
            
            int Res = connect(SocketFD, (struct sockaddr*)&OurAddress, sizeof(OurAddress));
            
            long long ErrOrNotFirst = ToRecieve(SocketFD, (void*)&NameLen, sizeof(int), 0);
            long long ErrOrNotSecond = ToRecieve(SocketFD, (void*)&MessageLength, sizeof(long long), 0);
            long long ErrOrNotThird = ToRecieve(SocketFD, FileName, NameLen, 0);
            long long ErrOrNotForth = ToRecieve(SocketFD, Message, MessageLength, 0);
            long long ErrOrNotFifth = ToRecieve(SocketFD, (void*)&FromWho, sizeof(int), 0);

            
            if(NameLen == 0)
                continue;
            
            
            printf("You've recieved a FILE FROM  {%d} User\n", FromWho);
            
            
            printf("- NameLen --> %d\n", NameLen);
            printf("- Name --> %s\n", FileName);
            printf("- DataSize --> %lld \n", MessageLength);
            printf("- Data --> %s\n", Message);
            
            char* ToOpenFile = malloc(1 << 10);
            strcpy(ToOpenFile, SavePath);  // "/Users/semenfedotov/Threads/" optionally
            strcat(ToOpenFile, FileName);
            free(FileName);
            
            
            printf("FileNaame is = %s\n", ToOpenFile);
            
            int DatFileFD = open(ToOpenFile, O_RDWR | O_CREAT | S_IRWXU, 0777);
            if(DatFileFD < 0) {
                perror("Error while opening Server File");
                exit(1);
            }
            
            long long NumberWritten = 0;
            while (NumberWritten < MessageLength) {
                NumberWritten += write(DatFileFD, Message + NumberWritten, 4096);
            }
            if(NumberWritten < 0) {
                perror("Error while Writing Data into File");
                exit(1);
            }
            
            close(DatFileFD);
            
            
            free(ToOpenFile);

        }
        
        memset(Message, '\0', strlen(Message));
        free(Message);
    }
    pthread_exit(0);
}




int main(int argc, const char * argv[]) {
    
    printf("<><><><><><>-Hello-<><><><><><> \n");
    
    setbuf(stdout, 0);
    char* Mode = malloc(10);
    scanf("%s", Mode);
    SocketFD = socket(AF_INET, SOCK_STREAM, 0);
    if(SocketFD < 0) {
        perror("Error While socketing Client");
        exit(1);
    }
    
    int PORT;
    printf("Please, enter Port Number : ");
    scanf("%d", &PORT);
    
    OurAddress.sin_family = AF_INET;
    OurAddress.sin_port = htons(PORT);
    
    if(!strcmp(Mode, "Server")) {//Server
        printf("/\-/\-/\-Hello, Today you're a Server-/\-/\-/\ \n");
        OurAddress.sin_addr.s_addr = htonl(INADDR_ANY);
        
        if(bind(SocketFD, (struct sockaddr*)&OurAddress, sizeof(OurAddress)) < 0) {
            perror("Binding Error");
            exit(1);
        }
        listen(SocketFD, UsersNumber);
        memset(UsersSockets, -1, UsersNumber * sizeof(int));
        
        printf("_-_-_-__-_-_-_Server Has started without errors_-_-_-__-_-_-_\n");
        
        while (1) {
            int ClientSocket = accept(SocketFD,NULL, NULL);
            if(ClientSocket < 0){
                perror("Error While accepting");
                exit(1);
            }
            
            
            int UserID = 0;
            while (UsersSockets[UserID] != -1 && UserID < UsersNumber) {
                ++UserID;
            }
            if(UserID == UsersNumber) {
                printf("Error! Too many Users online\n");
                continue;
            }
            UsersSockets[UserID] = ClientSocket;
            
            pthread_t ServerThread;
            int bubu = pthread_create(&ServerThread, NULL, ServerJob, &UserID);
            if(bubu != 0)
                perror("Creating pthread error in Server");
        }
        
        
        
        
        
    }
    
    else {
        
        char* SavePath = malloc(255);
        printf("Enter IP : ");
        
        //        printf("/\.'.'.'.'.You're A Client!.'.'.'.'.'/\\n");
        //        printf("\/-_-_-____-_-_-____-_-_-____-_-_-___\/\n");
        //
        //        int PORT;
        //        char* IP = malloc(255);
        //        printf("Enter Port Number : ");
        //        scanf("%d", &PORT);
        //        printf("Enter IP Number : ");
        //        scanf("%s", IP);
        char * IP = malloc(255);
        scanf("%s", IP);
        
        OurAddress.sin_addr.s_addr = inet_addr(IP);
        
        printf("Please enter a Path Where would you like to save Files, which you'll recieve?\n");
        scanf("%s", SavePath);
        
        
        //        OurAddress.sin_family = AF_INET;
        //        OurAddress.sin_port = htons(PORT);
        //        OurAddress.sin_addr.s_addr = inet_addr(IP);
        //
        pthread_t ToRecieveClientThread;
        int bubu = pthread_create(&ToRecieveClientThread, NULL, ClientRecieving, SavePath);
        if(bubu != 0)
            perror("Creating pthread error in Client");
        //
        printf("Hi, now we're starting a Chat. Just enter your message and enjoy....\n");
        printf("Enter /file or /text and UserID and Message or phrase 'broadcast' to send a message for all Users \n");
        while(1){
            char* KindOfData = malloc(255);
            char* Message = malloc(1 << 10);
            long long MessageLength = 0;
            char WrittenSymbol = 0;
            int ToWhoID;
            char Command[9];
            scanf("%s", KindOfData);
            scanf("%s\n", Command);
            if(!strcmp(Command, "broadcast"))
                ToWhoID = -1;
            else
                ToWhoID = atoi(Command);
            
            if(!strcmp(KindOfData, SMS)) {
                int PositionInMessage = 0;
                do {
                    WrittenSymbol = getchar();
                    Message[PositionInMessage] = WrittenSymbol;
                    ++PositionInMessage;
                }while(WrittenSymbol != '\n' && WrittenSymbol != EOF);
                
                
                Message[strlen(Message) - 1] = '\0';
                MessageLength = strlen(Message);
                
                
                
                if(connect(SocketFD, (struct sockaddr *)&OurAddress, sizeof(OurAddress)) < 0){
                    //                perror("Client connection Error");
                }
                
                ToSend(SocketFD, (void*)&ToWhoID, sizeof(int), 0);
                ToSend(SocketFD, KindOfData, 5 * sizeof(char), 0);
                ToSend(SocketFD, (void*)&MessageLength, sizeof(long long), 0);
                ToSend(SocketFD, Message, MessageLength, 0);
            }
            else {
                printf("\n");
                char* FileName = malloc(1 << 10);
                char* Data = malloc(2000000000);
                int NameLen = 0;
                long long FileSize = 0;
                char* SendPath = malloc(255);
                printf("Enter Send Path\n");
                scanf("%s", SendPath);
                
                printf("Please, enter  Name of File: ");
                scanf("%s", FileName);
                
                char* FilePath = malloc(1 << 10);
                strcpy(FilePath, SendPath); // /Users/semenfedotov/FilesForThreads/ by default
                strcat(FilePath, FileName);
                
                
                printf("FileName = %s\n", FilePath);
                
                int ClientDescr = open(FilePath, O_RDWR | O_CREAT | S_IRWXU, 0777);
                if(ClientDescr < 0) {
                    perror("Error While opening Client File");
                    exit(1);
                }
                
                struct stat OurStatistic;
                int Check = fstat(ClientDescr, &OurStatistic);
                if(Check < 0) {
                    perror("Error while getting statistics");
                    exit(1);
                }
                
                
                
                printf("Size = %lld\n", OurStatistic.st_size);
                
                long long NumBytes = 0;
                long  long Counter = 0;
                while (NumBytes < OurStatistic.st_size) {
                    NumBytes += read(ClientDescr, Data + NumBytes, 4096);
                    ++Counter;
                    printf("Counter = %d\n", Counter);
                    printf("NumBytes = %d\n", NumBytes);
                }
                if(NumBytes < 0) {
                    perror("Error while reading Client File");
                    exit(1);
                }
                NumBytes = 0;
                Counter = 0;
                close(ClientDescr);
                
                NameLen = strlen(FileName);
                FileSize = OurStatistic.st_size;
                
                
                printf("ToSend Name = %s\n", FilePath);
                
                
                printf("Data !!! = %s\n Size = %d", Data, strlen(Data));
                printf("FileSize = %lld\n", FileSize);
                
                printf("Kind %s\n", KindOfData);
                
                ToSend(SocketFD, (void*)&ToWhoID, sizeof(int), 0);
                ToSend(SocketFD, KindOfData, 5 * sizeof(char), 0);
                ToSend(SocketFD, (void*)&NameLen, sizeof(int), 0);
                ToSend(SocketFD, (void*)&FileSize, sizeof(long long), 0);
                ToSend(SocketFD, FileName, NameLen, 0);
                ToSend(SocketFD, Data, FileSize, 0);
                
                free(FilePath);
                free(FileName);
                free(Data);
                
                
        }
                
            
            memset(Message, '\0', strlen(Message));
            free(Message);
        }
        
    }
    
    close(SocketFD);
    
    
    return 0;
}