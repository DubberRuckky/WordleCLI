#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define WORD_FILE "wordle-words/wordle-words.txt"
#define WORD_LEN 5
#define MAX_TRIES 6

#define GREY "\033[40m"
#define YELLOW "\033[43m"
#define GREEN "\033[42m"
#define RESET "\033[0m"
#define CLEAR_PREV_LINE "\033[A\033[2K" //makes sure that the output is on the same line (deletes the line and moves it to the beginning of the line)

/* Picks a random word from WORD_FILE. Returns NULL if the file can't be read. */
const char *pickWord(void) {
  static char word[WORD_LEN + 2];
  FILE *file = fopen(WORD_FILE, "r");
  if (file == NULL) {
    return NULL;
  }

  /* First pass: count the words. */
  int count = 0;
  while (fgets(word, sizeof word, file) != NULL) {
    count++;
  }
  if (count == 0) {
    fclose(file);
    return NULL;
  }

  /* Second pass: read up to the randomly chosen word. */
  int chosen = rand() % count;
  rewind(file);
  for (int i = 0; i <= chosen; i++) {
    fgets(word, sizeof word, file);
  }
  fclose(file);

  word[strcspn(word, "\r\n")] = '\0';
  return word;
}

/* Returns 1 if word (lowercase) is in WORD_FILE, otherwise 0. */
int isInWordList(const char *word) {
  char line[WORD_LEN + 2];
  FILE *file = fopen(WORD_FILE, "r");
  if (file == NULL) {
    return 0;
  }

  while (fgets(line, sizeof line, file) != NULL) {
    line[strcspn(line, "\r\n")] = '\0';
    if (strcmp(line, word) == 0) {
      fclose(file);
      return 1;
    }
  }

  fclose(file);
  return 0;
}

/* Reads one line from the user. Returns 0 on end of input, 1 otherwise. */
int readLine(char *buffer, int size) {
  if (fgets(buffer, size, stdin) == NULL) {
    return 0;
  }
  if (strchr(buffer, '\n') == NULL) {
    int c; /* line was too long: throw away the rest of it */
    while ((c = getchar()) != '\n' && c != EOF) {
    }
  }
  buffer[strcspn(buffer, "\n")] = '\0';
  return 1;
}

/* A guess is valid if it is exactly WORD_LEN letters. Lowercases it in place. */
int isValidGuess(char *guess) {
  if (strlen(guess) != WORD_LEN) {
    return 0;
  }
  for (int i = 0; i < WORD_LEN; i++) {
    if (!isalpha((unsigned char)guess[i])) {
      return 0;
    }
    guess[i] = (char)tolower((unsigned char)guess[i]);
  }
  return 1;
}

/* Prints the guess with each letter coloured green / yellow / grey. */
void printGuess(const char *guess, const char *answer) {
  const char *colors[] = {GREY, YELLOW, GREEN};
  int result[WORD_LEN] = {0}; /* 0 = grey, 1 = yellow, 2 = green */
  int used[WORD_LEN] = {0};   /* answer letters already matched */

  /* Green: right letter, right place. */
  for (int i = 0; i < WORD_LEN; i++) {
    if (guess[i] == answer[i]) {
      result[i] = 2;
      used[i] = 1;
    }
  }

  /* Yellow: right letter, wrong place (each answer letter counts only once). */
  for (int i = 0; i < WORD_LEN; i++) {
    if (result[i] == 2) {
      continue;
    }
    for (int j = 0; j < WORD_LEN; j++) {
      if (!used[j] && guess[i] == answer[j]) {
        result[i] = 1;
        used[j] = 1;
        break;
      }
    }
  }

  for (int i = 0; i < WORD_LEN; i++) {
    printf("%s %c %s", colors[result[i]], toupper((unsigned char)guess[i]), RESET);
  }
  printf("\n");
}

/* Plays one round. Returns the number of tries used, or -1 if input ended. */
int playRound(const char *answer) {
  char input[100];
  int tries = 0;

  while (tries < MAX_TRIES) {
    printf("Guess %d/%d: ", tries + 1, MAX_TRIES);
    if (!readLine(input, sizeof input)) {
      return -1;
    }

    if (!isValidGuess(input)) {
      printf("Please enter a %d-letter word.\n", WORD_LEN);
      continue;
    }

    if (!isInWordList(input)) {
      printf("Not in the word list.\n");
      continue;
    }

    tries++;
    printf(CLEAR_PREV_LINE); /* replace the typed line with the coloured one */
    printGuess(input, answer);

    if (strcmp(input, answer) == 0) {
      return tries;
    }
  }

  printf("Out of tries! The word was: %s\n", answer);
  return tries;
}

/* Asks until the user answers y or n. Returns 1 for yes, 0 for no / end of input. */
int askToContinue(void) {
  char input[100];

  while (1) {
    printf("Do you want to continue [y/n]: ");
    if (!readLine(input, sizeof input)) {
      return 0;
    }
    if (tolower((unsigned char)input[0]) == 'y' && input[1] == '\0') {
      return 1;
    }
    if (tolower((unsigned char)input[0]) == 'n' && input[1] == '\0') {
      return 0;
    }
  }
}

int main(void) {
  srand((unsigned int)time(NULL));

  while (1) {
    const char *answer = pickWord();
    if (answer == NULL) {
      printf("Could not read the word list: %s\n", WORD_FILE);
      return 1;
    }

    printf("\n***WORDLE***\n");
    int tries = playRound(answer);
    if (tries < 0) {
      printf("\n");
      break;
    }

    printf("***Wordle in %d***\n", tries);
    if (!askToContinue()) {
      break;
    }
  }

  return 0;
}
