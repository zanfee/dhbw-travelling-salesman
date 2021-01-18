// Kurs INF20A
// Bearbeiter: Jan Fröhlich, Gabriel Nill, Fabian Weller

// TODO: Discuss whether we should switch to German output because the menu
// itself has to be German.

#include <ctype.h> // iscntrl() isspace() Funktionen für ASCII-Zeichen
#include <limits.h>
#include <stdbool.h>
#include <stdio.h> // Ein- und Ausgabefunktionen
#include <stdlib.h> // Stringkonvertierung, Zufallszahlen, Speicherallokation, Sortieren u.a.
#include <string.h> // prototype for strtok() because gcc expects int as return type
#include <unistd.h> // access(), maybe not needed

#ifdef _WIN32
#include <windows.h>
#define COLOR_PRIMARY 9
#define COLOR_SUCCESS 10
#define COLOR_INFO 11
#define COLOR_ERROR 12
#define COLOR_WARNING 6
#define COLOR_DEFAULT 15

#else
#define COLOR_PRIMARY 34
#define COLOR_SUCCESS 32
#define COLOR_INFO 36
#define COLOR_ERROR 31
#define COLOR_WARNING 33
#define COLOR_DEFAULT 0
#endif

typedef struct {
  int from;
  int to;
  int dist;
} Distance;

typedef struct {
  int n;
  char **cities;
  Distance *distances;
} DistanceTable;

typedef struct {
  char key;
  char *description;
} Menu;

void setConsoleColor(int color) {
#ifdef _WIN32
  SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
#else
  printf("\033[%dm", color);
#endif
}

// Return amount of digits in integer (0 => 1, 53 => 2, 1234 => 4)
int intDigits(int n) {
  if (n < 0)
    return intDigits(-n);
  if (n < 10)
    return 1;
  return 1 + intDigits(n / 10);
}

// Return length of string with support for unicode (ö counts as one character)
int strlen_utf8(char *s) {
  int i = 0, j = 0;
  while (s[i]) {
    if ((s[i] & 0xc0) != 0x80)
      j++;
    i++;
  }
  return j;
}

// Norms a string to a specific minimum/maximum length with support for unicode.
// If string is smaller than minimum, it will fill the rest with the filler
// char. If larger, it will crop.
char *substr_utf8(const char *src, size_t min, size_t max, char filler) {
  size_t i = 0, j = 0;
  while (j < max && src[i]) {
    if ((src[i] & 0xc0) != 0x80)
      j++;
    i++;
  }

  char *substr = "";
  substr = calloc((i > min ? i : min) + 1, sizeof(char));
  memcpy(substr, &src[0], i);
  substr[i] = '\0';

  for (int k = 0; min > k + j; k++) {
    strncat(substr, &filler, 1);
  }
  return substr;
}

// Loads the distance table from a file
DistanceTable loadData() {
  DistanceTable distanceTable = {.n = 0};

  setConsoleColor(COLOR_DEFAULT);
  printf("Please enter the name of the file which should be loaded.\n");
  setConsoleColor(COLOR_PRIMARY);
  char path[100];
  scanf("%99s", &path[0]);
  if (path[0] < 0) {
    setConsoleColor(COLOR_DEFAULT);
    distanceTable.n = -1;
    return distanceTable;
  }
  printf("\n");

  FILE *fpointer = fopen(path, "r");
  if (fpointer == NULL) {
    if (access(path, F_OK) == 0) {
      setConsoleColor(COLOR_ERROR);
      printf("Error loading data: File is blocked by a program.\n");
    } else {
      setConsoleColor(COLOR_ERROR);
      printf("Error loading data: File not found.\n");
    }
    distanceTable.n = -1;
    return distanceTable;
  } else {
    ssize_t read;
    char *line = NULL;
    size_t len = 10;
    int memcycle = 4;

    if ((read = getline(&line, &len, fpointer)) !=
        -1) // TODO look how this works
    {
      char *city = strtok(line, "\n ");
      while (city != NULL) {
        if (distanceTable.n % memcycle ==
            0) { // Allocate memory every (memcycle)th step/word
          char **tmpCities;
          tmpCities = realloc(distanceTable.cities,
                              (distanceTable.n + memcycle) * sizeof(char *));
          if (tmpCities == NULL) {
            setConsoleColor(COLOR_ERROR);
            printf("Error: Out of memory.\n");
            distanceTable.n = -1;
            return distanceTable;
          } else {
            distanceTable.cities = tmpCities;
          }
        }

        distanceTable.cities[distanceTable.n] =
            calloc(strlen(city) + 1, sizeof(char));

        strcpy(distanceTable.cities[distanceTable.n], city);

        city = strtok(NULL, "\n ");
        distanceTable.n++;
      }

      distanceTable.distances =
          calloc(distanceTable.n * distanceTable.n, sizeof(Distance));
      for (int i = 0; i < distanceTable.n; i++) {
        read = getline(&line, &len, fpointer);
        if (read > 0) {
          char *distString = strtok(line, " \n");
          for (int j = 0; j < distanceTable.n; j++) {
            long dist = strtol(distString, NULL, 0);

            if (i == j) { // start city == destination
              if (dist != 0) {
                setConsoleColor(COLOR_ERROR);
                printf("Error reading from file: Expected '0', got '%s' in "
                       "line %u word %u.\n",
                       distString, i + 2, j + 1);
                i = j = distanceTable.n;
              } else {
                Distance distance = {.from = i, .to = j, .dist = dist};
                distanceTable.distances[i * distanceTable.n + j] = distance;
              }
            } else {
              if (dist > 0) {
                Distance distance = {.from = i, .to = j, .dist = dist};
                distanceTable.distances[i * distanceTable.n + j] = distance;
              } else {
                setConsoleColor(COLOR_ERROR);
                printf("Error reading from file: Expected number greater than "
                       "0, got '%s' in line %u "
                       "word %u.\n",
                       distString, i + 2, j + 1);
                i = j = distanceTable.n;
              }
            }
            distString = strtok(NULL, "\n ");
          }
        } else {
          setConsoleColor(COLOR_ERROR);
          printf("Error reading from file: Line %u is empty.\n", i);
          i = distanceTable.n;
        }
      }
    }

    fclose(fpointer);
    if (line) {
      free(line);
    }
    setConsoleColor(COLOR_SUCCESS);
    printf("The distance table was loaded successfully. (%s)\n", path);
    return distanceTable;
  }
}

void saveData() { printf("save"); }

void showData(
    DistanceTable distanceTable) // TODO Print if there are unsaved changes
{
  // Check if there is any data to display
  if (distanceTable.n > 0) {
    int largestCityNameLength = 0;

    // Initialize length of column (in chars) with minimum value of 3
    int *columnLengths;
    columnLengths = calloc(distanceTable.n, sizeof(int));
    for (int i = 0; i < distanceTable.n; i++) {
      columnLengths[i] = 3;
    }

    for (int i = 0; i < distanceTable.n; i++) {
      // Find out how many characters the longest city name has
      int cityNameLength = strlen_utf8(distanceTable.cities[i]);
      if (cityNameLength > largestCityNameLength)
        largestCityNameLength = cityNameLength;

      for (int j = 0; j < distanceTable.n; j++) {
        // Find out how many digits the largest distance has
        int distanceLength = intDigits(distanceTable.distances[i * 5 + j].dist);
        if (distanceLength > columnLengths[j])
          columnLengths[j] = distanceLength;
      }
    }

    // Print title row
    printf("%*s", largestCityNameLength + 1, ""); // Margin for the first line
    setConsoleColor(COLOR_INFO);
    for (int i = 0; i < distanceTable.n; i++) {
      // Print titles of columns
      printf("%s ", substr_utf8(distanceTable.cities[i], columnLengths[i],
                                columnLengths[i], ' '));
    }
    printf("\n");

    // Print rows except for title row
    for (int i = 0; i < distanceTable.n; i++) {
      setConsoleColor(COLOR_INFO);
      // Print titles of rows with margin, so that the entire row has the same
      // width
      printf("%s ", substr_utf8(distanceTable.cities[i], largestCityNameLength,
                                largestCityNameLength, ' '));
      setConsoleColor(COLOR_DEFAULT);
      for (int j = 0; j < distanceTable.n; j++) {
        // Print digits (distances)
        printf("%*d ", columnLengths[j],
               distanceTable.distances[i * distanceTable.n + j].dist);
      }
      printf("\n");
    }
  } else {
    printf("The distance table is empty.\n");
  }
}

int getCityNumber(DistanceTable *distanceTable, char city[100]) {
  for (int i = 0; i < distanceTable->n; i++) {
    if (strcmp(distanceTable->cities[i], city) == 0) {
      return i;
    }
  }
  return -1; // City was not found
}

Distance *getDistanceStructBetweenCities(DistanceTable *distanceTable, int from,
                                         int to) {
  for (int i = 0; i < distanceTable->n * distanceTable->n - 1; i++) {
    if (from == distanceTable->distances[i].from &&
        to == distanceTable->distances[i].to)
      return &distanceTable->distances[i];
  }

  return 0;
}

// TODO Error in reading complete file? Last city isn't resolved. Is my code
// wrong?
void changeDistanceBetweenCities(DistanceTable *distanceTable) {
  if (distanceTable->n == 0) {
    setConsoleColor(COLOR_ERROR);
    printf("Please load a distance table first.\n");
    return;
  }

  setConsoleColor(COLOR_DEFAULT);
  printf("Please enter the name of the first city:\n");
  char firstCity[100];
  scanf("%s", firstCity);

  int firstCityNumber = getCityNumber(distanceTable, firstCity);
  if (firstCityNumber == -1) {
    setConsoleColor(COLOR_ERROR);
    printf("This city doesn't exist.\n");
    return;
  }

  printf("\n");

  printf("Please enter the name of the second city:\n");
  char secondCity[100];
  scanf("%s", secondCity);

  int secondCityNumber = getCityNumber(distanceTable, secondCity);
  if (secondCityNumber == -1) {
    setConsoleColor(COLOR_ERROR);
    printf("This city doesn't exist.\n");
    return;
  }

  printf("\n");

  Distance *firstToSecond = getDistanceStructBetweenCities(
      distanceTable, firstCityNumber, secondCityNumber);
  Distance *secondToFirst = getDistanceStructBetweenCities(
      distanceTable, secondCityNumber, firstCityNumber);
  printf("Current distances:\n");
  printf("%s to %s: %d\n", firstCity, secondCity, firstToSecond->dist);
  printf("%s to %s: %d\n", secondCity, firstCity, secondToFirst->dist);
  printf("\n");

  bool invalid;
  int firstDist;
  int secondDist;
  char input[100];
  // TODO Maybe put into a helper method

  do {
    invalid = false;
    printf("Please input a new distance for %s to %s\n", firstCity, secondCity);
    scanf("%s", input);
    long int newDistance = strtol(input, NULL, 10);

    // Check for validity
    if (newDistance < 0) {
      printf("The distance must be positiv.\n");
      invalid = true;
    } else if (newDistance > INT_MAX) {
      printf("The number is too big.\n");
      invalid = true;
    } else {
      firstDist = (int)newDistance;
    }
  } while (invalid);

  do {
    invalid = false;
    printf("Please input a new distance for %s to %s\n", secondCity, firstCity);
    scanf("%s", input);
    long int newDistance = strtol(input, NULL, 10);

    // Check for validity
    if (newDistance < 0) {
      printf("The distance must be positiv.\n");
      invalid = true;
    } else if (newDistance > INT_MAX) {
      printf("The number is too big.\n");
      invalid = true;
    } else {
      secondDist = (int)newDistance;
    }
  } while (invalid);

  // Replace distances
  firstToSecond->dist = firstDist;
  secondToFirst->dist = secondDist;

  printf("Success!");
}

void calculateShortestRoute() { printf("calculate"); }

void exitProgram() {
  printf("exit");
  setConsoleColor(COLOR_DEFAULT);
}

void printMenu(Menu *menu, int length) {
  printf("\n");
  for (int i = 0; i < length; i++) {
    printf("(%c) %s\n", menu[i].key, menu[i].description);
  }
}

int main() {
#ifdef _WIN32
  SetConsoleOutputCP(65001);
#endif

  DistanceTable distanceTable = {.n = 0};
  DistanceTable tmpDistanceTable = {.n = 0};

  Menu startMenu[] = {
      {'a', "Entfernungstabelle laden"},
      {'b', "Entfernungstabelle speichern"},
      {'c', "Entfernungstabelle anzeigen"},
      {'d', "Entfernung zwischen zwei Städten ändern"},
      {'e', "Kürzeste Route berechnen"},
      {'f', "Programm beenden"},
  };
  int startMenuLength = sizeof(startMenu) / sizeof(startMenu[0]);

  char c;
  do {
    setConsoleColor(COLOR_DEFAULT);
    printMenu(startMenu, startMenuLength);

    setConsoleColor(COLOR_PRIMARY);
    do {
      c = getchar();
    } while (isspace(c));
    printf("\n");

    switch (c) {
    case 'a':
      tmpDistanceTable = loadData();
      if (tmpDistanceTable.n >= 0) {
        distanceTable = tmpDistanceTable;
      }
      break;
    case 'b':
      saveData();
      break;
    case 'c':
      showData(distanceTable);
      break;
    case 'd':
      changeDistanceBetweenCities(&distanceTable);
      break;
    case 'e':
      calculateShortestRoute();
      break;
    case 'f':
      exitProgram();
      break;
    case -1:
      break;
    default:
      setConsoleColor(COLOR_ERROR);
      printf("'%c' is not a valid input.\n", c);
    }
  } while (c != 'f');
  return 0;
}
