#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<stdbool.h>
#include<signal.h>
#include<pthread.h>
#include<time.h>
#include<errno.h>

///////////////////////
//// Global Declaration of Variables
///////////////////////

///////////////////////
//// Const Block
///////////////////////
const unsigned long MIN_THREADS = 1;
const unsigned long MAX_THREADS = 24;
const unsigned long MIN_LIMIT = 100;
const unsigned long BLOCK  = 5000;

///////////////////////
//// Non-Const Block
///////////////////////
unsigned long currentIndex = 1;
unsigned long userLimit = 0;
unsigned long evilNumberCount = 0;
unsigned long odiousNumberCount = 0;
int           functionnSelector = 0;

///////////////////////
///// Mutex Locks
///////////////////////
pthread_mutex_t evilLock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t odiusLock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t indexLock = PTHREAD_MUTEX_INITIALIZER;

///////////////////////
//// Function Stubs 
///////////////////////
bool getArguments(int argc, char* argv[]);
void *findEvilNumbersCnt1();
void *findEvilNumbersCnt2();
int findBinaryOnes(unsigned long val);
bool parseInput(char * string);
bool checkStrings(char * str1, char * str2);
bool checkStrings(char * str1, char * str2);

int main(int argc, char* argv[]){
    if (getArguments(argc, argv)){
        // Local Main Variable Declarations
        unsigned long userThreads = strtoul(argv[2], NULL, 10); // Number of threads to assign threads
        userLimit = strtoul(argv[4], NULL, 10);                 // This is the limit or "block"
        int funcSelection = atoi(argv[6]);                      // Function selection
        pthread_t threadID[*argv[2]];                           // this is the part where we create the threads

        printf("Hardware Cores: %d\t\n", 8);
        printf("Thread Count: %ld\t\n", userThreads);
        printf("Numbers Limit: %ld\t\n\n",userLimit);
        printf("Please wait. Running...\n");    

        if(funcSelection == 1){
            for (int i = 0 ; i < userThreads ; i++ ){
                if (pthread_create(&threadID[i], NULL, findEvilNumbersCnt1, NULL) != 0){
                    printf("Thread Creation Error");
                    return -1;
                }
            }
        } else {
            for (int i = 0 ; i < userThreads ; i++ ){
                if (pthread_create(&threadID[i], NULL, findEvilNumbersCnt2, NULL) != 0){
                    printf("Thread Creation Error");
                    return -1;
                }
            }
        }

        // Join up all the used threads
        for (int i = 0 ; i < userThreads ; i++){
            pthread_join(threadID[i], NULL);
        }

        // Print block for values;
        printf("Evil/Odious Numbers Results\n\n");
        printf("\tEvil Number Count:\t%ld\n", evilNumberCount);
        printf("\tOdious Number Count:\t%ld\n", odiousNumberCount);
    } else{
        // Error, this means that get args failed to return true
        return 1;
    }

    return 0;
}

/// @brief Function that uses bitwise operators and mod to find number of bits.
/// @param val 
/// @return returns number of bits used for number.
int findBinaryOnes(unsigned long val){
    unsigned long bitshift = val;
    unsigned long count = 0;

    while (bitshift > 0){
        // printf("bitshift pre:  %ld\n", bitshift);
        if (bitshift % 2 == 1){
            count++;
            bitshift = bitshift >> 1;
        } else {
            bitshift = bitshift >> 1;
        }
        // printf("bitshift post:  %ld\n", bitshift);
        // printf("count: %ld\n", count);
    }

    return count;
}

/// @brief Takes in the values from argv/argc to process them
/// @param numOfArgs 
/// @param args 
/// @return returns true if good false if not good
bool getArguments(int numOfArgs, char* args[]){
    if (numOfArgs < 7){
        printf("./evilNums -t <threadCount> -l <limitValue> -f<1|2>\n");
        return false;
    }

    ////////////////////////////////////
    // Check the T block
    ////////////////////////////////////

    if(!checkStrings("-t", args[1])){
        printf("Error, invalid thread count specifier.\n");
        return false;
    }

    if(!parseInput(args[2])){
        printf("invalid thread num");
        return false;
    }
    
    unsigned long userThreads = strtoul(args[2], NULL, 10);
    if(userThreads < MIN_THREADS || userThreads > MAX_THREADS){
        printf("Error, thread count out of range.\n");
        return false;
    }

    ////////////////////////////////////
    // Check L Block
    ////////////////////////////////////

    // Error, invalid limit specifier
    if(!checkStrings("-l", args[3])){
        printf("Error, invalid limit specifier.");
        return false;
    }


    // Error, user limit must be > 100
    userLimit = strtoul(args[4], NULL, 10);
    if (*args[4] > MIN_LIMIT){
        printf("Error, Limit must be > 100\n");
        return false;
    }

    ////////////////////////////////////
    // Check F Block
    ////////////////////////////////////

    if (!checkStrings("-f", args[5])){
        printf("Error, invalid function specifier");
        return false;
    }

    if (*args[6] != '1' && *args[6] != '2'){
        printf("Error, wrong function selection");
        return false;
    }

    return true;
}


/// @brief This takes in a value and finds the evil/odious numbers
/// @param val 
/// @return returns null, void star.
void * findEvilNumbersCnt1(){

    while (1){    
        pthread_mutex_lock(&indexLock);         // mutex lock for our index

        if (currentIndex >= userLimit){
            pthread_mutex_unlock(&indexLock);   // unlock before break
            break;
        }

        unsigned long localIndex = currentIndex++; // Local Index used for odi numbers.
        pthread_mutex_unlock(&indexLock);
    
        if(findBinaryOnes(localIndex) % 2 == 0){
            pthread_mutex_lock(&evilLock);  // lock increment unlock
            evilNumberCount++;
            pthread_mutex_unlock(&evilLock);
        } else {
            pthread_mutex_lock(&odiusLock); // lock increment unlock
            odiousNumberCount++;
            pthread_mutex_unlock(&odiusLock);
        }
    }
    
    return NULL;                    // Void star so returns null.
}

/// @brief Finds Odious Numbers with multi threading, updates locally instead of globally
/// @return returns null, void star.
void * findEvilNumbersCnt2(){
   unsigned long localEvilCounter = 0;      // local counters
   unsigned long localOdiousCounter = 0;    // local counters
   
    while(1){
        pthread_mutex_lock(&indexLock);

        if (currentIndex >= userLimit){
            pthread_mutex_unlock(&indexLock);
            break;                          // can leave because we have hit our limit
        }

        unsigned long localIndex = currentIndex++;
        pthread_mutex_unlock(&indexLock);

        if(findBinaryOnes(localIndex) % 2 == 0){
            localEvilCounter++;
        } else {
            localOdiousCounter++;
        }
    }

    // Locks and handles count increase
    pthread_mutex_lock(&evilLock);
    evilNumberCount += localEvilCounter;
    pthread_mutex_unlock(&evilLock);

    pthread_mutex_lock(&odiusLock);
    odiousNumberCount += localOdiousCounter;
    pthread_mutex_unlock(&odiusLock);

    return NULL;
}

/// @brief Used to make sure that inputs only contain numbers and no letters
/// @param string 
/// @return returns true on successful parse
bool parseInput(char * string){
    bool goodInput = true;

    for (int i = 0 ; string[i] != '\0' && goodInput == true ; i++){
        if(string[i] <= '0' || string[i] >= '9'){
            goodInput = false;
            // printf("False\n");
        }

    }

    return goodInput;
}

/// @brief Made to varify strings for validation.
/// @param str1 
/// @param str2 
/// @return true if strings are the same.
bool checkStrings(char * str1, char * str2){
    bool valid = true;
    for (unsigned long i = 0 ; str1[i] != '\0' && valid ; i++){
        if (str1[i] != str2[i]){
            valid = false;
        }

    }

    return valid;
}