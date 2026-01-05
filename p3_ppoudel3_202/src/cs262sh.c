#include "listnode.h"
#include "exec.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

/* Name: Pratyush Poudel
 * GNumber: G01491203
 *
 */

// function prototypes
void countArgs(char *userInput, int *count);
void populate(char *userInput, char *command, char *args1, char *args2, int *count);
void addToHist(char **histArr, char *userInput, int *index);
void printHistArr(char **histArr, int index);
void clearHist(char **histArr, int *index);
int checkIfValid(int index, char *args1);
void reDo(char *buffer, char *userInput, int hasInp, char **histArr, ListNode *head);
void getNewInp(char *args1, char **histArr, char *buffer);
void freeHistArr(char **histArr);
ListNode *makeNode(char *cmd, char *userInput, int count);
ListNode *insertNode(ListNode *head, char *cmd, char *userInput, int count);
void listsNodes(ListNode *head);
void freeLinks(ListNode *head);
void addToFileContents(ListNode *head, FILE *file, int index);
ListNode *removeNodes(ListNode *head, int id);
void freeSingleNode(ListNode *node);
ListNode *getNode(int id, ListNode *head);




int main(){
        // main buffer
        char buffer[10000];

        // history array
        char **histArr = malloc(sizeof(char *) * 100);
        // malloced space for each string with it being possibly 10k bites
        for (int i = 0; i < 100; i++){
                histArr[i] = malloc(sizeof(char) * 10000);
        }
        // index used in the history array
        int index = 0;
	int hasInput = 0;
	ListNode *head = NULL;

        while (1){
                // used to hold the char input until tokenized and parsed
                char userInput[10000] = " ";
                char command[10000] = " ";
                char args1[10000] = " ";
                char args2[10000] = " ";
                int count = 0;
		
		if (!hasInput){	
                	printf("262$");
                	fflush(stdout);
		}
                // reads each line and populate userInput

		reDo(buffer, userInput, hasInput, histArr, head);
		
		hasInput = 0;

                // populate the fields
                populate(userInput, command, args1, args2, &count);


                // quit command
                // ends and deallocates
                if (strcmp(command, "quit") == 0){
			freeHistArr(histArr);
			freeLinks(head);
                        exit(0);
                }


                // cd command
                // moves directories using chdir
                else if (strcmp(command, "cd") == 0){
                        addToHist(histArr, userInput, &index);
                        // check if valid amount of args
                        // then check if directory is valid
                        if (count > 1){
                                fprintf(stderr, "error: too many arguments provided\n");
                        }
                        else{
                                int chdRet = chdir(args1);
                                if (chdRet == -1){
                                        fprintf(stderr, "error: %s\n", strerror(errno));

                                }
                        }
                }

                // history command
                else if (strcmp(command, "history") == 0){
			// for regular hist
                        if (count == 0){
                                printHistArr(histArr, index);
                        }
			// for -c and index
                        else if (count == 1){
                                if (strcmp(args1, "-c") == 0){
                                        clearHist(histArr, &index);
                                }
				// if it has an index we first check it
                                else{
                                        if (checkIfValid(index, args1) == 1){
						// gets the input from the history buffer
						// then changes the hasInput to 1
						getNewInp(args1, histArr, buffer);
						hasInput = 1;
                                        }
                                }
                        }
                }


		// new command
		else if (strcmp(command, "new") == 0){
			addToHist(histArr, userInput, &index);
			if (count < 1){
				fprintf(stderr, "error: too few arguments provided\n");
			}
			else{				
				head = insertNode(head, args1, userInput, count);
			}
		}

		// list command
		else if (strcmp(command, "list") == 0){
			addToHist(histArr, userInput, &index);
			listsNodes(head);
		}

		// open command
		else if (strcmp(command, "open") == 0){
			addToHist(histArr, userInput, &index);
			if (head != NULL){
				if (count != 2){
					fprintf(stderr, "error: incorrect number of arguments\n");
				}
				else{
					FILE *fp_file = fopen(args2, "r");
					if (fp_file == NULL){
						fprintf(stderr, "error: file cannot be opened\n");
					}
					else{
						addToFileContents(head, fp_file, atoi(args1));
						fclose(fp_file);
						fp_file = NULL;
				       	}
				}
			}
		}

		// execute command

		else if (strcmp(command, "execute") == 0){
			addToHist(histArr, userInput, &index);
			if (count != 1){
				fprintf(stderr, "error: incorrect number of arguments\n");
				continue;
			}
			ListNode *node = getNode(atoi(args1), head);
			if (node == NULL){
				fprintf(stderr, "error: %s\n", "Id does not exist");
				continue;
			}

			if (node->arguments_length > 128){
				fprintf(stderr, "error: %s\n", "too many arguments");
				continue;
			}

			int apiRet = run_command(node);
			// moving 8 bits to the right
			int exit_status = (apiRet >> 8) & 0xFF;

			if (exit_status != 0){
				fprintf(stderr, "error: %s\n", strerror(exit_status));
				continue;
			}
		
		}

		// remove command
		else if (strcmp(command, "remove") == 0){
			addToHist(histArr, userInput, &index);
			if (count < 1){
				fprintf(stderr, "error: too few arguments provided\n");
			}
			else{
				removeNodes(head, atoi(args1));
			}
		}
		
		// everything else
                else {
                        if (strcmp(command, " ") != 0 && strcmp(command, "\n") != 0){
                                addToHist(histArr, userInput, &index);
                        }
                }
        } // for while loop

        return 0;
} // for main



// gets the node associated to the id
ListNode *getNode(int id, ListNode *head){
	ListNode *walker = head;
	while (walker != NULL && walker->id != id){
		walker = walker->next;
	}
	if (walker == NULL){
		return NULL;	
	}
	else{
		return walker;
	}
}

// frees the given node
void freeSingleNode(ListNode *node){
	free(node->command);
	node->command = NULL;
	for (int i = 0; i < node->arguments_length; i++){
		free(node->arguments[i]);
		node->arguments[i] = NULL;
	}
	free(node->arguments);
	node->arguments = NULL;
	free(node->file_contents);
	node->file_contents = NULL;
}

// used for removing nodes
ListNode *removeNodes(ListNode *head, int id){
	ListNode *walker = head;
	ListNode *reaper = NULL;

	if (walker == NULL){
		return NULL;
	}
	
	if (walker->id == id){
		walker->next->prev = NULL;
		reaper = walker->next;
		walker->next = NULL;
		freeSingleNode(walker);
		free(walker);
		walker = NULL;
		return reaper;
	}
	while (walker->next != NULL && walker->next->id != id){
		walker = walker->next;
	}
	
	if (walker->next == NULL){
		if (walker->id == id){
			walker->prev->next = NULL;
			walker->prev = NULL;
			freeSingleNode(walker);
			free(walker);
			walker = NULL;
		}
		else{
			fprintf(stderr, "error: %s\n", "Id does not exist");
		}
	}
	else{
		reaper = walker->next;
		walker->next = reaper->next;
		walker->next->prev = walker;
		reaper->next = NULL;
		reaper->prev = NULL;
		freeSingleNode(reaper);
		free(reaper);
		reaper = NULL;
	}
	return head;
}


// used to add to the file contents
void addToFileContents(ListNode *head, FILE *file, int index){
	ListNode *walker = head;

	// getting the number of bytes in the file to assign to our file_contents
	int cc = 0;
	char letter = fgetc(file);
	while (letter != EOF){
		cc++;
		letter = fgetc(file);
	}

	rewind(file);
	if (cc != 0){
		char buff[cc];

		while (walker != NULL && walker->id != index){
			walker = walker->next;
		}

		if (walker == NULL){
			fprintf(stderr, "error: %s\n", "Id does not exist");
		}
		else{
			if (walker->file_contents != NULL){
				free(walker->file_contents);
			}
			walker->file_contents = malloc(sizeof(char)*(cc+1));
			strcpy(walker->file_contents, "");
			while (fgets(buff, sizeof(buff), file) != NULL){
				strcat(walker->file_contents, buff);
			}
		}
	}
}

// creates and inserts a node in proper order
ListNode *insertNode(ListNode *head, char *cmd, char *userInput, int count){
	ListNode *node = makeNode(cmd, userInput, count);
	if (head == NULL){
		node->id = 0;
		return node;
	}
	else if (head->id != 0){
		node->next = head;
		node->id = 0;
		head->prev = node;
		return node;
	}
	else if (head->id == 0 && head->next == NULL){
		head->next = node;
		node->id = 1;
		node->prev = head;
		return head;
	}
	else{
		ListNode *walker = head;
		while (walker->next != NULL && ((walker->next->id) - (walker->id)) <= 1){
			walker = walker->next;
		}
		
		if (walker->next != NULL){	
			ListNode *temp = walker->next;
			walker->next = node;
			node->prev = walker;
			node->id = (walker->id)+1;
			node->next = temp;
			temp->prev = node;
		}
		else{
			walker->next = node;
			node->prev = walker;
			node->id = (walker->id)+1;
		}
		return head;
	}
}


// prints the nodes in order	
void listsNodes(ListNode *head){
	ListNode *walker = head;
	while (walker != NULL){
		printf("List Node %d\n", walker->id);
		printf("\tCommand: %s\n", walker->command);
		printf("\tFile Contents:\n");
		if (walker->file_contents != NULL){
			char *temp = strdup(walker->file_contents);
			char *token = strtok(temp, "\n");
			while (token != NULL){
				printf("\t\t%s\n", token);
				token = strtok(NULL, "\n");
			}	
			free(temp);	
		}
		walker = walker->next;
	}
}

// used to make a new node;
ListNode *makeNode(char *cmd, char *userInput, int count){
	// makes new listnode
	ListNode *new = malloc(sizeof(ListNode));
	// assigns heap memory for command
	new->command = malloc(sizeof(char) * 10000);
	// assigns command
	strcpy(new->command, cmd);
	// makes heap memory for arguments
	new->arguments = malloc(sizeof(char *) * (count + 1));
	for (int i = 0; i < count; i++){
		new->arguments[i] = malloc(sizeof(char) * 10000);
	}

	new->arguments[count] = NULL;

	// setting file_contents to Null for now
	new->file_contents = NULL;

	// setting arguments length
	new->arguments_length = count;
	
	// filling in the arguments array
	// by tokenizing using strtok
	char temp[10000];
	strcpy(temp, userInput);
	char *token = strtok(temp, " "); 
	token = strtok(NULL, " ");
	for (int i = 0; i < count; i++){
		strcpy(new->arguments[i], token);
		token = strtok(NULL, " ");
	}
	// setting final to NULL
	new->arguments[count] = NULL;
	// setting the pointer to next to NULL
	new->next = NULL;
	new->prev = NULL;
	return new;
}

// frees the linked list
void freeLinks(ListNode *head){
	ListNode *walker = head;
	ListNode *reaper = NULL;
	
	while (walker != NULL){
		reaper = walker;
		walker = walker->next;
		free(reaper->command);
		reaper->command = NULL;
		free(reaper->file_contents);
		reaper->file_contents = NULL;
		for (int i = 0; i < reaper->arguments_length; i++){
			free(reaper->arguments[i]);
			reaper->arguments[i] = NULL;
		}
		free(reaper->arguments);
		reaper->arguments = NULL;
		free(reaper);
		reaper = NULL;
	}	
}


// populating the fields with this
void populate(char *userInput, char *command, char *args1, char *args2, int *count){
        countArgs(userInput, count);
        char temp[10000];
        strcpy(temp, userInput);
        char *token = strtok(temp, " ");

        // check to avoid seg fault due to " "
        if (token != NULL){
                strcpy(command, token);
        }

        int i = 0;
                // populating using strtok again
                // checking if it has 1 or 2 args
                // and populating based on that
                while (i < *count){
                        token = strtok(NULL, " ");
                        if (i == 0){
                                strcpy(args1, token);
                        }
                        else if (i == 1){
                                strcpy(args2, token);
                        }
                        i++;
                }
}


// frees the history array
void freeHistArr(char **histArr){
	for (int i = 0; i < 100; i++){
		free(histArr[i]);
		histArr[i] = NULL;
	}
	free(histArr);
	histArr = NULL;
}


// counting the no of args
void countArgs(char *userInput, int *count){
        // created a temp array and tokenized it to count
        char temp[10000];
        strcpy(temp, userInput);
        char *token = strtok(temp, " ");
        int x = 0;
        while (token != NULL){
                token = strtok(NULL, " ");
                if (token != NULL){
                        x++;
                }
        }

        *count = x;
}

// used to add to the history array
void addToHist(char **histArr, char *userInput, int *index){

        if ((*index) < 100){
                strcpy(histArr[*index], userInput);
                (*index)++;
        }
        else{
                // changes the index of the values and make them shift one step to the right
                for (int i = 0; i < 99; i++){
                        strcpy(histArr[i], histArr[i+1]);
                }
                // add this to the end
                strcpy(histArr[99], userInput);
        }

}

// used to print history
void printHistArr(char **histArr, int index){
        for (int i = 0; i < index; i++){
                printf("%d: %s\n", i, histArr[i]);
        }
}


// clears history array
void clearHist(char **histArr, int *index){
        for (int i = 0; i < (*index) + 1; i++){
                strcpy(histArr[i], "");
        }
        (*index) = 0;
}

// checks if the args is valid
int checkIfValid(int index, char *args1){
        if (atoi(args1) < index && atoi(args1) >= 0){
                return 1;
        }
        else{
                fprintf(stderr, "error: %s\n", "Index in history list does not exist");
                return 0;
        }
}

// reads userInput
void reDo(char *buffer, char *userInput, int hasInp, char **histArr, ListNode *head){
	// if the user has an input they want to do in the array then doesnt ask
	// if they do not asks
	if (!hasInp){

		if (fgets(buffer, 10000, stdin) != NULL){
			sscanf(buffer, "%[^\n]", userInput);
		}
		else{
			freeHistArr(histArr);
			freeLinks(head);
                        exit(0);
			exit(0);
		}
	}
	else{
		sscanf(buffer, "%[^\n]", userInput);
	}
}

// gets the user's input from the history array
void getNewInp(char *args1, char **histArr, char *buffer){
	strcpy(buffer, histArr[atoi(args1)]);
}
