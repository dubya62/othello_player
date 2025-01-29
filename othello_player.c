/*
 * Name: Holden Wells
 * CWID: 103-89-008
 * Date: November 11, 2024
 * Assignment Number: 3
 * Description: An Intelligent Othello player (and TUI) that uses a mini-max algorithm to make intelligent moves
 *              Hueristic simply uses (my pieces - your pieces) to decide
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>

// define what each of the spaces represent
#define BLACK 0
#define WHITE 1
#define EMPTY 2

// define the board dimensions (even though they should never change)
#define BOARD_WIDTH 8
#define BOARD_HEIGHT 8
#define BOARD_SIZE BOARD_WIDTH * BOARD_HEIGHT

#define ASCII_WIDTH 7
#define ASCII_HEIGHT 4

// settings to be changed
int debug = 0;
int pruning = 0;
int depth = 2;


// create (and setup) a board (an array of 64 integers) 
int* createBoard(){
    int* result = (int*) malloc(sizeof(int) * BOARD_SIZE);
    // initialize the board to empty
    for (int i=0; i<BOARD_SIZE; i++){
        result[i] = EMPTY;
    }

    // put the 2 black and 2 white pieces in the center
    int heightMiddle = (BOARD_HEIGHT >> 1) - 1;
    int widthMiddle = (BOARD_WIDTH >> 1) - 1;
    result[heightMiddle * BOARD_WIDTH + widthMiddle] = WHITE;
    result[heightMiddle * BOARD_WIDTH + widthMiddle + 1] = BLACK;
    result[(heightMiddle + 1) * BOARD_WIDTH + widthMiddle] = BLACK;
    result[(heightMiddle + 1) * BOARD_WIDTH + widthMiddle + 1] = WHITE;

    return result;
}

// print out a basic representation of the board
void printBoard(int* theBoard){
    int index = 0;
    for (int i=0; i<BOARD_HEIGHT; i++){
        for (int j=0; j<BOARD_WIDTH; j++){
            printf("%d ", theBoard[index]);
            index++;
        }
        printf("\n");
    }
}

// print ascii representation of the board
void printAsciiBoard(int* theBoard, int* validMoves){
    int bufferWidth = ASCII_WIDTH * BOARD_WIDTH + (BOARD_WIDTH-1);
    int bufferHeight = ASCII_HEIGHT * BOARD_HEIGHT + (BOARD_HEIGHT-1);
    int buffSize = bufferWidth * bufferHeight + bufferHeight;
    char* buffer = (char*) malloc(sizeof(char) * buffSize);

    for (int i=0; i<buffSize; i++){
        buffer[i] = ' ';
    }
    buffer[buffSize-1] = '\0';

    // put \n on the board
    for (int i=0; i<bufferHeight; i++){
        buffer[i*bufferWidth+bufferWidth+i] = '\n';
    }

    // put the pipes on the board
    for (int j=0; j<bufferHeight; j++){
        for (int i=ASCII_WIDTH; i<bufferWidth; i+=ASCII_WIDTH+1){
            buffer[j*bufferWidth+i+j] = '|';
        }
    }

    // put the dashes on the board
    for (int i=0; i<bufferHeight; i++){
        if (i % (ASCII_HEIGHT+1) == (ASCII_HEIGHT)){
            for (int j=0; j<bufferWidth; j++){
                if (j % (ASCII_WIDTH+1) == ASCII_WIDTH){
                    buffer[i*bufferWidth+j+i] = '+';
                } else {
                    buffer[i*bufferWidth+j+i] = '-';
                }
            }
        }
        
    }

    // put the numbers on the board
    char numBuffer[ASCII_WIDTH+1];
    for (int i=0; i<ASCII_WIDTH+1; i++){
        numBuffer[i] = ' ';
    }
    
    int index = 0;
    int stringLength;
    for (int i=0; i<bufferHeight; i+=ASCII_HEIGHT+1){
        for (int j=0; j<bufferWidth; j += ASCII_WIDTH+1){
            switch(theBoard[index]){
                case EMPTY:
                    if (validMoves == NULL || validMoves[index] == 1){
                        snprintf(numBuffer, ASCII_WIDTH, "%d", index+1);
                        stringLength = strlen(numBuffer);
                        memcpy(buffer+i*bufferWidth+j+i, numBuffer, stringLength);
                    }
                    break;
                case BLACK:
                    // black should be an empty square/O
                    for (int k=0; k<ASCII_WIDTH; k++){
                        for (int m=0; m<ASCII_HEIGHT; m++){
                            if (!m || !k || m == ASCII_HEIGHT-1 || k == ASCII_WIDTH-1){
                                buffer[(i+m)*bufferWidth+j+i+k+m] = '*';
                            }
                        }
                    }
                    break;
                case WHITE:
                    // white should be A filled in square
                    for (int k=0; k<ASCII_WIDTH; k++){
                        for (int m=0; m<ASCII_HEIGHT; m++){
                            buffer[(i+m)*bufferWidth+j+i+k+m] = 'X';
                        }
                    }
                    break;
            }
            index++;
        }
    }

    // put a \0 at the end of the content
    buffer[buffSize-1] = '\0';

    printf("%s\n", buffer);

    free(buffer);
}

// put a 1 in the result where that move is valid; 0 otherwise
// return the number of valid moves
int getValidMoves(int* theBoard, int* resultBoard, int moveColor){
    int result = 0;
    int otherPlayer = moveColor ^ 1;

    // start with no valid moves
    for (int i=0; i<BOARD_SIZE; i++){
        resultBoard[i] = INT_MIN;
    }

    // check each square for the valid moves it creates
    for (int i=0; i<BOARD_SIZE; i++){
        if (theBoard[i] == moveColor){
            int j = i - BOARD_WIDTH;
            int otherFound = 0;

            // check above
            while (j >= 0){
                if (otherFound && theBoard[j] == EMPTY){
                    if (resultBoard[j] != 1){
                        result++;
                    }
                    resultBoard[j] = 1;
                    break;
                } else if (theBoard[j] == otherPlayer){
                    otherFound = 1;
                } else if (otherFound){
                    break;
                } else {
                    break;
                }
                j -= BOARD_WIDTH;
            }

            // check below
            j = i + BOARD_WIDTH;
            otherFound = 0;
            while (j < BOARD_SIZE){
                if (otherFound && theBoard[j] == EMPTY){
                    if (resultBoard[j] != 1){
                        result++;
                    }
                    resultBoard[j] = 1;
                    break;
                } else if (theBoard[j] == otherPlayer){
                    otherFound = 1;
                } else if (otherFound){
                    break;
                } else {
                    break;
                }
                j += BOARD_WIDTH;
            }

            // 0, 1, 2, 3, 4, 5, 6, 7,
            // 0, 1, 2, 3, 4, 5, 6, 7,
            // 0, 1, 2, 3, 4, 5, 6, 7,
            // check left
            j = i - 1;
            otherFound = 0;
            while (j >= 0 && j % BOARD_WIDTH < BOARD_WIDTH-1){
                if (otherFound && theBoard[j] == EMPTY){
                    if (resultBoard[j] != 1){
                        result++;
                    }
                    resultBoard[j] = 1;
                    break;
                } else if (theBoard[j] == otherPlayer){
                    otherFound = 1;
                } else if (otherFound){
                    break;
                } else {
                    break;
                }
                j -= 1;
            }

            // check right
            j = i + 1;
            otherFound = 0;
            while (j % BOARD_WIDTH > 0){
                if (otherFound && theBoard[j] == EMPTY){
                    if (resultBoard[j] != 1){
                        result++;
                    }
                    resultBoard[j] = 1;
                    break;
                } else if (theBoard[j] == otherPlayer){
                    otherFound = 1;
                } else if (otherFound){
                    break;
                } else {
                    break;
                }
                j += 1;
            }

            // check up-right
            j = i + 1 - BOARD_WIDTH;
            otherFound = 0;
            while (j >= 0 && j % BOARD_WIDTH > 0){
                if (otherFound && theBoard[j] == EMPTY){
                    if (resultBoard[j] != 1){
                        result++;
                    }
                    resultBoard[j] = 1;
                    break;
                } else if (theBoard[j] == otherPlayer){
                    otherFound = 1;
                } else if (otherFound){
                    break;
                } else {
                    break;
                }
                j += 1 - BOARD_WIDTH;
            }

            // check down-right
            j = i + 1 + BOARD_WIDTH;
            otherFound = 0;
            while (j < BOARD_SIZE && j % BOARD_WIDTH > 0){
                if (otherFound && theBoard[j] == EMPTY){
                    if (resultBoard[j] != 1){
                        result++;
                    }
                    resultBoard[j] = 1;
                    break;
                } else if (theBoard[j] == otherPlayer){
                    otherFound = 1;
                } else if (otherFound){
                    break;
                } else {
                    break;
                }
                j += 1 + BOARD_WIDTH;
            }

            // check down-left
            j = i - 1 + BOARD_WIDTH;
            otherFound = 0;
            while (j < BOARD_SIZE && j % BOARD_WIDTH < BOARD_WIDTH-1){
                if (otherFound && theBoard[j] == EMPTY){
                    if (resultBoard[j] != 1){
                        result++;
                    }
                    resultBoard[j] = 1;
                    break;
                } else if (theBoard[j] == otherPlayer){
                    otherFound = 1;
                } else if (otherFound){
                    break;
                } else {
                    break;
                }
                j += -1 + BOARD_WIDTH;
            }

            // check up-left
            j = i - 1 - BOARD_WIDTH;
            otherFound = 0;
            while (j >= 0 && j % BOARD_WIDTH < BOARD_WIDTH-1){
                if (otherFound && theBoard[j] == EMPTY){
                    if (resultBoard[j] != 1){
                        result++;
                    }
                    resultBoard[j] = 1;
                    break;
                } else if (theBoard[j] == otherPlayer){
                    otherFound = 1;
                } else if (otherFound){
                    break;
                } else {
                    break;
                }
                j += -1 - BOARD_WIDTH;
            }


        }
    }
    
    return result;
}


// try to execute a string as a command
int runCommand(char* buffer){
    if (!strcmp(buffer, "debug on\n")){
        debug = 1;
        printf("Turning on debug mode!\n");
        return 1;
    } else if (!strcmp(buffer, "debug off\n")){
        debug = 0;
        printf("Turning off debug mode!\n");
        return 1;
    } else if (!strcmp(buffer, "pruning on\n")){
        pruning = 1;
        printf("Turning on alpha-beta pruning!\n");
        return 1;
    } else if (!strcmp(buffer, "pruning off\n")){
        pruning = 0;
        printf("Turning off alpha-beta pruning!\n");
        return 1;
    } else if (!strcmp(buffer, "depth\n")){
        printf("Changing the depth!\n");
        int waiting = 1;
        while (waiting){
            printf("What would you like the new depth to be? (odd to end on min layer): ");
            char depthBuffer[64];
            fgets(depthBuffer, 64, stdin);
            int newDepth = atoi(depthBuffer);
            if (newDepth <= 0){
                printf("Invalid Depth! Please try again.\n");
            } else {
                depth = newDepth;
                printf("Search depth has been changed to %d!\n", depth);
                waiting = 0;
            }
        }
        return 1;
    } else if (!strcmp(buffer, "help\n")){
        printf("Printing Help!\n");
        printf("debug [on/off] - change debug level\n");
        printf("pruning [on/off] - turn alpha-beta pruning on/off\n");
        printf("depth - change the depth of the search\n");
        printf("help - print this information\n");
        return 1;
    } else if (!strcmp(buffer, "settings\n")){
        printf("Displaying settings!\n");
        printf("DEBUG: %d\n", debug);
        printf("PRUNING: %d\n", pruning);
        printf("DEPTH: %d\n", depth);
        return 1;
    }

    return 0;
}


// ask a human player to make a move
int humanPlayer(int color, int* board){
    int response;
    char answerBuffer[64];
    while (1){
        printf("It is your turn! Where would you like to play?: ");
        fgets(answerBuffer, 64, stdin);
        answerBuffer[63] = '\0';
        response = atoi(answerBuffer);

        if (response == 0){
            // check to see if this is a command
            int commandStatus = runCommand(answerBuffer);

            // error condition
            if (!commandStatus){
                printf("Invalid Response. Please try again...\n");
            }
        } else {
            return response-1;
        }
    }
}

// place a piece at a certain place on the board and flip respective pieces
int placePiece(int* theBoard, int space, int moveColor){
    // return the number of pieces flipped
    int result = 0;
    int otherPlayer = moveColor ^ 1;

    // check each square for the valid moves it creates
    int i = space;
    int j = i - BOARD_WIDTH;
    int k = j;
    int otherFound = 0;

    theBoard[space] = moveColor;

    // check above
    while (j >= 0){
        if (otherFound && theBoard[j] == moveColor){
            do {
                theBoard[k] = moveColor;
                k -= BOARD_WIDTH;
                result++;
            } while (k >= j-1);
            break;
        } else if (theBoard[j] == otherPlayer){
            otherFound = 1;
        } else {
            break;
        }
        j -= BOARD_WIDTH;
    }

    // check below
    j = i + BOARD_WIDTH;
    k = j;
    otherFound = 0;
    while (j < BOARD_SIZE){
        if (otherFound && theBoard[j] == moveColor){
            do {
                theBoard[k] = moveColor;
                k += BOARD_WIDTH;
                result++;
            } while (k <= j);
            break;
        } else if (theBoard[j] == otherPlayer){
            otherFound = 1;
        } else {
            break;
        }
        j += BOARD_WIDTH;
    }

    // check left
    j = i - 1;
    k = j;
    otherFound = 0;
    while (j >= 0 && j % BOARD_WIDTH < BOARD_WIDTH-1){
        if (otherFound && theBoard[j] == moveColor){
            do {
                theBoard[k] = moveColor;
                k -= 1;
                result++;
            } while (k >= j);
            break;
        } else if (theBoard[j] == otherPlayer){
            otherFound = 1;
        } else {
            break;
        }
        j -= 1;
    }

    // check right
    j = i + 1;
    k = j;
    otherFound = 0;
    while (j % BOARD_WIDTH > 0){
        if (otherFound && theBoard[j] == moveColor){
            do {
                theBoard[k] = moveColor;
                k += 1;
                result++;
            } while (k <= j);
            break;
        } else if (theBoard[j] == otherPlayer){
            otherFound = 1;
        } else {
            break;
        }
        j += 1;
    }

    // check up-right
    j = i + 1 - BOARD_WIDTH;
    k = j;
    otherFound = 0;
    while (j >= 0 && j % BOARD_WIDTH > 0){
        if (otherFound && theBoard[j] == moveColor){
            do {
                theBoard[k] = moveColor;
                k += 1 - BOARD_WIDTH;
                result++;
            } while (k >= j);
            break;
        } else if (theBoard[j] == otherPlayer){
            otherFound = 1;
        } else {
            break;
        }
        j += 1 - BOARD_WIDTH;
    }

    // check down-right
    j = i + 1 + BOARD_WIDTH;
    k = j;
    otherFound = 0;
    while (j < BOARD_SIZE && j % BOARD_WIDTH > 0){
        if (otherFound && theBoard[j] == moveColor){
            do {
                theBoard[k] = moveColor;
                k += 1 + BOARD_WIDTH;
                result++;
            } while (k <= j);
            break;
        } else if (theBoard[j] == otherPlayer){
            otherFound = 1;
        } else {
            break;
        }
        j += 1 + BOARD_WIDTH;
    }

    // check down-left
    j = i - 1 + BOARD_WIDTH;
    k = j;
    otherFound = 0;
    while (j < BOARD_SIZE && j % BOARD_WIDTH < BOARD_WIDTH -1){
        if (otherFound && theBoard[j] == moveColor){
            do {
                theBoard[k] = moveColor;
                k += -1 + BOARD_WIDTH;
                result++;
            } while (k <= j);
            break;
        } else if (theBoard[j] == otherPlayer){
            otherFound = 1;
        } else {
            break;
        }
        j += -1 + BOARD_WIDTH;
    }

    // check up-left
    j = i - 1 - BOARD_WIDTH;
    k = j;
    otherFound = 0;
    while (j >= 0 && j % BOARD_WIDTH < BOARD_WIDTH-1){
        if (otherFound && theBoard[j] == moveColor){
            do {
                theBoard[k] = moveColor;
                k += -1 - BOARD_WIDTH;
                result++;
            } while (k >= j);
            break;
        } else if (theBoard[j] == otherPlayer){
            otherFound = 1;
        } else {
            break;
        }
        j += -1 - BOARD_WIDTH;
    }
    
    return result;
}

// basic heuristic: my_score - your_score
int getHeuristicValue(int color, int* boardState){
    int result = 0;
    for (int i=0; i<BOARD_SIZE; i++){
        if (boardState[i] == color){
            result++;
        } else if (boardState[i] != EMPTY){
            result--;
        }
    }
    return result;
}

// Recursively find the best child move
int findBestChild(int originalColor, int color, int* scanStatus, int** boardStates, int** validMoves, int actualDepth, int currentDepth, size_t* examinedStates, size_t* examinedNodes, int* pruneValues){
    if (debug){
        if (color == BLACK){
            printf("Finding the best move for BLACK\n");
        } else {
            printf("Finding the best move for WHITE\n");
        }
        printBoard(boardStates[currentDepth]);
        if (currentDepth & 1 == 1){
            printf("At depth %d, which is a min layer\n", currentDepth);
        } else {
            printf("At depth %d, which is a max layer\n", currentDepth);
        }
        if (currentDepth > 0){
            printf("Parent scanning progress: %d/%d\n", scanStatus[currentDepth-1], BOARD_SIZE);
        }
    }


    // if currentDepth == actualDepth-1, return the heuristic value
    if (currentDepth == actualDepth-1){
        int heuristicVal = getHeuristicValue(originalColor, boardStates[currentDepth]);
        if (debug){
            printf("Heuristic Value calculated at depth %d: %d\n", currentDepth, heuristicVal);
        }
        (*examinedStates)++;
        return heuristicVal;
    }


    // get valid moves for the current state
    int numValidMoves = getValidMoves(boardStates[currentDepth], validMoves[currentDepth], color);
    if (debug){
        printf("%d valid moves possible\n", numValidMoves);
    }
    scanStatus[currentDepth] = 0;


    if (numValidMoves == 0){
        memcpy(boardStates[currentDepth+1], boardStates[currentDepth], BOARD_SIZE * sizeof(int));
        // reset the next pruneValues since we are on a new branch
        pruneValues[currentDepth+1] = INT_MIN;

        // there is no valid move, so do not place a piece and just go to next state
        int bestChild = findBestChild(originalColor, color^1, scanStatus, boardStates, validMoves, actualDepth, currentDepth+1, examinedStates, examinedNodes, pruneValues);
        validMoves[currentDepth-1][scanStatus[currentDepth-1]] = bestChild;

        return bestChild;
    }

    int maxLayer = currentDepth % 2 == 0; // if even, max layer
    
    // otherwise, continue scanning for children at this depth
    int bestSoFar = INT_MIN;
    while(scanStatus[currentDepth] < BOARD_SIZE){
        if (validMoves[currentDepth][scanStatus[currentDepth]] == 1){ // this is a valid move that we have not looked at yet
            // reset the next pruneValues since we are just now exploring
            pruneValues[currentDepth+1] = INT_MIN;

            // copy the current status of the board to the next spot
            memcpy(boardStates[currentDepth+1], boardStates[currentDepth], BOARD_SIZE * sizeof(int));
            // make the move for the next state
            placePiece(boardStates[currentDepth+1], scanStatus[currentDepth], color);
            (*examinedNodes)++;
            
            // recursively get the heuristic value for this node
            int bestChild = findBestChild(originalColor, color^1, scanStatus, boardStates, validMoves, actualDepth, currentDepth+1, examinedStates, examinedNodes, pruneValues);

            if (bestSoFar == INT_MIN){
                bestSoFar = bestChild;
            } else {
                if (maxLayer && bestChild > bestSoFar){
                    bestSoFar = bestChild;
                } else if (!maxLayer && bestChild < bestSoFar){
                    bestSoFar = bestChild;
                }
            }

            validMoves[currentDepth][scanStatus[currentDepth]] = bestChild;

            if (pruning){
                if (currentDepth > 0){
                    if (pruneValues[currentDepth-1] != INT_MIN){
                        if (maxLayer){
                            // if this is a maxLayer and the best value so far is greater than the previous pruneValue,
                            // return the best so far
                            if (bestSoFar >= pruneValues[currentDepth-1]){
                                if (debug){
                                    printf("Pruned nodes at depth %d (maxLayer)\n", currentDepth);
                                    printf("Value found (%d) was >= pruneValue[%d] (%d)\n", bestSoFar, currentDepth-1, pruneValues[currentDepth-1]);
                                }
                                return bestSoFar;
                            }
                        } else {
                            // if this is a minLayer and the best value so far is less than the previous pruneValue,
                            // return the bestSoFar
                            if (bestSoFar <= pruneValues[currentDepth-1]){
                                if (debug){
                                    printf("Pruned nodes at depth %d (minLayer)\n", currentDepth);
                                    printf("Value found (%d) was <= pruneValue[%d] (%d)\n", bestSoFar, currentDepth-1, pruneValues[currentDepth-1]);
                                }
                                return bestSoFar;
                            }
                        }
                    }
                }
            }
        }
        // increment the status counter
        scanStatus[currentDepth]++;
    }

    // get the best child depending on min/max layer
    if (!maxLayer){ // min layer
        int best = INT_MAX;
        int bestMove = -1;
        for (int i=0; i < BOARD_SIZE; i++){
            if (validMoves[currentDepth][i] < best && validMoves[currentDepth][i] != INT_MIN){
                best = validMoves[currentDepth][i];
                bestMove = i;
            }
        }
        // set initial pruning value if needed
        if (pruning && currentDepth > 0){
            if (pruneValues[currentDepth-1] == INT_MIN || best > pruneValues[currentDepth-1]){
                pruneValues[currentDepth-1] = best;
                if (debug){
                    printf("Updated pruning value at depth %d to %d\n", currentDepth-1, best);
                }
            }
        }
        if (debug){
            printf("Min child found at depth %d = %d: move %d\n", currentDepth, best, bestMove);
            printf("Value for each move: (INT_MIN = invalid move)\n");
            printBoard(validMoves[currentDepth]);
        }
        return best;
    } else { // max layer
        int best = INT_MIN;
        int bestMove = -1;
        for (int i=0; i < BOARD_SIZE; i++){
            if (validMoves[currentDepth][i] > best && validMoves[currentDepth][i] != INT_MIN){
                best = validMoves[currentDepth][i];
                bestMove = i;
            }
        }
        // set initial pruning value if needed
        if (pruning && currentDepth > 0){
            if (pruneValues[currentDepth-1] == INT_MIN || best < pruneValues[currentDepth-1]){
                pruneValues[currentDepth-1] = best;
                if (debug){
                    printf("Updated pruning value at depth %d to %d\n", currentDepth-1, best);
                }
            }
        }
        if (debug){
            printf("Max child found at depth %d = %d: move %d\n", currentDepth, best, bestMove);
            printf("Value for each move: (INT_MIN = invalid move)\n");
            printBoard(validMoves[currentDepth]);
        }
        return best;
    }

}

// perform a move using the cpu
int cpuMove(int color, int* startingBoard){
    // use iteration instead of recursion
    clock_t startTime = clock();

    size_t examinedStates = 0;
    size_t examinedNodes = 0;
    int actualDepth = depth+1;
    int* scanStatus = (int*) malloc(sizeof(int) * actualDepth);
    int** boardStates = (int**) malloc(sizeof(int*) * actualDepth);
    int** validMoves = (int**) malloc(sizeof(int*) * actualDepth);
    int* pruneValues = (int*) malloc(sizeof(int) * actualDepth);
    for (int i=0; i<actualDepth; i++){
        scanStatus[i] = 0;
        pruneValues[i] = INT_MIN;
        boardStates[i] = createBoard();
        validMoves[i] = createBoard();
    }

    // copy the first board state to the first slot
    memcpy(boardStates[0], startingBoard, BOARD_SIZE*sizeof(int));

    // start recursively finding the best move
    findBestChild(color, color, scanStatus, boardStates, validMoves, actualDepth, 0, &examinedStates, &examinedNodes, pruneValues);

    // validMoves[0] now holds the score of each move. Maximize it for our color
    int bestMove = -1;
    int bestValue = INT_MIN;
    for (int i=0; i<BOARD_SIZE; i++){
        if (validMoves[0][i] > bestValue){
            bestMove = i;
            bestValue = validMoves[0][i];
        }
    }
    if (debug){
        printf("Final values of each move. (Maximize this)\n");
        printBoard(validMoves[0]);
    }
    printf("Computer search depth = %d\n", depth);
    printf("Best move value = %d\n", bestValue);
    printf("Computer thinks the best move is %d\n", bestMove+1);
    // if bestMove == -1, just pick the first valid move
    if (bestMove == -1){
        if (debug){
            printf("The computer thinks it is going to lose no matter what.\n");
        }
        int* onlyValidMoves = createBoard();
        getValidMoves(startingBoard, onlyValidMoves, color);
        for (int i=0; i<BOARD_SIZE; i++){
            if (onlyValidMoves[i] == 1){
                bestMove = i;
                if (debug){
                    printf("Computer picked %d since it is going to lose anyway :(\n", bestMove+1);
                }
                break;
            }
        }
    }

    // measure the time spent
    clock_t endTime = clock();
    double timeSpent = ((double) (endTime - startTime)) / CLOCKS_PER_SEC;

    printf("Computer examined %zu states (%zu nodes)!\n", examinedStates, examinedNodes);
    printf("Computer took %.2f seconds to decide!\n", timeSpent);

    // memory cleanup
    free(scanStatus);
    for (int i=0; i<actualDepth; i++){
        free(boardStates[i]);
        free(validMoves[i]);
    }
    free(boardStates);
    free(validMoves);
    return bestMove;
}


int computerPlayer(int color, int* board){
    printf("Computer is thinking...\n");
    return cpuMove(color, board);
}


// display the current score
int printScore(int* board){
    int blackScore = 0;
    int whiteScore = 0;
    for (int i=0; i<BOARD_SIZE; i++){
        if (board[i] == BLACK){
            blackScore++;
        } else if (board[i] == WHITE){
            whiteScore++;
        }
    }
    printf("Score:\n");
    printf("BLACK: %d\tWHITE: %d\n", blackScore, whiteScore);
    return 0;
}


// main function (entry point)
int main(int argc, char** argv){
    printf("Hello, World!\n");

    // create board state
    int* board = createBoard();
    int* validMoves = createBoard();

    // start the game loop
    printAsciiBoard(board, NULL);

    // ask how many human players you would like
    int waiting = 1;
    char answerBuffer[5];
    int numberOfPlayers;
    while (waiting){
        printf("How many human players?: ");
        fgets(answerBuffer, 5, stdin);
        answerBuffer[4] = '\0';
        numberOfPlayers = atoi(answerBuffer);
        if (!strcmp("\n", answerBuffer)){
            
        } else if (numberOfPlayers < 0 || numberOfPlayers > 2){
            printf("Invalid Response. Please try again...\n");
        } else {
            waiting = 0;
        }
    }

    int (*players[2])(int, int*);
    int playerColors[2];

    // put the playing functions in the correct spot depending on how many humans
    for (int i=0; i<2; i++){
        if (i < numberOfPlayers){
            players[i] = humanPlayer;
        } else {
            players[i] = computerPlayer;
        }
    }
    
    int turn = 0;

    // if there is one player, allow the human to choose which color they are
    waiting = 1;
    if (numberOfPlayers == 1){
        while (waiting){
            waiting = 0;
            printf("What color would you like to play as? (%d = BLACK, %d = WHITE)?: ", BLACK, WHITE);
            fgets(answerBuffer, 5, stdin);
            answerBuffer[4] = '\0';
            int theColor = atoi(answerBuffer);
            if (!strcmp("\n", answerBuffer)){
                waiting = 1;
            } else if (theColor == BLACK){
                playerColors[0] = BLACK;
                playerColors[1] = WHITE;
            } else if (theColor == WHITE){
                playerColors[0] = WHITE;
                playerColors[1] = BLACK;
                turn = 1;
            } else {
                printf("Invalid Response. Please try again...\n");
                waiting = 1;
            }
        }

    } else {
        playerColors[0] = BLACK;
        playerColors[1] = WHITE;
    }

    char commandBuffer[64];
    waiting = 1;
    while (waiting){
        printf("Would you like to change any settings before the game? [n to continue]: ");
        fgets(commandBuffer, 64, stdin);
        commandBuffer[63] = '\0';
        int theColor = atoi(commandBuffer);
        if (!strcmp(commandBuffer, "n\n")){
            waiting = 0;
        } else {
            runCommand(commandBuffer);
        }
    }

    int depth1 = -1;
    int depth2 = -1;
    if (numberOfPlayers == 0){
        waiting = 1;
        while (waiting){
            printf("What would you like the BLACK computer's depth to be?: ");
            fgets(commandBuffer, 64, stdin);
            commandBuffer[63] = '\0';
            int theDepth = atoi(commandBuffer);
            if (theDepth <= 0){
                printf("Invalid depth. Please try again...\n");
            } else {
                depth1 = theDepth;
                printf("BLACK computer's depth set to %d\n", depth1);
                waiting = 0;
                depth = depth1;
            }
        }
        waiting = 1;
        while (waiting){
            printf("What would you like the WHITE computer's depth to be?: ");
            fgets(commandBuffer, 64, stdin);
            commandBuffer[63] = '\0';
            int theDepth = atoi(commandBuffer);
            if (theDepth <= 0){
                printf("Invalid depth. Please try again...\n");
            } else {
                depth2 = theDepth;
                printf("WHITE computer's depth set to %d\n", depth2);
                waiting = 0;

            }
        }


    }


    // start the game loop
    int playing = 1;
    int lastValidMoves = 0;
    int skipped = 0;
    while (playing){
        if (playerColors[turn] == BLACK){
            printf("\nIt is BLACK's turn to go...\n");
        } else {
            printf("\nIt is WHITE's turn to go...\n");
        }

        printScore(board);

        // make sure that we always get a valid response
        int space;

        waiting = 1;
        while (waiting){
            int numberOfValidMoves = getValidMoves(board, validMoves, playerColors[turn]);
            printAsciiBoard(board, validMoves);

            if (numberOfValidMoves == 0){
                if (lastValidMoves == 0){
                    printf("Neither player has any valid moves left.\n");
                    playing = 0;
                    break;
                } 
                printf("You do not have any valid moves. Skipping your turn!\n");
                skipped = 1;
                lastValidMoves = 0;
                break;
            }
            skipped = 0;
            // run that player's function
            space = players[turn](playerColors[turn], board);

            if (space < 0 || space >= BOARD_SIZE || validMoves[space] == INT_MIN){
                printf("Invalid Move! Please try again...\n");
            } else {
                waiting = 0;
            }

            lastValidMoves = numberOfValidMoves;
        }
        if (!playing){
            break;
        }

        if (!skipped){
            placePiece(board, space, playerColors[turn]);
        }

        turn ^= 1;

        // if it is two computers, change the depth before the other player's move
        if (numberOfPlayers == 0){
            if (turn == 0){
                depth = depth1;
            } else {
                depth = depth2;
            }
        }
    }

    printf("The Game is Over!\n");

    // count the number of tokens for each player
    int blackCount = 0;
    int whiteCount = 0;
    for (int i=0; i<BOARD_SIZE; i++){
        if (board[i] == BLACK){
            blackCount++;
        } else if (board[i] == WHITE){
            whiteCount++;
        }
    }

    // print final score and who won
    printf("Black's Score: %d\n", blackCount);
    printf("White's Score: %d\n", whiteCount);
    if (blackCount > whiteCount){
        printf("Black Wins!\n");
    } else if (whiteCount > blackCount){
        printf("White Wins!\n");
    } else {
        printf("It is a Tie!\n");
    }

    if (numberOfPlayers == 0){
        printf("\n");
        printf("BLACK depth: %d\n", depth1);
        printf("WHITE depth: %d\n", depth2);
    }

    return 0;
}


